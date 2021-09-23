#include "../csapp.h"

int main(int argc, char **argv) {
    int clientfd;
    char *host, *port, buf[MAXLINE];
    rio_t rio;

    if (argc != 3) {
        fprintf(stderr, "usage: %s <host> <port>\n", argv[0]);
        exit(0);
    }
    host = argv[1];
    port = argv[2];

    clientfd = Open_clientfd(host, port);
    printf("clientfd: %d\n", clientfd);
    rio_readinitb(&rio, clientfd);

    // printf("11\n");
    // fgets(buf, MAXLINE, stdin); // 이 라인이 처리될 때까지 다음 라인으로 넘어가지 않음 
    // printf("22\n");

    // stdin이 들어올 때까지 while문은 대기 상태
    // - stdin이 들어와야 fgets 함수가 실행되고, 그래야 fgets함수의 return값을 NULL과 비교할 수 있음 
    while (Fgets(buf, MAXLINE, stdin) != NULL) { 
        printf("121212\n");
        rio_writen(clientfd, buf, strlen(buf));
        rio_readlineb(&rio, buf, MAXLINE);
        Fputs(buf, stdout);
    }
    printf("client closed\n");
    Close(clientfd);
    exit(0);
}