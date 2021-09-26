#include <stdio.h>
#include "csapp.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr =
    "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 "
    "Firefox/10.0.3\r\n";
static const char *conn_hdr = "Connection: close\r\n";
static const char *prox_hdr = "Proxy-Connection: close\r\n";
static const char *host_hdr_format = "Host: %s\r\n"; // HOST: <host>:<port(optional)>
static const char *requestline_hdr_format = "GET %s HTTP/1.0\r\n";
static const char *endof_hdr = "\r\n";

static const char *connection_key = "Connection";
static const char *user_agent_key = "User_Agent";
static const char *proxy_connection_key = "Proxy-Connection";
static const char *host_key = "Host";


/* 함수 형태 정의 */
void doit(int connfd);
void parse_uri(char *uri, char *hostname, char *path, int *port);
void build_http_header(char *http_header, char *hostname, char *path, int port, rio_t *fromcli_rio);
int connect_to_server(char *hostname, int port, char *http_header);
void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg);


/* main */
int main(int argc, char **argv) {
  int listenfd, connfd;
  char hostname[MAXLINE], port[MAXLINE];
  socklen_t clientlen;
  struct sockaddr_storage clientaddr;

  /* check command line args */
  if (argc != 2) {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(1);
  }

  listenfd = Open_listenfd(argv[1]);
  while(1) {
    clientlen = sizeof(clientaddr);
    printf("\nwait for client's requests... (%d)\n", clientlen);
    connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);  // repeatedly accepting a connection request
    Getnameinfo((SA *)&clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE, 0);
    printf("Accepted connection from (%s, %s)\n", hostname, port);
    doit(connfd);  // performing transaction 
    Close(connfd);
  }

  printf("%s", user_agent_hdr);
  printf("SERVER CLOSED");
  return 0;
}


/* doit : handle the client HTTP transaction */
void doit(int connfd) {
  printf("[doit] doit doit chu~~ (fd: %d)\n", connfd);
  // proxy 관점에서 최종서버가 되는 최종 목적지의 fd (여기는 proxy)
  int tosvrfd; 
  // 클라이언트로 받은 요청의 정보
  char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
  char hostname[MAXLINE], path[MAXLINE];
  int port;
  // 서버로 보낼 request header
  char server_http_header[MAXLINE];
  // rio
  rio_t fromcli_rio, tosvr_rio;
  // 클라이언트에서 받은 request headers 확인
  Rio_readinitb(&fromcli_rio, connfd);  // connfd를 fromcli_rio structure에 연결
  Rio_readlineb(&fromcli_rio, buf, MAXLINE);  // connfd에 담긴 클라이언트 요청의 첫 번째 라인을 buf에 저장
  printf("Request headers: \n");
  printf("%s", buf);
  // 가령, GET http://www.cmu.edu/hub/index.html HTTP/1.1
  sscanf(buf, "%s %s %s", method, uri, version);
  // method가 GET이 아닌 경우 에러 처리
  if (strcasecmp(method, "GET")) { // case-insensitive sting comparisons
    clienterror(connfd, method, "501", "Not implemented", "Proxy does not implement this method");
    return;
  }
  // parse URI from GET request
  parse_uri(uri, hostname, path, &port);
  // 서버로 요청할 헤더 구성
  build_http_header(server_http_header, hostname, path, port, &fromcli_rio);
  // 서버와 연결: 프록시가 다시 서버에 요청
  tosvrfd = connect_to_server(hostname, port, server_http_header);
  if (tosvrfd < 0) {
    clienterror(connfd, method, "404", "Not Found", "Proxy couldn't find the requested page");
    return;
  }
  // 서버와 연결된 fd(socket)을 tosvr_rio에 연결
  Rio_readinitb(&tosvr_rio, tosvrfd);
  // 서버에 헤더 전달
  Rio_writen(tosvrfd, server_http_header, strlen(server_http_header));
  // 서버로부터 받은 데이터를 다시 클라이언트에 전달
  size_t n;
  while((n = Rio_readlineb(&tosvr_rio, buf, MAXLINE)) != 0) {
    printf("server to proxy, then proxy to server: %d bytes\n", n);
    Rio_writen(connfd, buf, n);
  }
  Close(tosvrfd);
}


void parse_uri(char *uri, char *hostname, char *path, int *port) {
  // uri가 http://www.cmu.edu/hub/index.html 와 같은 형태라고 기대됨
  char *pos_hostname = strstr(uri, "//");
  pos_hostname = pos_hostname != NULL ? pos_hostname+2 : uri;
  char *pos_port = strstr(pos_hostname, ":");
  // port가 명시된 경우, 가령 www.naver.com:80/blah/blah
  // - port와 path 확보
  if (pos_port != NULL) {
    *pos_port = '\0';
    sscanf(pos_hostname, "%s", hostname);
    sscanf(pos_port+1, "%d%s", port, path);
  } 
  // port가 명시되지 않은 경우
  // - port를 80으로 설정 후 path 확보
  else {
    *port = 80;
    pos_port = strstr(pos_hostname, "/");
    if (pos_port != NULL) {
      *pos_port = '\0';
      sscanf(pos_hostname, "%s", hostname);
      *pos_port = '/';
      sscanf(pos_port, "%s", path);
    } else {
      sscanf(pos_hostname, "%s", hostname);
    }
  }
  return;
}


void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg) {
  printf("[clienterror] %s %s\n", cause, shortmsg);
  char buf[MAXLINE], body[MAXBUF];

  /* Build the HTTP response body */
  sprintf(body, "<html><title>Proxy Error</title>");
  sprintf(body, "%s<body bgcolor=""ffffff"">\r\n", body);
  sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
  sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);  
  sprintf(body, "%s<hr><em>The Proxy Web server</em>\r\n", body);

  /* Print the HTTP response */
  sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
  rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "Content-type: text/html\r\n");
  rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "Content-length: %d\r\n\r\n", (int)strlen(body));
  rio_writen(fd, buf, strlen(buf));
  rio_writen(fd, body, strlen(body));
}