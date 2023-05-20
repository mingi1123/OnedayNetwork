/* 20211670 권민기 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "usage: tcp_client hostname port\n");
        return 1;
    }

    // set addrinfo
    printf("Configuring remote address...\n");
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    struct addrinfo *peer_address;
    if (getaddrinfo(argv[1], argv[2], &hints, &peer_address)) {
        fprintf(stderr, "getaddrinfo() failed. (%d)\n", errno);
        return 1;
    }

    // set remote address
    printf("Remote address is: ");
    char address_buffer[100];
    char service_buffer[100];
    getnameinfo(peer_address->ai_addr, peer_address->ai_addrlen,
                address_buffer, sizeof(address_buffer),
                service_buffer, sizeof(service_buffer),
                NI_NUMERICHOST);
    printf("%s %s\n", address_buffer, service_buffer);

    // create socket
    printf("Creating socket...\n");
    int socket_peer;
    socket_peer = socket(peer_address->ai_family,
                         peer_address->ai_socktype, peer_address->ai_protocol);
    if (socket_peer < 0) {
        fprintf(stderr, "socket() failed. (%d)\n", errno);
        return 1;
    }

    // connect server
    printf("Connecting...\n");
    if (connect(socket_peer,
                peer_address->ai_addr, peer_address->ai_addrlen)) {
        fprintf(stderr, "connect() failed. (%d)\n", errno);
        return 1;
    }
    freeaddrinfo(peer_address);

    printf("[ Connected with Server ]\n");

    while (1) {
        fd_set reads;
        FD_ZERO(&reads);
        FD_SET(socket_peer, &reads);
        FD_SET(0, &reads);

        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 100000;

        if (select(socket_peer + 1, &reads, 0, 0, &timeout) < 0) {
            fprintf(stderr, "select() failed. (%d)\n", errno);
            return 1;
        }

        if (FD_ISSET(socket_peer, &reads)) { // connection closed
            char read[4096];
            int bytes_received = recv(socket_peer, read, 4096, 0);
            if (bytes_received < 1) {
                printf("Connection closed by peer.\n");
                break;
            }
            read[bytes_received] = '\0';
            printf("%s\n", read);
        }

        if (FD_ISSET(0, &reads)) { // input client message
            char read[4096];
            if (!fgets(read, 4096, stdin)) break; // break while 
            printf("Sending: %s", read);

            struct timeval sendTime, recvTime;
            gettimeofday(&sendTime, NULL);
            // timestamp 값 출력
            
            int bytes_sent = send(socket_peer, read, strlen(read), 0);

            gettimeofday(&recvTime, NULL); // timestamp 코드 추가
            long sendSeconds = sendTime.tv_sec;
            long sendMicroseconds = sendTime.tv_usec;
            long recvSeconds = recvTime.tv_sec;
            long recvMicroseconds = recvTime.tv_usec;

            long sendTimeMicro = sendSeconds * 1000000 + sendMicroseconds; // timestamp 시간 차이 잘 나보이도록 추가한 코드
            long recvTimeMicro = recvSeconds * 1000000 + recvMicroseconds;
            long timeDiffMicro = recvTimeMicro - sendTimeMicro;

             printf("Sent Time: %ld.%06ld\n", sendSeconds, sendMicroseconds);
            printf("Received Time: %ld.%06ld\n", recvSeconds, recvMicroseconds);
            printf("Time Difference: %ld microseconds\n", timeDiffMicro);

        }
    }  // end while

    printf("Closing socket...\n");
    close(socket_peer);

    printf("End client.\n");
    return 0;
}

