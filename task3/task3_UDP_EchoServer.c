#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>

void printError(const char* str)
{
	fprintf(stderr,"%s: %s \n", str, strerror(errno));
	exit(1);
}

int main(int argc, char **argv)
{
    int srvSd, recvLen;
	char buff[BUFSIZ];
    char *strAddr;
    struct sockaddr_in srcAddr, destAddr;
    socklen_t addrLen;

    if (argc != 2) {
		fprintf(stderr,"Usage: %s Port",argv[0]);
		return 0;
	}

    //Create UDP Socket
    srvSd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(srvSd == -1) printError("Socket Error");	
	memset(&srcAddr, 0, sizeof(srcAddr));
	srcAddr.sin_addr.s_addr = htonl(INADDR_ANY);	
	srcAddr.sin_family = AF_INET; //IPv4
	srcAddr.sin_port = htons(atoi(argv[1]));

    // Server Socket Binding
    if (bind(srvSd,(struct sockaddr *) &srcAddr, sizeof(srcAddr)) == -1)
	    printError("bind Error");
    addrLen = sizeof(destAddr);

    while (1)
    {
        // Receive Client Message
        recvLen = recvfrom(srvSd, buff, BUFSIZ-1, 0, (struct sockaddr *) &destAddr, &addrLen);
        if(recvLen == -1) printError("recvfrom");
		if(recvLen > 0) buff[recvLen-1]='\0';
        else buff[recvLen] = '\0';

        // Print Message
        strAddr = inet_ntoa(destAddr.sin_addr);
        printf("%s : %d > %s \n", strAddr, ntohs(destAddr.sin_port), buff);
        recvLen = strlen(buff);

        // Send to Client
        sendto(srvSd, buff, recvLen, 0, (struct sockaddr *) &destAddr, addrLen);
    }

    close(srvSd);

	return 0;
}