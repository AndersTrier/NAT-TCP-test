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


int do_connect(struct sockaddr_in *dst) {
    int s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s == -1) {
        perror("socket");
        return -1;
    }

    if (connect(s, (struct sockaddr *) dst, sizeof(struct sockaddr_in)) == -1) {
        perror("connect");
        return -1;
    }

/* for faster debugging
    // specifies the maximum amount of time in milliseconds
    // that transmitted data may remain unacknowledged before
    // TCP will forcibly close the corresponding connection
    // and return ETIMEDOUT to the application.
    if (setsockopt(s, IPPROTO_TCP, TCP_USER_TIMEOUT,
                (int []){ 10 * 1000 }, sizeof(int)) == -1) {
        perror("setsockopt TCP_USER_TIMEOUT");
        return -1;
    }
*/

    return s;
}

#define NCONNECTIONS 130
int main(int argc, char *argv[]){
    int tcpsessions[NCONNECTIONS] = {0};
    char buf;
    int ret;
    struct sockaddr_in dst_addr = {
        .sin_family = AF_INET,
        .sin_port   = htons(31415),
        .sin_addr   = inet_addr("130.225.254.111"),
    };

    printf("[+] Trying to establish connections: ");
    for (int i = 0; i < NCONNECTIONS; i++) {
        printf("%d ", i);
        fflush(stdout);
        tcpsessions[i] = do_connect(&dst_addr);
        if (tcpsessions[i] == -1) {
            return EXIT_FAILURE;
        }
    }

    printf("\n[+] All connections established\n");
    fflush(stdout);

    for (int i = 0; i < NCONNECTIONS; i++) {
        // This is not exact - we'll gradually drift
        // by the time it takes to write() and read()
        sleep(60);
        if (write(tcpsessions[i], "A", 1) != 1) {
            if (errno != ETIMEDOUT) {
                perror("write");
                return EXIT_FAILURE;
            }
            printf("[-] Connection %d is dead (write)\n", i);
            fflush(stdout);
            close(tcpsessions[i]);
            continue;
        }

        if (read(tcpsessions[i], &buf, 1) != 1) {
            if (errno != ETIMEDOUT) {
                perror("read");
                return EXIT_FAILURE;
            }
            printf("[-] Connection %d is dead (read)\n", i);
            fflush(stdout);
            close(tcpsessions[i]);
            continue;
        }

        printf("[+] Connection %d worked\n", i);
        fflush(stdout);

        close(tcpsessions[i]);
    }

    return 0;
}
