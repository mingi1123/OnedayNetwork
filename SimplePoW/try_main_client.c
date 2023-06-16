#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <openssl/sha.h>

// 서버 주소
#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8080

// 해시 값의 비트 단위로 표현된 난이도
#define DIFFICULTY_BITS 7
#define SHA256_BLOCK_SIZE 64

// 해시 값 계산
void calculateHash(const char* input, uint32_t nonce, char* hash) {
    char data[256];
    sprintf(data, "%s%u", input, nonce);

    SHA256_CTX ctx;
    SHA256_Init(&ctx);
    SHA256_Update(&ctx, (uint8_t*)data, strlen(data));
    SHA256_Final((uint8_t*)hash, &ctx);
}

// 난이도 확인
int isValidHash(const char* hash, int difficulty) {
    int prefixLength = difficulty / 4; // 비트 단위 난이도를 바이트 단위로 변환

    for (int i = 0; i < prefixLength; i++) {
        if (hash[i] != 0) { // 비트가 모두 0인지 확인
            return 0; // 난이도를 만족하지 않음
        }
    }
    return 1; // 난이도를 만족함
}

int main() {
    int sockfd;
    struct sockaddr_in server_addr;
    char challenge[256];

    // 서버 소켓 생성
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Error creating socket");
        exit(1);
    }

    // 서버 주소 설정
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
        perror("Invalid address");
        exit(1);
    }

    // 서버에 연결
    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error connecting to server");
        exit(1);
    }

    printf("Connected to server.\n");

    // challenge 값 받기
    if (recv(sockfd, challenge, sizeof(challenge), 0) < 0) {
        perror("Error receiving challenge");
        exit(1);
    }

    printf("Received challenge: %s\n", challenge);

    // Proof of Work 수행
    uint32_t nonce = 0;
    char hash[SHA256_DIGEST_LENGTH];

    while (1) {
        calculateHash(challenge, nonce, hash);

        if (isValidHash(hash, DIFFICULTY_BITS)) {
            printf("Valid nonce found: %u\n", nonce);
            printf("Hash: %s\n", hash);
            break;
        }

        nonce++;
    }

    // 해시 값을 서버에 전송
    if (send(sockfd, hash, sizeof(hash), 0) < 0) {
        perror("Error sending hash to server");
        exit(1);
    }

    printf("Hash sent to server.\n");

    // 소켓 종료
    close(sockfd);

    return 0;
}
