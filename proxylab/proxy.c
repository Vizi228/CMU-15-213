#include <stdio.h>
#include "csapp.h"
#include "cache.h"

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";

typedef struct {
    char hostname[100];
    char path[400];
    char user_agent[400];
    char buf[MAXLINE];
} headers;

int startsWith(const char *pre, const char *str);
void *redirect(void *fd);
void read_requesthdrs(rio_t *rp, headers *hdrs);
void clienterror(int fd, char *cause, char *errnum,
                 char *shortmsg, char *longmsg);

void sigpipe_handler(int sig) {
    return;
}

int main(int argc, char **argv)
{
    int listenfd, *connfd;
    char hostname[MAXLINE], port[MAXLINE];
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    pthread_t tid;

    /* Check command line args */
    if (argc != 2)
    {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(1);
    }

    Signal(SIGPIPE, sigpipe_handler);
    init_cache();

    listenfd = Open_listenfd(argv[1]);
    while (1)
    {
        clientlen = sizeof(clientaddr);
        connfd = Malloc(sizeof(int));
        *connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
        Getnameinfo((SA *)&clientaddr, clientlen, hostname, MAXLINE,
                    port, MAXLINE, 0);
        printf("Accepted connection from (%s, %s)\n", hostname, port);
        Pthread_create(&tid, NULL, redirect, connfd);
    }
}

void *redirect(void *fd)
{
    int connfd = *((int *)fd);
    Pthread_detach(pthread_self());
    Free(fd);
    Signal(SIGPIPE, sigpipe_handler);

    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
    rio_t rio;
    headers hdrs;
    memset(&hdrs, 0, sizeof(headers));

    /* Read request line and headers */
    Rio_readinitb(&rio, connfd);
    if (!Rio_readlineb(&rio, buf, MAXLINE))
        return NULL;
    printf("%s", buf);
    sscanf(buf, "%s %s %s", method, uri, version);
    if (strcasecmp(method, "GET"))
    {
        clienterror(connfd, method, "501", "Not Implemented",
                    "Proxy does not implement this method");
        return NULL;
    }
    if (startsWith("https://", uri)) {
        clienterror(connfd, method, "501", "Not Implemented",
                    "Proxy does not work with https protocol");
        return NULL;
    }

    read_requesthdrs(&rio, &hdrs);

    if (startsWith("http://", uri)) {
        sscanf(uri, "http://%99[^/]%99s", hdrs.hostname, hdrs.path);
    } else {
        strncpy(hdrs.path, uri, strlen(uri)); 
    }

    if (strlen(hdrs.user_agent) < 1) {
        strncpy(hdrs.user_agent, user_agent_hdr, strlen(user_agent_hdr));
    }

    char host[100], port[100];
    sscanf(hdrs.hostname, "%99[^:]:%99s", host, port);
    if (!strlen(port)) {
        sprintf(port, "80");
    }

    char url[MAXLINE];
    memset(url, 0, MAXLINE);
    strncat(url, host, strlen(host));
    strncat(url, port, strlen(port));
    strncat(url, hdrs.path, strlen(hdrs.path));
    struct cache_block *block;
    if ((block = read_cache(url)) != NULL) {
        Rio_writen(connfd, block->data, block->data_size);
        return NULL;
    } 

    int cfd;
    if ((cfd = open_clientfd(host, port)) < 0) {
        clienterror(connfd, method, "404", "Not found", "Coudn't open connection with client!\n");
        return NULL;
    } 
    
    sprintf(buf, "GET %s HTTP/1.0\r\n", hdrs.path);
    Rio_writen(cfd, buf, strlen(buf));
    sprintf(buf, "Host: %s\r\n", hdrs.hostname);
    Rio_writen(cfd, buf, strlen(buf));
    sprintf(buf, "User-Agent: %s\r\n", hdrs.user_agent);
    Rio_writen(cfd, buf, strlen(buf));
    sprintf(buf, "Connection: close\r\n");
    Rio_writen(cfd, buf, strlen(buf));
    sprintf(buf, "Proxy-connection: close\r\n");
    Rio_writen(cfd, buf, strlen(buf));
    Rio_writen(cfd, hdrs.buf, strlen(hdrs.buf));
    
    ssize_t res;
    char cache_data[MAX_OBJECT_SIZE];
    memset(cache_data, 0, MAX_OBJECT_SIZE);
    int cache_data_size = 0;
    Rio_readinitb(&rio, cfd);
    while((res = Rio_readlineb(&rio, buf, MAXLINE)) != 0) {
        cache_data_size += res;
        if (cache_data_size <= MAX_OBJECT_SIZE) {
            memcpy(cache_data + (cache_data_size - res), buf, res);
        }        
        Rio_writen(connfd, buf, res);
    }
    write_cache(url, cache_data, cache_data_size);

    Close(connfd);
    Close(cfd);
    return NULL;
}

int startsWith(const char *pre, const char *str)
{
    size_t lenpre = strlen(pre),
           lenstr = strlen(str);
    return lenstr < lenpre ? 0 : memcmp(pre, str, lenpre) == 0;
}

void read_requesthdrs(rio_t *rp, headers *hdrs)
{
    char buf[MAXLINE];

    do {
        Rio_readlineb(rp, buf, MAXLINE);
        printf("%s", buf);
        if (startsWith("Host", buf)) {
            sscanf(buf, "Host:%s", hdrs->hostname);
        } else if (
            startsWith("User-Agent", buf)
        ) {
            sscanf(buf, "User-Agent:%s", hdrs->user_agent);
        } else if (
            !startsWith("Connection", buf) &&
            !startsWith("Proxy-Connection", buf)
        ) {
            strncat(hdrs->buf, buf, strlen(buf));
        }
    } while(strcmp(buf, "\r\n"));

    return;
}

void clienterror(int fd, char *cause, char *errnum,
                 char *shortmsg, char *longmsg)
{
    char buf[MAXLINE];

    /* Print the HTTP response headers */
    sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-type: text/html\r\n\r\n");
    Rio_writen(fd, buf, strlen(buf));

    /* Print the HTTP response body */
    sprintf(buf, "<html><title>Proxy Error</title>");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "<body bgcolor="
                 "ffffff"
                 ">\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "%s: %s\r\n", errnum, shortmsg);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "<p>%s: %s\r\n", longmsg, cause);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "<hr><em>The Proxy Web server</em>\r\n");
    Rio_writen(fd, buf, strlen(buf));
}
