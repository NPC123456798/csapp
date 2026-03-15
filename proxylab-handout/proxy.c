#include <stdio.h>
#include "csapp.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";





void * proxy_thread(void *vargp);
void handle_client(int fd);





int main(int argc, char **argv)
{
    signal(SIGPIPE, SIG_IGN);
    
    int listenfd, * connfd; // connfd is unique corresponding for a file just like any other normal file.
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;

    pthread_t tid;
    
    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(1);
    }
    
    listenfd = Open_listenfd(argv[1]);
    while(1) {
        clientlen = sizeof(clientaddr);
        connfd = Malloc(sizeof(int));
        *connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
        Pthread_create(&tid, NULL, proxy_thread, connfd);
    }

    printf("%s", user_agent_hdr);
    return 0;
}


void * proxy_thread(void *vargp) 
{
    int connfd = *((int *)vargp);
    Pthread_detach(pthread_self());
    Free(vargp);



    handle_client(connfd);




    Close(connfd);

    return NULL;
}

/* handle_client - Handle a client request */
void handle_client(int fd) 
{
    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
    rio_t rio;

    Rio_readinitb(&rio, fd);
    Rio_readlineb(&rio, buf, MAXLINE);
    sscanf(buf, "%s %s %s", method, uri, version);

    if (strcasecmp(method, "GET")) {
        printf("Proxy does not implement the method");
        return;
    }

    printf("Method: %s\n", method);
    printf("URI: %s\n", uri);
    printf("Version: %s\n", version);
}