/*************************************************************************
	> File Name: selectServer.c
	> Author: lidongmeng
	> Mail:lidongmeng0213@163.com 
	> Created Time: Fri 11 Sep 2015 04:37:43 AM PDT
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <errno.h>
#include <string.h>

#define MAXLINE 1024

void handle(int * clientSockFd, int maxfd, fd_set *rset, fd_set * allset);

int main(int argc, char ** argv) {
	int serverPort = 6888;
	int listenBackLog = 1024;

	int listenfd, connfd;
	struct sockaddr_in serverAddr, clientAddr;
	socklen_t clientLen = sizeof(struct sockaddr_in);
	int nready, nread;
	char buf[MAXLINE];
	int clientSockFd[FD_SETSIZE];
	fd_set allset, rset;
	int maxfd;

	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	if (listenfd < 0) {
		perror("listen error");
		return -1;
	}

	int opt = 1;
	if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
		perror("set sockopt error");
	}

	bzero(&serverAddr, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddr.sin_port = htons(serverPort);

	if (bind(listenfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
		perror("bind error");
		return -1;
	}

	if (listen(listenfd, listenBackLog) < 0) {
		perror("listen error");
		return -1;
	}

	int i = 0;
	for (i = 0; i < FD_SETSIZE; ++i) {
		clientSockFd[i] = -1;
	}
	FD_ZERO(&allset);
	FD_SET(listenfd, &allset);
	maxfd = listenfd;

	while (1) {
		rset = allset;
		nready = select(maxfd+1, &rset, NULL, NULL, NULL);
		if (nready < 0) {
			perror("select error");
			continue;
		}

		if (FD_ISSET(listenfd, &rset)) {
			connfd = accept(listenfd, (struct sockaddr*)&clientAddr, &clientLen);
			if (connfd < 0) {
				perror("accept error");
				continue;
			}

			sprintf(buf, "accept form %s:%d\n", inet_ntoa(clientAddr.sin_addr), clientAddr.sin_port);
			printf(buf, "");

			for (i = 0; i < FD_SETSIZE; ++i) {
				if (clientSockFd[i] == -1) {
					clientSockFd[i] = connfd;
					break;
				}
			}

			if (i == FD_SETSIZE) {
				printf("Too many connection\n");
				close(connfd);
				continue;
			}

			if (connfd > maxfd) {
				maxfd = connfd;
			}

			FD_SET(connfd, &allset);
			if (--nready <= 0)
				continue;
		}
		handle(clientSockFd, maxfd, &rset, &allset);
	}
}


void handle(int * clientSockFds, int maxfd, fd_set * rset, fd_set * allset) {
	int nread;
	int i;
	char buf[MAXLINE];
	for (i = 0; i < maxfd; ++i) {
		if (FD_ISSET(clientSockFds[i], rset)) {
			nread = read(clientSockFds[i], buf, MAXLINE);
			if (nread < 0) {
				perror("read error");
				close(clientSockFds[i]);
				FD_CLR(clientSockFds[i], allset);
				clientSockFds[i] = -1;
				continue;
			}
			if (nread == 0) {
				printf("client closet the connection\n");
				close(clientSockFds[i]);
				FD_CLR(clientSockFds[i], allset);
				clientSockFds[i] = -1;
				continue;
			}
			buf[strlen(buf)-1] = '\0';
			printf("receive from %d:[%s]\n", clientSockFds[i], buf);
			write(clientSockFds[i], buf, nread);
		}
	}
}
