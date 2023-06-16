#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <openssl/sha.h>

#define CHALLENGE_SIZE 50
#define HASH_DIFFICULTY 8
#define PORT 8888

void performPoW(char* challenge, int mainServerSocket) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    int nonce = 0;

    while (1) {
        // challenge와 nonce를 합쳐서 입력 데이터 생성
        char input[CHALLENGE_SIZE + sizeof(int)];
        snprintf(input, CHALLENGE_SIZE + sizeof(int), "%s%d", challenge, nonce);

        // SHA256 해시 계산
        SHA256((unsigned char*)input, strlen(input), hash);

        // 해시 값의 난이도 확인
        int difficultyCount = 0;
        for (int i = 0; i < HASH_DIFFICULTY / 2; i++) {
            if (hash[i] == 0) {
                difficultyCount++;
            }
        }

        // 난이도가 충족되면 작업 완료
        if (difficultyCount >= HASH_DIFFICULTY / 2) {
            break;
        }

        nonce++;
    }

    // 결과 전송
    send(mainServerSocket, &nonce, sizeof(nonce), 0);
}

int main() {
    // Main Server와의 연결 설정
    int mainServerSocket = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT); // Main Server의 포트 번호
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    memset(serverAddress.sin_zero, '\0', sizeof(serverAddress.sin_zero));

    connect(mainServerSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));

    // challenge 값 수신
    char challenge[CHALLENGE_SIZE];
    recv(mainServerSocket, challenge, CHALLENGE_SIZE, 0);

    // 난이도 확인
    int difficulty = strlen(challenge) - 2;
    if (difficulty < HASH_DIFFICULTY) {
        // 난이도가 충족되지 않으면 종료
        close(mainServerSocket);
        return 0;
    }

    // Proof of Work 수행
    performPoW(challenge, mainServerSocket);

    // Main Server와의 연결 종료
    close(mainServerSocket);

    return 0;
}
