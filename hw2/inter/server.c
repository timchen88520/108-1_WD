#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

typedef struct sockaddr *sockaddrp;

//存儲客戶端地址的結構體數組
struct sockaddr_in src_addr[50];
socklen_t src_len = sizeof(src_addr[0]);



//連接後記錄confd數組
int confd[50] = {};


//設置連接人數
int count = 0;


void *broadcast(void *indexp)
{
	int index = *(int *)indexp;
	char buf_rcv[255] = {};
	char buf_snd[255] = {};
	//第一次讀取用戶姓名
	char name[20] = {};
	int ret = recv(confd[index],name,sizeof(name),0);
	if(0 > ret)
	{
		perror("recv");
		close(confd[index]);
		return;
	}

	while(1)
	{
		bzero(buf_rcv,sizeof(buf_rcv));
		recv(confd[index],buf_rcv,sizeof(buf_rcv),0);

		//判斷是否退出
		if(0 == strcmp("quit",buf_rcv))
		{
			sprintf(buf_snd,"%s已經退出悟空聊天室",name);
			for(int i = 0;i <= count;i++)
			{
				if(i == index || 0 == confd[i])
				{
					continue;
				}
				send(confd[i],buf_snd,strlen(buf_snd),0);
			}
			confd[index] = -1;
			pthread_exit(0);

		}


		sprintf(buf_snd,"%s:%s",name,buf_rcv);
		printf("%s\n",buf_snd);
		for(int i = 0;i <= count;i++)
		{
			if(i == index || 0 == confd[i])
			{
				continue;
			}

			send(confd[i],buf_snd,sizeof(buf_snd),0);
		}

	}

}





int main(int argc,char **argv)
{
	printf("悟空聊天室服務器端開始運行\n");


	//創建通信對象
	int sockfd = socket(AF_INET,SOCK_STREAM,0);
	if(0 > sockfd)
	{
		perror("socket");
		return -1;
	}

	//準備地址
	struct sockaddr_in addr = {AF_INET};
	addr.sin_port = htons(atoi(argv[1]));
	addr.sin_addr.s_addr = inet_addr(argv[2]);

	socklen_t addr_len = sizeof(addr);



	//綁定
	int ret = bind(sockfd,(sockaddrp)&addr,addr_len);
	if(0 > ret)
	{
		perror("bind");
		return -1;
	}


	//設置最大排隊數
	listen(sockfd,50);

	int index = 0;


	while(count <= 50)
	{
		confd[count] = accept(sockfd,(sockaddrp)&src_addr[count],&src_len);
		++count;
		//保存此次客戶端地址所在下標方便後續傳入
		index = count-1;

		pthread_t tid;
		int ret = pthread_create(&tid,NULL,broadcast,&index);
		if(0 > ret)
		{
			perror("pthread_create");
			return -1;
		}


	}


}
