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

int maxnprobes;
int nprobes;
int secofgap;
void heartbeat_server();
void alrm_handler(int signo);
void urg_handler(int signo);
int tcp_server(unsigned short port);
int connfd;

int main(int argc, char ** argv) {
	if (argc != 3) {
		printf("Usage: client maxnprobes(max numer of probes of unreachable) secofgap(gao of two sequece heart beat packet)\n");
		return 0;
	}
	maxnprobes = atoi(argv[1]);
	secofgap = atoi(argv[2]);

	int sockfd = tcp_server(6234);
	struct sockaddr_in clientAddr;
	socklen_t clientLen = sizeof(socklen_t);
	if ((connfd = accept(sockfd, (struct sockaddr*)&clientAddr, &clientLen)) < 0) ERR_EXIT("accept");
	heartbeat_server();
	while (1) {}
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

void heartbeat_server() {
	nprobes = 0;
	signal(SIGURG, urg_handler);
	signal(SIGALRM, alrm_handler);	
    fcntl(connfd, F_SETOWN, getpid());
	alarm(secofgap);
}

void urg_handler(int signo) {
	int n;
	char c;
	if ( (n = recv(connfd, &c, 1, MSG_OOB)) < 0) {
		if (errno != EWOULDBLOCK) ERR_EXIT("recv error");
	}
	printf("receive hear beat packet from client\n");
	nprobes = 0;
	return ;
}

void alrm_handler(int singo) {
	if (++nprobes > maxnprobes) {
		printf("no probes from client\n");
		exit(0);
	}

	send(connfd, "1", 1, MSG_OOB);
//	alarm(secofgap);
	printf("send heart beat packet to client\n"); 
	return ;
}
