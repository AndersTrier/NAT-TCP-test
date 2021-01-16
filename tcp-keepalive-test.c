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

#include "util.h"

#ifdef __APPLE__
#define TCP_KEEPIDLE TCP_KEEPALIVE
#endif

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

    // Enable TCP keepalive
    if (setsockopt(s, SOL_SOCKET, SO_KEEPALIVE,
                (int []){1}, sizeof(int)) == -1) {
        perror("setsockopt SO_KEEPALIVE");
        return -1;
    }

    // The time (in seconds) the connection needs to remain idle before
    // TCP starts sending keepalive probes, if the socket option
    // SO_KEEPALIVE has been set on this socket.
    if (setsockopt(s, IPPROTO_TCP, TCP_KEEPIDLE,
                &keepalive_time, sizeof keepalive_time) == -1) {
        perror("setsockopt SO_KEEPIDLE");
        return -1;
    }

/* For faster debugging
    // The maximum number of keepalive probes TCP  should  send  before
    // dropping the connection.
    if (setsockopt(s, IPPROTO_TCP, TCP_KEEPCNT,
                (int []){3}, sizeof(int)) == -1) {
        perror("setsockopt TCP_KEEPCNT");
        return -1;
    }

    // The time (in seconds) between individual keepalive probes.
    if (setsockopt(s, IPPROTO_TCP, TCP_KEEPINTVL,
                (int []){5}, sizeof(int)) == -1) {
        perror("setsockopt TCP_KEEPINTVL");
        return -1;
    }
*/

    return s;
}

#define NCONNECTIONS 130
int main(int argc, char *argv[]){
    int tcpsessions[NCONNECTIONS] = {0};
    int keepalivetime[NCONNECTIONS] = {0};
    char buf;
    int ret;
    struct sockaddr_in dst_addr = {
        .sin_family = AF_INET,
        .sin_port   = htons(31415),
        .sin_addr   = inet_addr("130.225.254.111"),
    };

    printf("[+] Trying to establish connections: ");
    for (int i = 0; i < NCONNECTIONS; i++) {
        // The first connection will have a keepalivetime of 1 min,
        // the second one 2 min, etc.
        keepalivetime[i] = (i + 1) * 60;
        printf("%d (%ds), ", i, keepalivetime[i]);
        fflush(stdout);
        tcpsessions[i] = do_connect(&dst_addr, keepalivetime[i]);
        if (tcpsessions[i] == -1) {
            return EXIT_FAILURE;
        }
    }
    printf("\n");
    msg("[+] All connections established\n");

    // select() loop
    for (;;) {
        fd_set rfds;
        FD_ZERO(&rfds);
        int openconnections = 0;
        int maxfd = -1;

        for (int i = 0; i < NCONNECTIONS; i++) {
            if (tcpsessions[i] == -1)
                continue;
            FD_SET(tcpsessions[i], &rfds);
            maxfd = tcpsessions[i] > maxfd ? tcpsessions[i] : maxfd;
            openconnections++;
        }
        if (openconnections == 0) {
            msg("[+] No more alive connections left\n");
            return EXIT_SUCCESS;
        }

        msg("[+] Waiting for a connection to timeout.\n");
        msg("[+] Open connections: %d\n", openconnections);

        ret = select(maxfd + 1, &rfds, NULL, NULL, NULL);
        if (ret == -1) {
            if (errno == EINTR)
                continue;
            perror("select");
            return EXIT_FAILURE;
        } else if (ret) {
            for (int i = 0; i < NCONNECTIONS; i++) {
                if (tcpsessions[i] != -1 && FD_ISSET(tcpsessions[i], &rfds)) {
                    ssize_t readret = read(tcpsessions[i], &buf, 1);
                    if (readret != -1 || errno != ETIMEDOUT) {
                        msg("[-] shouldn't happen:\n"
                                "readret: %ld\n"
                                "errno: %d (%m)\n"
                                "i: %d\n"
                                "tcpsessions[i]: %d\n",
                                readret, errno, i, tcpsessions[i]
                              );
                        close(tcpsessions[i]);
                        tcpsessions[i] = -1;
                        continue;
                    }
                    msg("[+] TCP connection died: ");
                    printf("Connection %d, keepalivetime: %dm %ds\n",
                            i, keepalivetime[i]/60, keepalivetime[i]%60);
                    fflush(stdout);
                    close(tcpsessions[i]);
                    tcpsessions[i] = -1;
                }
            }
        } else {
            msg("[+] No data\n");
        }
    }

    return 0;
}
