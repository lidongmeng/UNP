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
#include <sys/types.h>
#include <arpa/inet.h>
#define ERR_EXIT(m) \
	do {\
		perror(m); \
		exit(EXIT_FAILURE); \
	}while (0)


int main() {
    int sockfd;
	char * ip = "127.0.0.1";
	unsigned short port = 6234;

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) ERR_EXIT("socket");

	struct sockaddr_in serverAddr;
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);
	inet_pton(AF_INET, ip, &serverAddr.sin_addr);

	if (connect(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) ERR_EXIT("connect");

	if (send(sockfd, "123", 3, 0) == -1) ERR_EXIT("send");
	printf("send 3 byte of normal data(3)\n");
	sleep(2);

	if (send(sockfd, "4", 1, MSG_OOB) == -1) ERR_EXIT("send");
	printf("send 1 byte of OOB data(4)\n");
	sleep(2);
	
	if (send(sockfd, "56", 2, 0) == -1) ERR_EXIT("send");
	printf("send 2 byte of normal data(56)\n");
	sleep(2);
	
	if (send(sockfd, "ABC", 3, MSG_OOB) == -1) ERR_EXIT("send");
	printf("send 3 byte of OOB data(ABC)\n");

	if (send(sockfd, "abc", 3, MSG_OOB) == -1) ERR_EXIT("send");
	printf("send 3 byte of OOB data(abc)\n");
	sleep(2);


	close(sockfd);
	return 0;
}
