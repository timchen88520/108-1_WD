#include<stdio.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#define SVERPNET  "192.168.1.103"                        
                                   //客户端不用定义自己的端口号，
                                   //但是需要声明服务器开放给客户端的端口号。
#define SERVport   8848
int sendret=-1;
int recvret=-1;
int flag = 0;
char recvbuf[100]={0};        //收数据的数组
 
int setsocketret=-1;
int bindfd=-1;
    int connectfd=-1;
    int sendfd=-1;
    char sendbuf[100]={0};        //发送数据的数组
    struct sockaddr_in servaddr={0};
	pthread_t  th=-1;  //线程标识号
	int sockfd=-1;
	int jionret=-1;
	
//显示登录////////////////////////////////////////////////////////////////////////////////
void Display()
{
	char passwd[6]={0};
	int b=-1;
	 char namebuf[12]={0};   //六位数用户账号
	printf("用户名账号:");
	scanf("%s",namebuf);
	printf("用户密码:");
	scanf("%s",passwd);
	strcat(namebuf,passwd);
	send(sockfd,namebuf,strlen(namebuf),0);
	//等待登录结果
	recvret=recv(sockfd,recvbuf,sizeof(recvbuf),0);
	if((recvret==0)||(strcmp(recvbuf,"0"))==0)
	{
		printf("登录错误 !请重新登录\n");
		close(sockfd);
		exit(-1);
	}
	if((recvret>0)&&(strcmp(recvbuf,"1"))==0)
	{
		printf("成功登录\n");
	}
}
 
///////////////////////////////////////////////////////////////////////////////////////////////
//终止处理函数
void sig1(int signum)
{
	char quit[]={"signal_quit"};
 
	if(SIGINT==signum)
	{
		flag=-1;
		send(sockfd,quit,strlen(quit),0);
	    close(sockfd);
		sleep(1);
	    jionret=pthread_join(th,NULL);
			if(jionret!=0)
			{
				printf("子线程没有被回收！");
			}
	    if(jionret==0)
	    {
		 printf("安全退出！\n");
	    }
        exit(0);
	}
}
 
//////////////////////////////////////////////////////////////////////////////////////////////
//子线程函数
void *func(void *arg)
{
	while(  flag==0  )
	{
		recvret=recv(sockfd,recvbuf,sizeof(recvbuf),0); //阻塞接收，
		if(recvret>0)
		{    
			printf("\t\t%s\n",recvbuf);
			memset(recvbuf,0,sizeof(recvbuf));
		}
		if(recvret==0)
		{
			printf("连接断开\n");
			break;
		}
	}
	printf("子线程已退出\n");
	pthread_exit(NULL);
}
 
////////////////////////////////////////////////////////////////////////////////////////////
 
int main(int argc,char *argv[])
{
    
	signal(SIGINT,sig1);
//第一步：创建套接字
 
 sockfd=socket(AF_INET,SOCK_STREAM,0);
 if(-1==sockfd)
    {
        perror("socket");
        return -1;
    }
//第二步：connect 连接服务器
    servaddr.sin_family=AF_INET;
    servaddr.sin_port=htons(SERVport);
    servaddr.sin_addr.s_addr=inet_addr(SVERPNET);
    connectfd=connect(sockfd,(const struct sockaddr *)&servaddr,sizeof(servaddr));
    if(-1==connectfd)
    {
        perror("connect");
        return -1;
    }
 
 
	//显示登录
    Display();
	//子线程读
	pthread_create(&th,NULL,func,NULL);
	//主线程写
	while(1)
	{
		scanf("%s",sendbuf);
		send(sockfd,sendbuf,strlen(sendbuf),0);
		memset(sendbuf,0,sizeof(sendbuf));   
	}
    return 0;
}
