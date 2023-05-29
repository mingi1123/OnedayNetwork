#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <wait.h>

void errProc(const char *str);
void errPrint(const char *str);
void sigchildHandler(int sig); // New Handler

int main(int argc, char** argv)
{
    int srvSd, clntSd;
    struct sockaddr_in srvAddr, clntAddr;
    int clntAddrLen, readLen, strLen;
    char rBuff[BUFSIZ];
    pid_t pid;

    if(argc != 2) {
        printf("Usage: %s [port] \n", argv[0]);
        exit(1);
    }
    printf("Server start...\n");

    // Create a socket
    srvSd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(srvSd == -1 ) errProc("socket");

    // Configure the server address
    memset(&srvAddr, 0, sizeof(srvAddr));
    srvAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    srvAddr.sin_family = AF_INET;
    srvAddr.sin_port = htons(atoi(argv[1]));

    // Bind the socket to the server address
    if(bind(srvSd, (struct sockaddr *) &srvAddr, sizeof(srvAddr)) == -1)
        errProc("bind");

    // Listen for incoming connections
    if(listen(srvSd, 5) < 0)
        errProc("listen");

    clntAddrLen = sizeof(clntAddr);

    // Set up a signal handler for child processes
    signal(SIGCHLD, sigchildHandler);

    while(1)
    {
        // Accept a client connection
        clntSd = accept(srvSd, (struct sockaddr *) &clntAddr, &clntAddrLen);
        if(clntSd == -1) {
            errPrint("accept");
            continue;	
        }
        printf("client %s:%d is connected...\n",
            inet_ntoa(clntAddr.sin_addr),
            ntohs(clntAddr.sin_port));

        // Fork a child process to handle the client request
        pid = fork();
        if(pid == 0) { /* child process */
            close(srvSd);
            while(1) {
                // Read data from the client
                readLen = read(clntSd, rBuff, sizeof(rBuff)-1);
                if(readLen == 0) break;
                rBuff[readLen] = '\0';
                printf("Client(%d): %s\n",
                    ntohs(clntAddr.sin_port), rBuff);

                // Echo the received data back to the client
                write(clntSd, rBuff, strlen(rBuff));
            }
            printf("Client(%d): is disconnected\n",
                ntohs(clntAddr.sin_port));

            // Close the client socket and exit the child process
            close(clntSd);
            return 0;
        }
        else if(pid == -1) errProc("fork");
        else { /* Parent Process */
            close(clntSd);
        }
    }

    // Close the server socket
    close(srvSd);
    return 0;
}

// Function to handle and print error messages
void errProc(const char *str)
{
    fprintf(stderr,"%s: %s \n",str, strerror(errno));
    exit(1);
}

// Function to print error messages
void errPrint(const char *str)
{
    fprintf(stderr, "%s: %s \n", str, strerror(errno));
}

// Signal handler for child processes
void sigchildHandler(int sig)
{
    pid_t pid;
    int status;

    // Wait for child process to terminate
    pid = wait(&status);

    if (pid == -1)
        printf("Error : %d\n", errno);
    else if (WIFEXITED(status))
        printf("(Normal exit) child process pid : %d, child process return : %d\n", pid, WEXITSTATUS(status));
    else if (WIFSIGNALED(status))
        printf("(Abnormal exit) child process pid : %d, child process return : %d\n", pid, WTERMSIG(status));
}
