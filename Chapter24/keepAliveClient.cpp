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
#include <arpa/inet.h>
#include <signal.h>

#define ERR_EXIT(m) \
	do {\
		perror(m); \
		exit(EXIT_FAILURE); \
	}while (0)

int maxnprobes;
int nprobes;
int secofgap;
int server_fd;

void urg_handler(int signo);
void alrm_handler(int signo);
void heartbeat_client();
void tcp_client();


int main(int argc, char ** argv) {
	if (argc != 3) {
		printf("Usage: client maxnprobes(max numer of probes of unreachable) secofgap(gao of two sequece heart beat packet)\n");
		return 0;
	}
	maxnprobes = atoi(argv[1]);
	secofgap = atoi(argv[2]);

	tcp_client();
	heartbeat_client();
	while (1) {}
	return 0;
}

void tcp_client() {
	char * ip = "127.0.0.1";
	unsigned short port = 6234;

	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) ERR_EXIT("socket");

	struct sockaddr_in serverAddr;
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);
	inet_pton(AF_INET, ip, &serverAddr.sin_addr);

	if (connect(server_fd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) ERR_EXIT("connect");
}

void heartbeat_client() {
	nprobes = 0;
	signal(SIGURG, urg_handler);
	signal(SIGALRM, alrm_handler);	
    fcntl(server_fd, F_SETOWN, getpid());
	alarm(secofgap);
}

void urg_handler(int signo) {
	int n;
	char c;
	if ( (n = recv(server_fd, &c, 1, MSG_OOB)) < 0) {
		if (errno != EWOULDBLOCK) ERR_EXIT("recv error");
	}
	printf("receive heart beat packet from server\n");
	nprobes = 0;
	return ;
}

void alrm_handler(int singo) {
	if (++nprobes > maxnprobes) {
		printf("server is unreachable\n");
		exit(0);
	}

	send(server_fd, "1", 1, MSG_OOB);
	alarm(secofgap);
	printf("send heart beat packet to server\n");
	return ;
}
