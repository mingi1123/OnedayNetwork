#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/time.h>

void printError(const char* str)
{
	fprintf(stderr,"%s: %s \n", str, strerror(errno));
	exit(1);
}

int main(int argc, char** argv)
{
	int clntSd, nSent, nRecv, readLen;
	char buff[BUFSIZ];
    char *strAddr;
    struct sockaddr_in destAddr;
    socklen_t addrLen;

    if (argc != 3) {
		fprintf(stderr,"Usage: %s Port",argv[0]);
		return 0;
	}

    //Create UDP Socket
    clntSd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(clntSd == -1) printError("Socket Error");	
	memset(&destAddr, 0, sizeof(destAddr));
	destAddr.sin_addr.s_addr = inet_addr(argv[1]);
	destAddr.sin_family = AF_INET; //IPv4
	destAddr.sin_port = htons(atoi(argv[2]));
    addrLen = sizeof(destAddr);

    while (1)
    {
        // Input buff if Client
        // fgets(buff, BUFSIZ-1, stdin);
        char buff[BUFSIZ] = "test UDP loop";
        readLen = strlen(buff);

        struct timeval sendTime, recvTime;
        gettimeofday(&sendTime, NULL);
        // timestamp 값 출력

        // Send to Server
        nSent = sendto(clntSd, buff, readLen, 0, (struct sockaddr*) &destAddr, addrLen);
        if (nSent == -1)
            printError("Sendto Error");

        // Receive From Server
        nRecv = recvfrom(clntSd, buff, BUFSIZ-1, 0, (struct sockaddr*) &destAddr, &addrLen);
        if (nRecv == -1)
            printError("Receive Error");
        buff[nRecv] = '\0';

        gettimeofday(&recvTime, NULL); // timestamp 코드 추가
        long sendSeconds = sendTime.tv_sec;
        long sendMicroseconds = sendTime.tv_usec;
        long recvSeconds = recvTime.tv_sec;
        long recvMicroseconds = recvTime.tv_usec;

        long sendTimeMicro = sendSeconds * 1000000 + sendMicroseconds; // timestamp 시간 차이 잘 나보이도록 추가한 코드
        long recvTimeMicro = recvSeconds * 1000000 + recvMicroseconds;
        long timeDiffMicro = recvTimeMicro - sendTimeMicro;

        // print receive
        printf("Server : %s\n", buff);
  
        printf("Sent Time: %ld.%06ld\n", sendSeconds, sendMicroseconds);
        printf("Received Time: %ld.%06ld\n", recvSeconds, recvMicroseconds);
        printf("Time Difference: %ld microseconds\n", timeDiffMicro);
        
        // END
        if(!strncmp(buff,"END",3)) break;
    }
    close(clntSd);
    
    return 0;
}