#include "../csapp.h"

// struct addrinfo {
//   	int	ai_flags;	/* AI_PASSIVE, AI_CANONNAME, AI_NUMERICHOST */
// 	    int	ai_family;	/* PF_xxx */
// 	    int	ai_socktype;	/* SOCK_xxx */
// 	    int	ai_protocol;	/* 0 or IPPROTO_xxx for IPv4 and IPv6 */
// 	    socklen_t ai_addrlen;	/* length of ai_addr */
// 	    char	*ai_canonname;	/* canonical name for hostname */
// 	    struct	sockaddr *ai_addr;	/* binary address */
// 	    struct	addrinfo *ai_next;	/* next structure in linked list */
// };

int main(int argc, char **argv) {
    printf("hello hostinfo\n");
    printf("%d %s\n", argc, argv[0]);
    struct addrinfo *p, *listp, hints;
    char buf[MAXLINE]; // MAXLINE == 8192 == 2**13
    int rc, flags;

    if (argc != 2) {
        fprintf(stderr, "usage: %s <domain name>\n", argv[0]);
        exit(0); // exit success
    }

    /* Get a list of addrinfo records */
    memset(&hints, 0, sizeof(struct addrinfo)); // hints의 시작 주소값부터 addrinfo의 크기 만큼 1바이트 단위로 0으로 변경
    hints.ai_family = AF_INET;  /* IPv4 only */
    hints.ai_socktype = SOCK_STREAM;  /* connection only */
    if ((rc = getaddrinfo(argv[1], NULL, &hints, &listp)) != 0) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(rc)); // gai_strerror(rc)는 error code를 문자열로 변환
        exit(1);
    }

    /* Walk the list and display each IP address */
    flags = NI_NUMERICHOST; /* Display address string instead of domain name */
    for (p = listp; p; p = p->ai_next) {
        Getnameinfo(p->ai_addr, p->ai_addrlen, buf, MAXLINE, NULL, 0, flags);
        printf("%s\n", buf);
    }

    /* Clean up */
    Freeaddrinfo(listp);

    exit(0);
}