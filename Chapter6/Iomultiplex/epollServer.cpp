/*************************************************************************
	> File Name: epollServer.c
	> Author: lidongmeng
	> Mail:lidongmeng0213@163.com 
	> Created Time: Fri 11 Sep 2015 05:09:02 PM PDT
 ************************************************************************/

#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <unistd.h>

#define MAXEPOLLSIZE 100000
#define MAXLINE 1024
int handle(int connfd);

int setnonblocking(int sockfd) {
	if (fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFD, 0)|O_NONBLOCK) == -1) {
		return -1;
	}
	return 0;
}

int main(int argc, char ** argv) {
	int serverPort = 6888;
	int listenBackLog = 1024;

	int listenfd, connfd, kdpfd, nfds, nread, n, curfds, acceptCnt = 0;
	struct sockaddr_in serverAddr, clientAddr;
	socklen_t clientLen = sizeof(struct sockaddr_in);
	struct epoll_event ev;
	struct epoll_event events[MAXEPOLLSIZE];
	struct rlimit rt;
	char buf[MAXLINE];

	rt.rlim_max = rt.rlim_cur = MAXEPOLLSIZE;
	if (setrlimit(RLIMIT_NOFILE, &rt) == -1) {
		perror("setrlimit error");
		return -1;
	}

	bzero(&serverAddr, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(serverPort);
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	if (listenfd < 0) {
		perror("socket error");
		return -1;
	}

	if (setnonblocking(listenfd) < 0) {
		perror("setnonblocking error");
	}

	if (bind(listenfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
		perror("bind error");
		return -1;
	}

	if (listen(listenfd, listenBackLog) == -1) {
		perror("listen error");
		return -1;
	}

	kdpfd = epoll_create(MAXEPOLLSIZE);
	ev.events = EPOLLIN|EPOLLET;
	ev.data.fd = listenfd;
	if (epoll_ctl(kdpfd, EPOLL_CTL_ADD, listenfd, &ev) < 0) {
		perror("epoll_Ctl add error");
		return -1;
	}
	curfds = 1;

	while (1) {
		nfds = epoll_wait(kdpfd, events, curfds, -1);
		if (nfds == -1) {
			perror("epoll wait");
			continue;
		}

		for (n = 0; n < nfds; ++n) {
			if (events[n].data.fd == listenfd) {
				connfd = accept(listenfd, (struct sockaddr*) &clientAddr, &clientLen);
				if (connfd < 0) {
					perror("accept error");
					continue;
				}
				sprintf(buf, "Accepet from %s : %d \n", inet_ntoa(clientAddr.sin_addr), clientAddr.sin_port);
				printf(buf, "");

				if (curfds >= MAXEPOLLSIZE) {
					printf("too many connection\n");
					close(connfd);
					continue;
				}

				if (setnonblocking(connfd) < 0) {
					perror("set nonblocking error");
				}

				ev.events = EPOLLIN|EPOLLET;
				ev.data.fd = connfd;
				if (epoll_ctl(kdpfd, EPOLL_CTL_ADD, connfd, &ev) < 0) {
					printf("epoll add error");
					return -1;
				}
				curfds++;
				continue;
			}

			if (handle(events[n].data.fd) < 0) {
				epoll_ctl(kdpfd, EPOLL_CTL_DEL, events[n].data.fd, &ev);
				curfds--;
			}
		}
	}
}


int handle(int connfd) {
	int nread;
	char buf[MAXLINE];

	nread = read(connfd, buf, MAXLINE);
	
	if (nread == 0) {
		printf("client close the connection");
		close(connfd);
		return -1;
	}
	if (nread < 0) {
		perror("read errror");
		close(connfd);
		return -1;
	}

	write(connfd, buf, nread);
	return 0;
}
