#include <sys/types.h> 
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <stdlib.h>
#include <errno.h>
#define Myport 8848                           //端口号                                       
#define Backlog 100                           //
 
 
int sockfd=-1;
int recvret=-1;
char recvbuf[50]={0};
int acceptfd=-1;
int i=0;
int j=0;
char username[12]={0};
int flag=0 ;
int jionret=-1;
 struct User
{
	int opclo;
	char name[20];
	char passwd[6];
	char name1[20];
	int fd;
	pthread_t th;
}user[3]={{-1,"chen","123456","chen123456",-1,-1},
          {-1,"xia","223456","xia223456",-1,-1},
          {-1,"fang","323456","fang323456",-1,-1}};
 
 ////////////////////////////////////////////////////////////////////////////////////
void sigfunc(int sig)
{
    if(sig==SIGINT)
    {
		int k;
		flag=-1;
		for(k=0;k<3;k++)
		{
			if(user[k].opclo==1)
			{
				if((close(user[k].fd))!=0)
				{
					printf("关闭出错sig\n");
				}
			}
		}
		if(close(sockfd)==0)
		{
        printf("安全退出！\n");
        exit(0);
		}
    }
}
/////////////////////////////////////////////////////////////////////////////////////
void* thfunc(void *arg)
{  
	int f=(int)arg;
	char buff[400]={0};
	while(flag==0)
	{
		size_t size;
		if((size=recv(user[f].fd,buff,sizeof(buff),0))<0)
		{
			perror("recv error");
			break;
		}else if(size==0)
		{
			printf("%s已断开\n",user[f].name);
			break;
		}else{
			    char opensig[]={" "};
			    if((strcmp(buff,"signal_quit"))==0)
				{
					send(user[f].fd,opensig,strlen(opensig),0);
					memset(opensig,0,sizeof(opensig));
					break;
				}
			    for(int i=0;i<3;i++)
				{
					if((user[i].opclo==1)&&(i!=f))
					{
						char sendbuff[512]={0};
						memcpy(sendbuff,user[f].name,sizeof(user[f].name));
						strcat(sendbuff," : ");  strcat(sendbuff,buff);
						if(send(user[i].fd,sendbuff,strlen(sendbuff),0)<0)
						{
							if(errno==ENOTCONN)
							    perror("send error");
							break;
						}
					}
				}
			   }
		memset(buff,0,sizeof(buff));
	}
	if((close(user[f].fd))==0)
	user[f].opclo=-1;
	printf("子线程已退出\n");
	pthread_exit(NULL);
	return (void *)0;
}
 
///////////////////////////////////////////////////////////////////////////////////////
int main(int argc,char *argv[])
{
   
    signal(SIGINT,sigfunc);
	//1.创建套接字
	//2.绑定端口和ip地址
	//套接字返
    int bindfd=-1;
    int listenfd=-1;           //监听函数返回值
    char savedate[100]={0};
    int a=-1;
	sockfd=socket(AF_INET,SOCK_STREAM,0);
	if(-1==sockfd)
	{
		perror("socket");               //返回值验证
		close(sockfd);
		return -1;
	}
    printf("sockfd=%d\n",sockfd);
 
	// 设置套接字选项避免地址使用错误  
    int on=1;  
    if((setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on)))<0)  
    {  
        perror("setsockopt failed");  
        exit(EXIT_FAILURE);  
    }  
	/***********************************
	/*上面部分为创建套接字*/
    struct sockaddr_in svraddr={0};  //bind函数需要传入的ip端口结构体
	svraddr.sin_family=AF_INET;                  //1     //ip类型    ipv4;
    svraddr.sin_port=htons(Myport);           //2     //端口号 需要转换成机器能识别的端口号 不能直接等于6003
    svraddr.sin_addr.s_addr=INADDR_ANY; //3     //ip
	bindfd=bind(sockfd,(const struct sockaddr *)&svraddr,sizeof(svraddr));
    if(-1==bindfd)
    {
        perror("bind");
		close(sockfd);
        return -1;
    }
    printf("bind success.\n");
    printf("bindfd=%d\n",bindfd);
    //3.第三步：listen 监听端口
    listenfd=listen(sockfd,Backlog);
    if(-1==listenfd)
    {
        perror("listen");
		close(sockfd);
        return -1;
    }
    //4.第四步：accept服务器阻塞等待连接
    struct sockaddr_in Acceptretaddr={0};
 
	socklen_t lenth={0};
	printf("等待客户端连接\n");
    
	/********************************/
	/*设置线程分离属性*/
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
	/********************************/
    //第五步：服务器接收数据
   while(flag==0)
   {   
	   acceptfd=accept(sockfd,(struct sockaddr *)&Acceptretaddr,&lenth);
	    if(acceptfd<0)
	   {
	       perror("accept");
	   }
	   //连上了，检测用户和密码
	   memset(username,0,sizeof(username));
	   recvret=recv(acceptfd,username,sizeof(username),0);
	   int t=0;
	   for(j=0;j<3;j++)
	   {
		   int cm=-1;
		   cm=strcmp(username,user[j].name1);
	   	if(cm==0)
	   	{//用户密码都正确创建子线程进行通信
			char information[]={"1"};
			    send(acceptfd,information,strlen(information),0);
				memset(information,0,sizeof(information));
			    printf("%s进入\n",user[j].name);
			    user[j].opclo=1;
				user[j].fd=acceptfd;
				int err=-1;
				err=pthread_create(&user[j].th,&attr,thfunc,(void*)j);//将用户的信息传给对应的线程
				if(err!=0)
				{
					perror("pthread create error");
			    }
				pthread_attr_destroy(&attr);//销毁分离属性
		}else{t++;}
      }
	  if(t==3)
	  {
		  char error_information[]={"0"};
		  send(acceptfd,error_information,sizeof(error_information),0);
		  close(acceptfd);
	  }
   }
   close(acceptfd);
   close(sockfd);
    return 0;
}