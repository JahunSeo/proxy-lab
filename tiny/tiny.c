/* $begin tinymain */
/*
 * tiny.c - A simple, iterative HTTP/1.0 Web server that uses the
 *     GET method to serve static and dynamic content.
 *
 * Updated 11/2019 droh
 *   - Fixed sprintf() aliasing issue in serve_static(), and clienterror().
 */
#include "csapp.h"

void doit(int fd);
void read_requesthdrs(rio_t *rp);
int parse_uri(char *uri, char *filename, char *cgiargs);
void serve_static(int fd, char *filename, int filesize);
void get_filetype(char *filename, char *filetype);
void serve_dynamic(int fd, char *filename, char *cgiargs);
void clienterror(int fd, char *cause, char *errnum, char *shortmsg,
                 char *longmsg);

int main(int argc, char **argv) {
  int listenfd, connfd;
  char hostname[MAXLINE], port[MAXLINE];
  socklen_t clientlen;
  struct sockaddr_storage clientaddr;

  /* Check command line args */
  if (argc != 2) {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(1);
  }

  listenfd = Open_listenfd(argv[1]);
  while (1) {
    clientlen = sizeof(clientaddr);
    printf("\nwait for client's requests... (%d)\n", clientlen);
    connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);  // repeatedly accepting a connection request // line:netp:tiny:accept
    Getnameinfo((SA *)&clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE, 0);
    printf("Accepted connection from (%s, %s)\n", hostname, port);
    doit(connfd);  // performing transaction  // line:netp:tiny:doit
    Close(connfd);  // line:netp:tiny:close
  }
  printf("server closed");
}


void doit(int fd) {
  printf("[doit] doit doit chu~~ (fd: %d)\n", fd);
  int is_static;  // client의 요청이 static content인지 여부
  struct stat sbuf;  // status of file을 저장하는 structure
  char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
  char filename[MAXLINE], cgiargs[MAXLINE];
  rio_t rio;

  /* Read request line and headers */
  rio_readinitb(&rio, fd);  // paremeter connfd를 rio structure에 연결
  rio_readlineb(&rio, buf, MAXLINE);  // connfd에 담겨 있는 client request의 첫 번째 라인을 buf에 저장
  printf("Request headers:\n");
  printf("%s", buf);
  sscanf(buf, "%s %s %s", method, uri, version);
  /* method가 GET이 아닌 경우 에러 처리 */
  if (strcasecmp(method, "GET")) { // case-insensitive sting comparisons
    clienterror(fd, method, "501", "Not implemented", "Tiny does not implement this method");
    return;
  }
  /* request header 읽기: 이건 rio의 fd를 비우기 위해 실행하는 걸까? */
  read_requesthdrs(&rio);
  /* Parse URI from GET request */
  is_static = parse_uri(uri, filename, cgiargs);

}


void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg) {
  printf("[clienterror] %s %s\n", cause, shortmsg);
  char buf[MAXLINE], body[MAXBUF];

  /* Build the HTTP response body */
  sprintf(body, "<html><title>Tiny Error</title>");
  sprintf(body, "%s<body bgcolor=""ffffff"">\r\n", body);
  sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
  sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);  
  sprintf(body, "%s<hr><em>The Tiny Web server</em>\r\n", body);

  /* Print the HTTP response */
  sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
  rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "Content-type: text/html\r\n");
  rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "Content-length: %d\r\n\r\n", (int)strlen(body));
  rio_writen(fd, buf, strlen(buf));
  rio_writen(fd, body, strlen(body));
}


void read_requesthdrs(rio_t *rp) {
  char buf[MAXLINE];

  rio_readlineb(rp, buf, MAXLINE);
  while(strcmp(buf, "\r\n")) {  // compare two strings char by char, if string are equal return 0
    rio_readlineb(rp, buf, MAXLINE);
    printf("%s", buf);
  }
  return;
}


int parse_uri(char *uri, char *filename, char *cgiargs) {

  char *ptr;
  
  if (!strstr(uri, "cgi-bin")) {  // substring 찾기: uri에서 "cgi-bin"이 처음 등장하는 위치의 주소값, 없으면 NULL
    /* Static content */
    strcpy(cgiargs, "");
    strcpy(filename, ".");  // 현재 디렉토리 뒤에 uri를 붙임: ./blah/blah
    strcat(filename, uri); 
    if (uri[strlen(uri)-1] == '/') {
      strcat(filename, "home.html");  // expand to some default home page
    }
    return 1;
  } 
  else {
    /* Dynamic content */
    ptr = index(uri, '?');
    if (ptr) {
      strcpy(cgiargs, ptr+1);
      *ptr = '\0';
    } else {
      strcpy(cgiargs, "");
    }
    strcpy(filename, ".");  // 현재 디렉토리 뒤에 uri를 붙임: ./blah/blah
    strcat(filename, uri);
    return 0;
  } 
}