#include <stdio.h>
#include "csapp.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";





void *proxy_thread(void *vargp);
void handle_client(int fd);
void do_get(int fd, char *uri, rio_t *client_rio);
void parse_uri(char *uri, char *hostname, char *port, char *path);
int should_forward(char *header);


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
    char buf[MAXLINE], method[MAXLINE] = {0}, uri[MAXLINE] = {0}, version[MAXLINE] = "HTTP/1.0";
    rio_t rio;

    Rio_readinitb(&rio, fd);
    Rio_readlineb(&rio, buf, MAXLINE);
    if (strlen(buf) >= MAXLINE - 1)
    {
        unix_error("URL too long!");
        return;
    }
    
    int n = sscanf(buf, "%s %s", method, uri);

    if (n < 2)
    {
        unix_error("Invalid request\n");
        return;
    }

    
    if (strcasecmp(method, "GET")) {
        unix_error("Proxy does not implement the method");
        return;
    }
    do_get(fd, uri, &rio);
    printf("Method: %s\n", method);
    printf("URI: %s\n", uri);
    printf("Version: %s\n", version);
}


void do_get(int fd, char *uri, rio_t *client_rio) {
    char hostname[MAXLINE] = "", port[10] = "80", path[MAXLINE];
    char buf[MAXLINE];
    char *headers = (char *)Calloc(MAXLINE, sizeof(char));
    int total = 0, capacity = MAXLINE;
    parse_uri(uri, hostname, port, path);

    while (Rio_readlineb(client_rio, buf, MAXLINE) > 0) {
        if (strcmp(buf, "\r\n") == 0) break;
        int len = strlen(buf);
    
        // extend size if too large the headers is
        if (total + len >= capacity) {
            capacity *= 2;
            headers = Realloc(headers, capacity);
            if (headers == NULL)
            {
                return;
            }
            
        }
        

        // if not has hostname£¬try to get from Host header
        if (strlen(hostname) == 0 && 
            strncasecmp(buf, "Host:", 5) == 0) {
            sscanf(buf, "Host: %s", hostname);
        }
        
        // filter unessential header
        if (should_forward(buf)) {
            strcpy(headers + total, buf);
            total += len;
        }
    }
}



/* parse the url */
void parse_uri(char *uri, char *hostname, char *port, char *path) {
    // handle http://hostname:port/path format
    char *p;
    
    // default port
    strcpy(port, "80");
    
    // jump http://
    if (strncasecmp(uri, "http://", 7) == 0) {
        p = uri + 7;
    } else {
        p = uri;
    }
    
    // get the path
    char *path_start = strchr(p, '/');
    if (path_start) {
        
        strcpy(path, path_start);
        *path_start = '\0';
    } else {
        strcpy(path, "/");
    }
    
    // parse host and the port
    char *colon = strchr(p, ':');
    if (colon) {
        *colon = '\0';
        strcpy(hostname, p);
        strcpy(port, colon + 1);
        *colon = ':';
    } else {
        strcpy(hostname, p);
    }
}

/* judge if need to add to headers */
int should_forward(char *header) {
    // don't add
    if (!strncasecmp(header, "Connection:", strlen("Connection:")))
        return 0;
    if (!strncasecmp(header, "Proxy-Connection:", strlen("Proxy-Connection:")))
        return 0;
    if (!strncasecmp(header, "User-Agent:", strlen("User-Agent:")))
        return 0;
    return 1;
}

