#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>


int main(int arc, char *argv[]){
	printf("Hello ");

	struct sockaddr_in dst_addr = {
		.sin_family = AF_INET,
		.sin_port   = htons(2000),
		.sin_addr   = inet_addr("130.225.254.98"),
	};

	int s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (s == -1) {
		perror("socket");
		return EXIT_FAILURE;
	}

	if (connect(s, (struct sockaddr *) &dst_addr, sizeof dst_addr) == -1) {
		perror("connect");
		return EXIT_FAILURE;
	}

	if (setsockopt(s, SOL_SOCKET, SO_KEEPALIVE,
                      (int []){1}, sizeof(int)) == -1) {
		perror("setsockopt SO_KEEPALIVE");
		return EXIT_FAILURE;
	}

	if (setsockopt(s, IPPROTO_TCP, TCP_KEEPIDLE,
                      (int []){100}, sizeof(int)) == -1) {
		perror("setsockopt SO_KEEPALIVE");
		return EXIT_FAILURE;
	}

	// select ...
	

	printf("World!\n");
	return 0;
}

