#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>


int do_connect(struct sockaddr_in *dst, int keepalive_time) {
	int s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (s == -1) {
		perror("socket");
		return -1;
	}

	if (connect(s, (struct sockaddr *) dst, sizeof(struct sockaddr_in)) == -1) {
		perror("connect");
		return -1;
	}

	if (setsockopt(s, SOL_SOCKET, SO_KEEPALIVE,
                      (int []){1}, sizeof(int)) == -1) {
		perror("setsockopt SO_KEEPALIVE");
		return -1;
	}

	if (setsockopt(s, IPPROTO_TCP, TCP_KEEPIDLE,
                &keepalive_time, sizeof keepalive_time) == -1) {
		perror("setsockopt SO_KEEPALIVE");
		return -1;
	}


        // The maximum number of keepalive probes TCP  should  send  before
        // dropping the connection.
	if (setsockopt(s, IPPROTO_TCP, TCP_KEEPCNT,
                    (int []){2}, sizeof(int)) == -1) {
		perror("setsockopt TCP_KEEPCNT");
		return -1;
	}

        // The time (in seconds) between individual keepalive probes.
	if (setsockopt(s, IPPROTO_TCP, TCP_KEEPINTVL,
                    (int []){10}, sizeof(int)) == -1) {
		perror("setsockopt TCP_KEEPINTVL");
		return -1;
	}

        return s;
}

#define NCONNECTIONS 4
int main(int argc, char *argv[]){
        int tcpsessions[NCONNECTIONS] = {0};
        char buf;
        int ret;
	struct sockaddr_in dst_addr = {
		.sin_family = AF_INET,
		.sin_port   = htons(2000),
		.sin_addr   = inet_addr("130.225.254.98"),
	};

        struct timeval tv = {0};


        for (int i = 0; i < NCONNECTIONS; i++) {
            tcpsessions[i] = do_connect(&dst_addr, (i+1) * 10);
            if (tcpsessions[i] == -1) {
                return EXIT_FAILURE;
            }
        }



        for (;;) {
            fd_set rfds;
            FD_ZERO(&rfds);
            tv.tv_sec = 30;
            tv.tv_usec = 0;
            int maxfd = -1;

            for (int i = 0; i < NCONNECTIONS; i++) {
                if (tcpsessions[i] == -1)
                    continue;
                FD_SET(tcpsessions[i], &rfds);
                maxfd = tcpsessions[i] > maxfd ? tcpsessions[i] : maxfd;
            }
            if (maxfd == -1) {
                printf("No more alive connections left\n");
                return EXIT_SUCCESS;
            }

            ret = select(maxfd + 1, &rfds, NULL,
                      NULL, /*&tv*/ NULL);
            if (ret == -1) {
                if (errno == EINTR)
                    continue;
                perror("select");
            } else if (ret) {
                printf("Data\n");
                for (int i = 0; i < NCONNECTIONS; i++) {
                    if (FD_ISSET(tcpsessions[i], &rfds)) {
                        ssize_t readret = read(tcpsessions[i], &buf, 1);
                        if (readret != -1 || errno != ETIMEDOUT){
                            printf("shouldn't happen:\n"
                                    "readret: %ld\n"
                                    "errno: %d\n"
                                    "i: %d\n"
                                    "tcpsessions[i]: %d\n",
                                    readret, errno, i, tcpsessions[i]
                                    );
                        }
                        printf("yay!\n");
                        printf("read returned %ld for i = %d, s = %d\n",
                                readret, i, tcpsessions[i]);
                        tcpsessions[i] = -1;
                    }
                }
            } else {
                printf("No data\n");
            }
        }

	return 0;
}

