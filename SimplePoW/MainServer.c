#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <openssl/sha.h>
#include <arpa/inet.h>

#define MAX_WORKERS 10
#define CHALLENGE_SIZE 50
#define HASH_DIFFICULTY 8
#define PORT 8888

void distributeChallenge(char* challenge, int* workers, int numWorkers) {
    // Working Server들에게 challenge 값을 배포
    for (int i = 0; i < numWorkers; i++) {
        send(workers[i], challenge, CHALLENGE_SIZE, 0);
    }
}

void handleWorkerResults(int* workers, int numWorkers) {
    // Working Server들의 결과 확인
    for (int i = 0; i < numWorkers; i++) {
        char result;
        recv(workers[i], &result, sizeof(result), 0);
        printf("Received result from Worker %d: %c\n", i + 1, result);
    }
}

int performPoW(const char* challenge) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    int nonce = 0;
    //...

int main() {
    struct sockaddr_in serverAddr, workerAddr;
    socklen_t addrLen;
    int listenFd, connFd;
    int workers[MAX_WORKERS];
    int numWorkers = 0;

    listenFd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenFd < 0) {
        perror("Failed to create socket");
        exit(EXIT_FAILURE);
    }

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(listenFd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Failed to bind");
        exit(EXIT_FAILURE);
    }

    if (listen(listenFd, MAX_WORKERS) < 0) {
        perror("Failed to listen");
        exit(EXIT_FAILURE);
    }

    addrLen = sizeof(workerAddr);

    while (numWorkers < MAX_WORKERS) {
        connFd = accept(listenFd, (struct sockaddr *)&workerAddr, &addrLen);

        if (connFd >= 0) {
            printf("Worker %d connected\n", numWorkers + 1);
            workers[numWorkers] = connFd;
            numWorkers++;
        } else {
            perror("Failed to accept connection");
        }
    }

    // challenge 생성, 코드 동일
    //...

    // 연결 종료
    for (int i = 0; i < numWorkers; i++) {
        close(workers[i]);
    }
    close(listenFd);

    return 0;
}
