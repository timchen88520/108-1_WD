#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>

#define PORT 3333

char *sys_os_get_socket_error() {
  char *p_estr = NULL;

#if __LINUX_OS__

  p_estr = strerror(errno);

#elif __WIN32_OS__

  int err = WSAGetLastError();
  static char serr_code_buf[24];
  sprintf(serr_code_buf, "WSAE-%d", err);
  p_estr = serr_code_buf;

#endif

  return p_estr;
}

//创建socket
int socketCreate(int *pFD) {
  int val = 1;
  int reuse_ret;
  struct sockaddr_in addr;

  *pFD = socket(AF_INET, SOCK_STREAM, 0);
  if (*pFD <= 0) {
    printf("socketCreate::socket err[%s]!!!\r\n", sys_os_get_socket_error());
    return -1;
  }

  reuse_ret = setsockopt(*pFD, SOL_SOCKET, SO_REUSEADDR, (char *)&val, 4);

  bzero(&addr, sizeof(struct sockaddr_in));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = htons(PORT);

  if (bind(*pFD, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
    printf("bind tcp socket fail, err[%s]!!!\n", sys_os_get_socket_error());

    close(*pFD);
    *pFD = 0;
    return -1;
  }

  if (listen(*pFD, 10) < 0) {
    printf("socketCreate::listen tcp socket fail,err[%s]!!!\r\n",
           sys_os_get_socket_error());
    close(*pFD);
    return -1;
  }

  return 0;
}

//发送图片
int sendPic(int *pFD) {
  int fd = *pFD;
  FILE *fp;
  int len;

  char readBuf[600 * 1024] = {0};  // 600k

  int tlen;
  char *p_bufs = NULL;

  /* read data */
  fp = fopen("./snapshot.jpg", "rb");
  if (NULL == fp) {
    return -1;
  }
  fseek(fp, 0, SEEK_END);

  len = ftell(fp);
  if (len <= 0) {
    fclose(fp);
    return -1;
  }
  fseek(fp, 0, SEEK_SET);

  len = fread(readBuf, 1, len, fp);
  fclose(fp);

  /* send data */
  p_bufs = (char *)malloc(len + 1024);
  if (NULL == p_bufs) {
    printf("malloc error!\n");
    return -1;
  }

  tlen = sprintf(p_bufs,
                 "HTTP/1.1 200 OK\r\n"
                 "Server: hsoap/2.8\r\n"
                 "Content-Type: image/jpeg\r\n"
                 "Content-Length: %d\r\n"
                 "Connection: close\r\n\r\n",
                 len);

  memcpy(p_bufs + tlen, readBuf, len);
  tlen += len;

  send(fd, p_bufs, tlen, 0);

  free(p_bufs);

  return 0;
}

//发送文字
int sendTxt(int *pFD) {
  int fd = *pFD;
  int len;

  // char readBuf[100] = "Just test!";
  char readBuf[100] =
      "<html>\
<head>\
    <title> 我是标题 </title>\
</head>\
<body>\
    <p> 测试测试, Just test! <p>\
</body>\
</html>";

  int tlen;
  char *p_bufs = NULL;

  len = strlen(readBuf);
  /* send data */
  p_bufs = (char *)malloc(len + 1024);
  if (NULL == p_bufs) {
    printf("malloc error!\n");
    return -1;
  }

  tlen = sprintf(p_bufs,
                 "HTTP/1.1 200 OK\r\n"
                 "Server: hsoap/2.8\r\n"
                 "Content-Type: text/html\r\n"
                 "Content-Length: %d\r\n"
                 "Connection: close\r\n\r\n",
                 len);

  memcpy(p_bufs + tlen, readBuf, len);
  tlen += len;

  send(fd, p_bufs, tlen, 0);

  free(p_bufs);

  return 0;
}

//接收数据
int socketRecv(int *pFD) {
  int fd = *pFD;
  fd_set fdr;
  int sret;
  struct timeval tv;

  int cfd;
  struct sockaddr_in caddr;
  socklen_t size;

  int rlen = 0;
  char rBuf[100 * 1024] = {0};

  while (1) {
    FD_ZERO(&fdr);
    FD_SET(fd, &fdr);

    tv.tv_sec = 0;
    tv.tv_usec = 200 * 1000;

    sret = select((int)(fd + 1), &fdr, NULL, NULL, &tv);
    if (sret == 0) {
      // timeout
      // return 0;
      continue;
    } else if (sret < 0) {
      printf("socketRecv::select err[%s]\r\n", sys_os_get_socket_error());
      // return -1;
      continue;
    }

    if (FD_ISSET(fd, &fdr)) {
      size = sizeof(struct sockaddr_in);
      cfd = accept(fd, (struct sockaddr *)&caddr, &size);
      if (cfd <= 0) {
        printf("socketRecv::accept\r\n");
        // return -1;
        continue;
      }

      rlen = recv(cfd, rBuf, sizeof(rBuf) - 1, 0);
      if (rlen <= 0) {
        continue;
      }

      printf("get %d bytes of normal data: %s \n", rlen, rBuf);

      //解析数据

      if (strstr(rBuf, "GET /test/jpg")) {
        sendPic(&cfd);
      } else if (strstr(rBuf, "GET /test/txt")) {
        sendTxt(&cfd);
      }

      close(cfd);
    }
  }

  return 0;
}

// http://192.168.123.100:3333/test/txt
// http://192.168.123.100:3333/test/jpg
int main() {
  int mFD = 0;
  printf("photo.c\n");
  if (0 != socketCreate(&mFD)) {
    printf("Create Socket Error!\n");
    return -1;
  }

  socketRecv(&mFD);

  return 0;
}