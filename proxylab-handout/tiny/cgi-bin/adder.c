/*
 * adder.c - a minimal CGI program that adds two numbers together
 */
/* $begin adder */
#include "csapp.h"

int main(void) {
    char *buf, *p;
    char arg1[100000], arg2[100000], content[100000];
    int n1=0, n2=0;

    /* Extract the two arguments */
    if ((buf = getenv("QUERY_STRING")) != NULL) {
	p = strchr(buf, '&');
	*p = '\0';
	strcpy(arg1, buf);
	strcpy(arg2, p+1);
	n1 = atoi(arg1);
	n2 = atoi(arg2);
    }

    /* Make the response body */
     strcpy(content, "Welcome to add.com: ");  // 初始化
    strcat(content, "THE Internet addition portal.\r\n<p>");  // 追加
    
    char result[MAXLINE];
    sprintf(result, "The answer is: %d + %d = %d\r\n<p>", n1, n2, n1 + n2);
    strcat(content, result);  // 追加结果
    
    strcat(content, "Thanks for visiting!\r\n");  // 追加结尾
    /* Generate the HTTP response */
    printf("Connection: close\r\n");
    printf("Content-length: %d\r\n", (int)strlen(content));
    printf("Content-type: text/html\r\n\r\n");
    printf("%s", content);
    fflush(stdout);

    exit(0);
}
/* $end adder */
