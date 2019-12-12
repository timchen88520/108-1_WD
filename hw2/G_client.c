#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include<string.h>
#define PORT 1234
#define MAXDATASIZE 100

char sendbuf[1024];
char recvbuf[1024];
char name[100];
int fd;
int chese_remain=9;
int place,type,chese_place[20];
int flag=0,flag1=0;

int determine(){
	int check[8]={0};
	int flag=0;

	check[0]=chese_place[0]+chese_place[3]+chese_place[6];  //0 1 2
	check[1]=chese_place[1]+chese_place[4]+chese_place[7];	 //3 4 5
	check[2]=chese_place[3]+chese_place[5]+chese_place[8];  //6 7 8
	check[3]=chese_place[0]+chese_place[1]+chese_place[2];
	check[4]=chese_place[3]+chese_place[4]+chese_place[5];
	check[5]=chese_place[6]+chese_place[7]+chese_place[8];
	check[6]=chese_place[0]+chese_place[4]+chese_place[8];
	check[7]=chese_place[2]+chese_place[4]+chese_place[6];

	for(int i=0;i<8;i++){
		//printf("check[%d]: %d\n",i,check[i]);
		if(check[i]==3){
			flag=1;
			return 1;
			break;
		}
		else if(check[i]==-3){
			flag=1;
			return 2;
			break;
		}

	}
	if(flag==0) return 0;
	return 3;
}
void chese(){
	//printf("in chese game\n");
	if (chese_remain>0){
		printf("choose a place to put your chese(1~9)\n 1==O  -1==X \nremain:%d\n",chese_remain);
		chese_place[place]=type;

		chese_remain--;
		strcat(chese_place+9,"cheseplace");
		return chese_place;
	}
	else if(chese_remain==0){
		int check;
		check=determine(chese_remain);
		if(check==1) printf("O is winner\n");
		if(check==2) printf("X is winner\n");
		if(check==0) printf("No one is winner\n");
		chese_remain=9;
		memset(chese_place,0,sizeof(chese_place));
	}

}
void pthread_recv(void* ptr)
{
	while(1)
	{	

		//printf("name: %s\nbuff: %s\n",name,recvbuf);
		if ((recv(fd,recvbuf,MAXDATASIZE,0)) == -1){
			printf("recv() error\n");
			exit(1);
		}
		//if(strstr(recvbuf,"/send")!=NULL) printf("got\n");
		else if(strstr(recvbuf,"ooxx")!=NULL && strstr(recvbuf,name)!=NULL)
			printf("comfirm game or not?(type 'start_play'/'shut_up')\n");

		else{
			printf("%s",recvbuf);
			memset(recvbuf,0,sizeof(recvbuf));
		}
	}
}



int main(int argc, char *argv[])
{
	int  numbytes;
	char buf[MAXDATASIZE];
	struct hostent *he;
	struct sockaddr_in server;


	if (argc !=2) {         printf("Usage: %s <IP Address>\n",argv[0]);
		exit(1);
	}


	if ((he=gethostbyname(argv[1]))==NULL){
		printf("gethostbyname() error\n");
		exit(1);
	}

	if ((fd=socket(AF_INET, SOCK_STREAM, 0))==-1){
		printf("socket() error\n");
		exit(1);
	}

	bzero(&server,sizeof(server));

	server.sin_family = AF_INET;
	server.sin_port = htons(PORT);
	server.sin_addr = *((struct in_addr *)he->h_addr);
	if(connect(fd, (struct sockaddr *)&server,sizeof(struct sockaddr))==-1){
		printf("connect() error\n");
		exit(1);
	}

	printf("connect success\n");
	char str[]="已进入聊天室\n";
	printf("请输入用户名：");
	fgets(name,sizeof(name),stdin);
	if(flag1==0){
		char tmp[100]="name:";
		strcat(tmp,name);
		strcpy(name,tmp);
		flag1=1;
	}

	//name[strlen(name)-1]='\0';
	send(fd,name,(strlen(name)-1),0);
	//send(fd,str,(strlen(str)),0);


	pthread_t tid;
	pthread_create(&tid,NULL,pthread_recv,NULL);


	
	while(1)
	{
		strcpy(name,name+5);
		memset(sendbuf,0,sizeof(sendbuf));
		fgets(sendbuf,sizeof(sendbuf),stdin);
		if(strstr(sendbuf,"exit")!=NULL){
			memset(sendbuf,0,sizeof(sendbuf));
			printf("您已退出群聊\n");
			send(fd,sendbuf,(strlen(sendbuf)),0);
			break;
		}
		/*if(flag>0){
			chese();
			printf("here: %s\n",chese_place);
			send(fd,name,(strlen(name)-1),0);
			send(fd,":",1,0);
			send(fd,sendbuf,(strlen(sendbuf)),0);
		//send(fd,chese_place,(strlen(chese_place)),0);
		flag--;
	}*/
	else if(strstr(sendbuf,"start_play"/*_play*/)!=NULL){
		printf("start play\tevery step please add insert before\n");
		flag=9;
		send(fd,name,(strlen(name)-1),0);
		send(fd,":",1,0);
		send(fd,sendbuf,(strlen(sendbuf)),0);
	//send(fd,chese_place,(strlen(chese_place)),0);
}
else if(strstr(sendbuf,"insert"/*_chese:*/)!=NULL){
	if(chese_remain>0){
		sscanf(sendbuf,"insert %d %d",&place,&type);
		chese_place[place]=type;
		int check=determine();
		chese_remain--;

		if(check==1){
			printf("Player O is winner\n GAME OVER\n");
			for(int i=0;i<9;i++) chese_place[i]=0;
			chese_remain=9;
		}
		if(check==2){
			printf("Player X is winner\n GAME OVER\n");
			for(int i=0;i<9;i++) chese_place[i]=0;
			chese_remain=9;
		}
	}
	send(fd,name,(strlen(name)-1),0);
	send(fd,":",1,0);
	send(fd,sendbuf,(strlen(sendbuf)),0);
}
else if(flag==0){
	send(fd,name,(strlen(name)-1),0);
	send(fd,":",1,0);
	send(fd,sendbuf,(strlen(sendbuf)),0);}
	else if(strstr(sendbuf,"shut_up")!=NULL){}
	else{
		send(fd,name,(strlen(name)-1),0);
		send(fd,":",1,0);
		send(fd,sendbuf,(strlen(sendbuf)),0);
	}


}
//  sprintf("")
//  if ((numbytes=recv(fd,buf,MAXDATASIZE,0)) == -1){
// printf("recv() error\n");
// exit(1);
// }

close(fd);
}
