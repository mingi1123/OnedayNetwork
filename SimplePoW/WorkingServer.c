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

// 해시 값의 비트 단위로 표현된 난이도
#define DIFFICULTY_BITS 7
#define SHA256_BLOCK_SIZE 32

// 난이도에 해당하는 해시 값의 접두사
const char* TARGET_PREFIX = "0000000";

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
    char data[512]; 
    int dataSize = snprintf(data, sizeof(data), "%s%u", input, nonce);
    if (dataSize < 0 || dataSize >= sizeof(data)) {
        // 버퍼 크기 초과 오류 처리
        fprintf(stderr, "Error: Buffer overflow.\n");
        exit(1);
    }

    SHA256_CTX ctx;
    SHA256_Init(&ctx);
    SHA256_Update(&ctx, (uint8_t*)data, dataSize);
    SHA256_Final((uint8_t*)hash, &ctx);
}

// 작업 증명을 수행하는 함수
void performProofOfWork(Block* block) {
    char hash[SHA256_BLOCK_SIZE * 2 + 1];
    char target[SHA256_BLOCK_SIZE * 2 + 1];
    memset(target, '0', DIFFICULTY_BITS);
    target[DIFFICULTY_BITS] = '\0';

    while (1) {
        calculateHash(block, hash);

        if (strncmp(hash, target, DIFFICULTY_BITS) == 0) {
            strncpy(block->hash, hash, SHA256_BLOCK_SIZE * 2 + 1);
            return;
        }

        block->nonce++;

        // 원하는 난이도에 해당하는 해시 값을 찾지 못하고 너무 많은 시도를 한 경우 작업 증명 종료
        if (block->nonce == UINT32_MAX) {
            printf("Proof of work failed. Unable to find a matching hash.\n");
            exit(1);
        }
    }
}

// 블록 정보 출력
void printBlockInfo(Block* block) {
    printf("Block Index: %u\n", block->index);
    printf("Timestamp: %llu\n", block->timestamp);
    printf("Data: %s\n", block->data);
    printf("Previous Hash: %s\n", block->previousHash);
    printf("Hash: %s\n", block->hash);
    printf("Nonce: %u\n", block->nonce);
}

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

    // 작업 완료된 블록 수신
    if (recv(sockfd, (void*)&block, sizeof(Block), 0) < 0) {
        perror("Error receiving block");
        exit(1);
    }

    // 작업 증명 수행
    printf("Start performProofOfWork\n");
    performProofOfWork(&block);
    printf("End performProofOfWork\n");

    // 작업 완료된 블록 전송
    if (send(sockfd, (void*)&block, sizeof(Block), 0) < 0) {
        perror("Error sending block");
        exit(1);
    }

    printf("Block sent.\n");

    // 블록 정보 출력
    printBlockInfo(&block);

    // 소켓 종료
    close(sockfd);

    return 0;
}
