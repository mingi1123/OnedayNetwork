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
#include <ctype.h>

int main() {

    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    struct addrinfo *bind_address;
    getaddrinfo(0, "8080", &hints, &bind_address); //set port

    // create listen socket
    int socket_listen;
    socket_listen = socket(bind_address->ai_family, bind_address->ai_socktype, bind_address->ai_protocol);
    if (socket_listen < 0) {
        fprintf(stderr, "socket() failed. (%d)\n", errno);
        return 1;
    }

    // Binding
    if (bind(socket_listen, bind_address->ai_addr, bind_address->ai_addrlen)) {
        fprintf(stderr, "bind() failed. (%d)\n", errno);
        return 1;
    }
    freeaddrinfo(bind_address);

    // Listening;
    if (listen(socket_listen, 10) < 0) {
        fprintf(stderr, "listen() failed. (%d)\n", errno);
        return 1;
    }

    fd_set master;
    FD_ZERO(&master);
    FD_SET(socket_listen, &master);
    int max_socket = socket_listen;

    printf("[ Server Started ]\n");
    printf("[ Waiting for connections...]\n");

    while(1) {
        fd_set reads;
        reads = master;
        if (select(max_socket+1, &reads, 0, 0, 0) < 0) {
            fprintf(stderr, "select() failed. (%d)\n", errno);
            return 1;
        }

        for(int i = 1; i <= max_socket; ++i) {
            if (FD_ISSET(i, &reads)) {
                if (i == socket_listen) { //accept client

                    // connect client
                    struct sockaddr_storage client_address;
                    socklen_t client_len = sizeof(client_address);
                    int socket_client = accept(socket_listen,
                            (struct sockaddr*) &client_address,
                            &client_len);
                    if (socket_client < 0) {
                        fprintf(stderr, "accept() failed. (%d)\n", errno);
                        return 1;
                    }

                    FD_SET(socket_client, &master);
                    if (socket_client > max_socket)
                        max_socket = socket_client;

                    char address_buffer[100];
                    getnameinfo((struct sockaddr*)&client_address,
                            client_len,
                            address_buffer, sizeof(address_buffer), 0, 0,
                            NI_NUMERICHOST);
                    // connection alarm
                    printf("\n[ Accept connection from client ]\n");
                    for (int j = 1; j <= max_socket; ++j) {
                        if (FD_ISSET(j, &master)) {
                            if (j == socket_client || j == i)
                                continue;
                            else {
                                // send connection alarm ohter clients
                                char connectmsg[50];
                                memset(connectmsg, 0, sizeof(connectmsg));
                                strcpy(connectmsg, "[ Accept connection from client ");
                                connectmsg[32] = '0' + socket_client - 3;
                                connectmsg[33] = ' '; connectmsg[34] = ']';
                                int n = send(j, connectmsg, strlen(connectmsg), 0);
                            }
                        }
                    }

                } else { // about client
                    char read[1024];
                    int bytes_received = recv(i, read, 1024, 0);
                    read[bytes_received] = '\0';

                    // disconnected client
                    if (bytes_received < 1) {
                        // disconnected alarm (\033[0;31 : 출력 색상 변경 코드입니다.)
                        printf("\033[0;31mLeave client %d \n\033[0m",i-3);
                        // send disconnected alarm ohter clients
                        for (int j = 1; j <= max_socket; ++j) {
                            if (FD_ISSET(j, &master)) {
                                if (j != socket_listen && j != i) {
                                    char leavemsg[30];
                                    memset(leavemsg, 0, 30);
                                    strcpy(leavemsg, "Leave client : ");
                                    leavemsg[14] = '0' + (i-3);
                                    send(j, leavemsg, strlen(leavemsg), 0);
                                }
                            }
                        }
                        FD_CLR(i, &master);
                        close(i);
                        continue;
                    }

                    // print message from client (\033[0;34 : 출력 색상 변경 코드입니다.)
                    printf("\033[0;34mFrom client %d : %s\n\033[0m",i-3,read);;

                    for (int j = 1; j <= max_socket; ++j) {
                        if (FD_ISSET(j, &master)) {
                            if (j == socket_listen || j == i)
                                continue;
                            else {
                                // send message from other clients
                                char frommsg[1050];
                                memset(frommsg, 0, sizeof(frommsg));
                                strcpy(frommsg, "From client ");
                                frommsg[11] = '0' + (i-3);
                                frommsg[12] = ' ';frommsg[13] = ':';frommsg[14] = ' ';
                                strcat(frommsg, read);
                                send(j, frommsg, strlen(frommsg), 0);
                            }
                        }
                    }
                }
                
            }
        }
    }

    printf("Closing listening socket...\n");
    close(socket_listen);

    printf("End Server.\n");

    return 0;
}
