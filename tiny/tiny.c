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
  /* Parse URI from GET request: 클라이언트로부터 요청 받은 filename과 cgiargs를 확인 */
  is_static = parse_uri(uri, filename, cgiargs);
  printf("[parse_uri] %d, %s %s\n", is_static, filename, cgiargs);
  /* 요청 받은 파일이 존재하는지 확인 */
  // stat은 파일의 상태 및 정보를 가져오는 함수, 성공 시 0, 실패 시 -1
  if (stat(filename, &sbuf) < 0) {
    clienterror(fd, filename, "404", "Not found", "Tiny couldn't find this file");
    return;
  }
  /*  */
  if (is_static) {
    /* serve static content */
    if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode)) {
      clienterror(fd, filename, "403", "Forbidden", "Tiny couldn't read the file");
      return;
    }
    serve_static(fd, filename, sbuf.st_size);
  } else {
    /* serve dynamic content */

  }

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
    strcat(filename, uri);  // 참고로, uri에 cgi-bin이 없을 경우, 즉 dynamic이 아닌 경우, '?' 뒤의 내용도 모두 filename 들어 간다
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


void serve_static(int fd, char *filename, int filesize) {
  int srcfd;
  char *srcp, filetype[MAXLINE], buf[MAXBUF];

  /* Send response headers to client */
  get_filetype(filename, filetype);
  sprintf(buf, "HTTP/1.0 200 OK\r\n");
  sprintf(buf, "%sServer: Tiny Web Server\r\n", buf);
  sprintf(buf, "%sConnection: close\r\n", buf);
  sprintf(buf, "%sContent-length: %d\r\n", buf, filesize);
  sprintf(buf, "%sContent-type:%s\r\n\r\n", buf, filetype);
  rio_writen(fd, buf, strlen(buf)); // 구성한 header를 일차로 client에 보냄
  printf("Response headers:\n");
  printf("%s", buf);

  /* Send response bod to client */
  srcfd = Open(filename, O_RDONLY, 0);
  srcp = Mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);
  Close(srcfd);
  rio_writen(fd, srcp, filesize);
  munmap(srcp, filesize);

}


void get_filetype(char *filename, char *filetype) {
  /* Derive file type from filename */
  if (strstr(filename, ".html")) {
    strcpy(filetype, "text/html");
  } else if (strstr(filename, ".gif")) {
    strcpy(filetype, "image/gif");
  } else if (strstr(filename, ".png")) {
    strcpy(filetype, "image/png");
  } else if (strstr(filename, ".jpg")) {
    strcpy(filetype, "image/jpeg");
  } else {
    strcpy(filetype, "text/plain");
  }
}