/**
 * Library for socket programming functions
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <syslog.h>
#include <signal.h>
#include <sys/wait.h>
#include <netdb.h>
#include <errno.h>

#ifdef	HAVE_SOCKADDR_DL_STRUCT
#include	<net/if_dl.h>
#endif


#define MAXFD   64
#define MAXLINE 1024

#define SERVER_IP    "127.0.0.1"
#define SERVER_PORT  "3030"
#define LISTENQ      8
#define UNIXPATH    "/tmp/unixdomain"


int  Socket(int domain, int type, int protocol);
void Bind(int sockfd, struct sockaddr *addr, int size);
void Listen(int sockfd, int backlog);
int  Accept(int sockfd, struct sockaddr *addr, socklen_t *size);
void Connect(int sockfd, struct sockaddr *addr, socklen_t size);
int  Inet_pton(int af, const char *src, void *dst);
int  Tcp_connect(char *host, char *service);
int  Tcp_listen(char *host, char *service, socklen_t *socklen);
int  Daemon_init(const char *ident);
int  Uxd_listen(const char *path);
int  Uxd_connect(const char *path);
