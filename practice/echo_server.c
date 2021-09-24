#include "../csapp.h"

void echo(int connfd);

int main(int argc, char **argv) {
    int listenfd, connfd;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr; // Enough space for any address
    char client_hostname[MAXLINE], client_port[MAXLINE];

    // 입력값 체크
    if (argc != 2) {
            fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(0);
    }
    // listenfd 생성: port 번호만 필요
    listenfd = Open_listenfd(argv[1]);
    // infinite loop에 진입하여, 각 회기에서 client의 요청을 기다림
    while(1) {
        // 한 번에 하나의 클라이언트만 처리됨
        // - 이미 하나의 클라이언트와 연결된 상태에서 새로운 클라이언트가 연결 요청이 들어오면
        // - 새로운 클라이언트는 기존의 클라이언트와의 연결이 끊길 때까지 대기 상태가 됨
        printf("wait for client's request..\n");        
        clientlen = sizeof(struct sockaddr_storage);
        printf("hmm..%lu\n", clientlen);
        // Accept에서 clientaddr이 잡힐 때까지 기다리는 듯함
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
        Getnameinfo((SA *) &clientaddr, clientlen, client_hostname, MAXLINE, client_port, MAXLINE, 0);
        printf("Connected to (%s, %s)\n", client_hostname, client_port);
        echo(connfd);
        Close(connfd);
    }
    printf("server closed");
    exit(0);    
}

void echo(int connfd) {
    printf("echo start! with connfd %d\n", connfd);
    size_t n;
    char buf[MAXLINE];
    rio_t rio;

    // rio structure에 connfd를 연결
    rio_readinitb(&rio, connfd);
    // 
    while((n = rio_readlineb(&rio, buf, MAXLINE)) != 0) {
        printf("server received %d bytes\n", (int)n);
        printf("before : %s", buf); // buf에는 \n까지 포함됨!
        buf[0] = 'j';
        printf("after  : %s", buf);
        rio_writen(connfd, buf, n);
    }
}