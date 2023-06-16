#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stddef.h>
#include <time.h>
#include <openssl/sha.h>
// 서버 주소
#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8080

// // 해시 값의 비트 단위로 표현된 난이도
// #define DIFFICULTY_BITS 4
#define SHA256_BLOCK_SIZE 64

// // 난이도에 해당하는 해시 값의 접두사
// const char* TARGET_PREFIX = "0000";

// 블록 구조체
typedef struct {
    uint32_t index;
    uint64_t timestamp;
    char data[256];
    char previousHash[SHA256_BLOCK_SIZE * 2 + 1];
    char hash[SHA256_BLOCK_SIZE * 2 + 1];
    uint32_t nonce;
} Block;

// 블록의 해시 값 계산
void calculateHash(Block* block, char* hash) {
    SHA256_CTX ctx;
    SHA256_Init(&ctx);
    SHA256_Update(&ctx, (uint8_t*)block, sizeof(Block));
    SHA256_Final(&ctx, (uint8_t*)hash);
}

// // 난이도에 해당하는 해시 값의 접두사가 일치하는지 확인
// int isValidHash(char* hash) {
//     return strncmp(hash, TARGET_PREFIX, DIFFICULTY_BITS) == 0;
// }

int main() {
    int sockfd;
    struct sockaddr_in server_addr;
    Block block;

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

    // // 블록 정보 설정
    // block.index = 0;
    // block.timestamp = time(NULL);
    // strcpy(block.data, "20211693 변은영");
    // strcpy(block.previousHash, "0000000000000000000000000000000000000000000000000000000000000000");
    // block.nonce = 0;

    // 작업 완료된 블록 수신
    if (recv(sockfd, (void*)&block, sizeof(Block), 0) < 0) {
        perror("Error receiving block");
        exit(1);
    }
    printf("Data: %s\n", block.data);
    printf("Block Hash: %s\n", block.hash);
    printf("Nonce: %u\n", block.nonce);

    // 소켓 종료
    close(sockfd);

    return 0;
}
