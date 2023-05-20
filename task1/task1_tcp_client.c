// tcp 기반 에코 클라이언트

#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/time.h>

void errProc(const char *);
int main(int argc, char **argv)
{
    int mySock, readLen, nSent, nRecv;
    char buff[BUFSIZ];
    struct sockaddr_in destAddr;

    mySock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); // TCP기반 클라이언트니까 SOCK_STREAM으로 변경해 tcp 소켓 생성

    memset(&destAddr, 0, sizeof(destAddr)); // 소켓 주소 세팅
    destAddr.sin_addr.s_addr = inet_addr(argv[1]);
    destAddr.sin_family = AF_INET;
    destAddr.sin_port = htons(atoi(argv[2]));

    if (connect(mySock, (struct sockaddr *)&destAddr, sizeof(destAddr)) == -1)
    {
        errProc("connect error");
    } // 서버와 connect 연결 시도

    while (1) // while문 무한루프
    {
        fgets(buff, BUFSIZ - 1, stdin);
        readLen = strlen(buff);               // 문자열을 읽어들여, buff에 저장

        struct timeval sendTime, recvTime;
        gettimeofday(&sendTime, NULL);
        // timestamp 값 출력

        nSent = write(mySock, buff, readLen); // 읽어들인 문자열을 readLen만큼 mySock으로 보낸다.
        if (nSent == -1)
            errProc("write error");
        nRecv = read(mySock, buff, BUFSIZ - 1); // mySock소켓으로부터 데이터를 읽어와 buff에 저장
        if (nRecv == -1)
            errProc("read error");
        buff[nRecv] = '\0';

        gettimeofday(&recvTime, NULL); // timestamp 코드 추가
        long sendSeconds = sendTime.tv_sec;
        long sendMicroseconds = sendTime.tv_usec;
        long recvSeconds = recvTime.tv_sec;
        long recvMicroseconds = recvTime.tv_usec;

        long sendTimeMicro = sendSeconds * 1000000 + sendMicroseconds; // timestamp 시간 차이 잘 나보이도록 추가한 코드
        long recvTimeMicro = recvSeconds * 1000000 + recvMicroseconds;
        long timeDiffMicro = recvTimeMicro - sendTimeMicro;

        printf("Server: %s\n", buff);
  
        printf("Sent Time: %ld.%06ld\n", sendSeconds, sendMicroseconds);
        printf("Received Time: %ld.%06ld\n", recvSeconds, recvMicroseconds);
        printf("Time Difference: %ld microseconds\n", timeDiffMicro);
       
        if (!strcmp(buff, "END")) // 받은 데이터가 "END"인 경우 반복문을 빠져나간다.
            break;
    }
    close(mySock); // 소켓 닫기
    return 0;
}

void errProc(const char *str)
{
    fprintf(stderr, "%s: %s \n", str, strerror(errno));
    exit(1);
}
