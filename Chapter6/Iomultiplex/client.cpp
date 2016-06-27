/*************************************************************************
	> File Name: client01.c
	> Author: lidongmeng
	> Mail:lidongmeng0213@163.com 
	> Created Time: Mon 31 Aug 2015 11:22:44 PM PDT
 ************************************************************************/
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#define MAXLINE 1024

int main(int argc, char * argv[]) {
    char buf[MAXLINE];
	int sockfd;
	char * serverIP = "127.0.0.1";
	int serverPort = 6888; 
	struct sockaddr_in serverAddr;
	int status;
	int n;
	char sendLine[MAXLINE], recvLine[MAXLINE];

	if (argc == 2) {
         serverIP = argv[1];			
	}
	
	if (argc == 3) {
		serverIP = argv[1];
		serverPort = atoi(argv[2]);
	}

	if (argc > 3) {
		printf("usage: client01 <IPAddr> <Port>\n");
		return -1;
	}

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("Error in socket build\n");
		return sockfd;
	}
	
	bzero(&serverAddr, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(serverPort);
	inet_pton(AF_INET, serverIP, &serverAddr.sin_addr);

    if (status = connect(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
		printf("connect error\n");
		return status;
	}
    printf("client connnect to the server\n");	
	while (1) {
		printf("input message>>");
		if (fgets(sendLine, MAXLINE, stdin) == NULL) {
			break;
		}

		n = write(sockfd, sendLine, strlen(sendLine));
		n = read(sockfd, recvLine, MAXLINE);

		if (n == 0) {
			printf("server terminate\n");
			break;
		}
		printf("receive from server: [%s]\n", recvLine);
	}

	close(sockfd);
	printf("End\n");
	return 0;
}
