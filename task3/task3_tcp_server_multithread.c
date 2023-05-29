#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/wait.h>
#include <pthread.h>
#include <unistd.h>

// Thread Function
void *client_module(void *data)
{
    char rBuff[BUFSIZ];
    int readLen;
    int connectSd;
    connectSd = *((int *)data);

    while (1)
    {
		// Read data from the client
        readLen = read(connectSd, rBuff, sizeof(rBuff) - 1);
        if (readLen <= 0)
            break;
        rBuff[readLen] = '\0';
        printf("Client(%d): %s\n", connectSd, rBuff);

		// received data back to the client
        write(connectSd, rBuff, strlen(rBuff));
    }
    fprintf(stderr, "The client is disconnected.\n");
    close(connectSd);
}

int main(int argc, char **argv)
{
    int listenSd, connectSd;
    struct sockaddr_in srvAddr, clntAddr;
    int strLen;

    struct sigaction act;
    pthread_t thread;

    if (argc != 2)
    {
        printf("Usage: %s [Port Number]\n", argv[0]);
        return -1;
    }

    printf("Server start...\n");
	// Create a socket
    listenSd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	// Configure the server address
    memset(&srvAddr, 0, sizeof(srvAddr));
    srvAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    srvAddr.sin_family = AF_INET;
    srvAddr.sin_port = htons(atoi(argv[1]));

	// Bind the socket to the server address
    bind(listenSd, (struct sockaddr *)&srvAddr, sizeof(srvAddr));
    listen(listenSd, 5);

    socklen_t clntAddrLen = sizeof(clntAddr);
    while (1)
    {
		// Accept a client connection
        connectSd = accept(listenSd,
                           (struct sockaddr *)&clntAddr, &clntAddrLen);
        if (connectSd == -1) // Connection failed
        {
            continue;
        }
        else
        {
            printf("A client is connected...\n");
        }

        // Create a thread to handle the client request
        pthread_create(&thread, NULL, client_module, (void *)&connectSd);
        pthread_detach(thread);
    }
    close(listenSd);
    return 0;
}
