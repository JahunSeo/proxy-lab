/*
 * adder.c - a minimal CGI program that adds two numbers together
 */
/* $begin adder */
#include "../csapp.h"

int main(void) {
  char *buf, *p1, *p2;
  char arg1[MAXLINE], arg2[MAXLINE], content[MAXLINE];
  int n1=0, n2=0;
  /* Extract the two arguments */
  if ((buf = getenv("QUERY_STRING")) != NULL) {
    p1 = strstr(buf, "n1=");
    p2 = strstr(buf, "n2=");
    if (!p1 || !p2) {
      sprintf(content, "Welcome to add.com: ");
      sprintf(content, "%sTHE Internet addition portal.\r\n<p>", content);
      sprintf(content, "%sParameters should be n1={number}&n2={number}\r\n<p>", content, n1, n2, n1+n2);
      sprintf(content, "%sThanks for visiting!\r\n", content);
      /* Generate the HTTP response */
      printf("Connection: close\r\n");
      printf("Content-length: %d\r\n", (int)strlen(content));
      printf("Content-type: text/html\r\n\r\n");
      printf("%s", content);
      fflush(stdout);

      exit(0);
    }
    strcpy(arg1, p1+3);
    strcpy(arg2, p2+3);
    n1 = atoi(arg1); // ascii to integer
    n2 = atoi(arg2);
  }
  /* Make the response body */
  sprintf(content, "QUERY_STRING=%s", buf);
  sprintf(content, "Welcome to add.com: ");
  sprintf(content, "%sTHE Internet addition portal.\r\n<p>", content);
  sprintf(content, "%sThe answer is %d + %d = %d\r\n<p>", content, n1, n2, n1+n2);
  sprintf(content, "%sThanks for visiting!\r\n", content);
  /* Generate the HTTP response */
  printf("Connection: close\r\n");
  printf("Content-length: %d\r\n", (int)strlen(content));
  printf("Content-type: text/html\r\n\r\n");
  printf("%s", content);
  fflush(stdout);

  exit(0);
}
/* $end adder */
