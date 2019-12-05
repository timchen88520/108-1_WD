#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
int main(int argc,char **argv){
  //show all ip address below
  struct addrinfo hints,*res,*p;
  int status;
  char ipstr[INET6_ADDRSTRLEN];

  memset(&hints,0,sizeof hints);
  hints.ai_family=AF_UNSPEC;
  hints.ai_socktype=SOCK_STREAM;

  if((status=getaddrinfo(argv[1],NULL,&hints,&res))!=0){
    fprintf(stderr,"wronginfo: %s\n",gai_stderror(status));
    return 2;
  }
}
 /*int status;
  struct addrinfo hints;
  struct addrinfo *servinfo;

  memset(&hints, 0, sizeof hints);//empty hints
  hints.ai_family=AF_UNSPEC;//dont to mind it is ipv4 or pv6
  hints.ai_socktype=SOCK_STREAM;//tcp stream socket

  status=getaddrinfo("www.example.net","3490",&hints,&servinfo);

  if((status=getaddrinfo(NULL,"3490",&hints,&servinfo))!=0){
    fprintf(stderr,"getaddrinfo error: %s\n", gai_strerror(status));
    exit(1);
  }*/