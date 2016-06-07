/*************************************************************************
	> File Name: oobServer.cpp
	> Author: lidongmeng
	> Mail: lidongmeng@ict.ac.cn
	> Created Time: Tue 31 May 2016 06:33:39 PM PDT
 ************************************************************************/

#include <sys/socket.h>
#include <fcntl.h>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <errno.h>
#include <unistd.h>
#include <netinet/in.h>
#include <signal.h>

#define ERR_EXIT(m) \
	do {\
		perror(m); \
		exit(EXIT_FAILURE); \
	}while (0)

void urg_handler(int signo);
int tcp_server(unsigned short port);
int connectFd;

int main() {
	int sockfd = tcp_server(6234);
	signal(SIGURG, urg_handler);
	
	struct sockaddr_in clientAddr;
	socklen_t clientLen = sizeof(socklen_t);
	char buff[1024];
	int n;

	while (1) {
		if ((connectFd = accept(sockfd, (struct sockaddr*)&clientAddr, &clientLen)) < 0) ERR_EXIT("accept");
		 fcntl(connectFd, F_SETOWN, getpid());
		while (1) {
			n = recv(connectFd, buff, sizeof(buff)-1, 0);
			if (n == -1) {
				ERR_EXIT("recv");
			} else if (n == 0) {
				break;
			} else {
				buff[n] = '\0';
				printf("recv from client of normal data:%s\n", buff);
			}
		}
		close(connectFd);
	}
	return 0;
}

int tcp_server(unsigned short port) {
	// establish a socket
	int sockfd;
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) ERR_EXIT("socket");

	struct sockaddr_in serverAddr;
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);
	serverAddr.sin_addr.s_addr = INADDR_ANY;

	if (bind(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) ERR_EXIT("bind");

	if (listen(sockfd, 1024) < 0) ERR_EXIT("listen");

	return sockfd;
}

void urg_handler(int signo) {
	int n;
	char buff[100];
	printf("SIGURG receive.\n");

	n = recv(connectFd, buff, sizeof(buff)-1, MSG_OOB);
	if (n > 0) {
		buff[n] = '\0';
		printf("recv %d OOB byte: %s\n", n, buff);
	} else {
		ERR_EXIT("MSG_OOB");
	}
}
