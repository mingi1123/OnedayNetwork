// tcp 기반 에코 서버

#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>

void errProc(const char *);

int main(int argc, char **argv)
{
    int mySock, readLen, nRecv, res;
    char buff[BUFSIZ];
    char *strAddr;
    struct sockaddr_in srcAddr, destAddr;
    socklen_t addrLen;

    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s Port", argv[0]);
        return 0;
    }

    mySock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); // tcp서버이기에 SOCK_STREAM로 설정해, tcp 소켓 생성
    if (mySock == -1)
        errProc("socket");

    memset(&srcAddr, 0, sizeof(srcAddr)); // tcp소켓 주소 세팅
    srcAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    srcAddr.sin_family = AF_INET;
    srcAddr.sin_port = htons(atoi(argv[1]));

    res = bind(mySock, (struct sockaddr *)&srcAddr, sizeof(srcAddr)); // bind함수
    if (res == -1)
        errProc("bind error");

    res = listen(mySock, 5); // listen함수, 연결요청 대기 큐를 만듦
    if (res == -1)
        errProc("listen error");

    addrLen = sizeof(destAddr);

    while (1)
    {
        int clientSock = accept(mySock, (struct sockaddr *)&destAddr, &addrLen); // accept함수로 클라이언트의 connect요청 수락
        if (clientSock == -1)
            errProc("accept error");

        strAddr = inet_ntoa(destAddr.sin_addr); // 연결된 클라이언트의 ip주소를 문자열 형태로 변환시켜, strAddr에 저장했다.

        while ((nRecv = recv(clientSock, buff, BUFSIZ - 1, 0)) > 0) // 클라이언트가 보낸 데이터를 읽어, 다시 send함수를 통해 클라이언트에게 보냄(echo)
        {
            buff[nRecv] = '\0';
            printf("received message : %s\n", buff);
            send(clientSock, buff, nRecv, 0);
        }

        if (nRecv == -1)
            errProc("recv error");

        close(clientSock); // 클라이언트 소켓 닫기
    }

    close(mySock); // 최종적으로 소켓 닫아 종료
    return 0;
}

void errProc(const char *str)
{
    fprintf(stderr, "%s: %s \n", str, strerror(errno));
    exit(1);
}
