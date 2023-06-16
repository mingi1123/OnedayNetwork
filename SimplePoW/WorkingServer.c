#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>
#include <openssl/sha.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8080

#define DIFFICULTY 5
#define SHA256_BLOCK_SIZE 32 
#define TARGET_PREFIX_MAX_LENGTH 9

typedef struct {
    uint32_t index;
    uint64_t timestamp;
    char data[256];
    char previousHash[SHA256_BLOCK_SIZE + 1];
    char hash[SHA256_BLOCK_SIZE * 2 + 1];
    uint32_t nonce;
    uint32_t difficulty;
    char targetPrefix[TARGET_PREFIX_MAX_LENGTH];
} Block;

void calculateHash(Block* block, char* hash) {
    char data[512]; 
    int dataSize = snprintf(data, sizeof(data), "%s%u", block->data, block->nonce);
    if (dataSize < 0 || dataSize >= sizeof(data)) {
        fprintf(stderr, "Error: Buffer overflow.\n");
        exit(1);
    }

    SHA256_CTX ctx;
    SHA256_Init(&ctx);
    SHA256_Update(&ctx, (uint8_t*)data, dataSize);
    unsigned char hashRaw[SHA256_DIGEST_LENGTH];
    SHA256_Final(hashRaw, &ctx);
    for(int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(hash + (i * 2), "%02x", hashRaw[i]);
    }
    hash[SHA256_BLOCK_SIZE * 2] = '\0';
}

void performProofOfWork(Block* block) {
    char hash[SHA256_BLOCK_SIZE * 2 + 1];
    char target[DIFFICULTY + 1];
    for (int i = 0; i < DIFFICULTY; i++) {
        target[i] = '0';
    }
    target[DIFFICULTY] = '\0';

    while (1) {
        block->nonce++;
        calculateHash(block, hash);
        if (strncmp(hash, target, DIFFICULTY) == 0) {
            strncpy(block->hash, hash, SHA256_BLOCK_SIZE * 2 + 1);
            break;
        }
    }
}

void printBlockInfo(Block* block) {
    printf("Block Index: %u\n", block->index);
    printf("Timestamp: %lu\n", block->timestamp);
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

    // 블록 정보 출력
    printBlockInfo(&block);

    // 작업 증명 수행
    struct timeval start_time, end_time;
    gettimeofday(&start_time, NULL);
    printf("Start performProofOfWork at %ld.%06ld\n", start_time.tv_sec, start_time.tv_usec);
    performProofOfWork(&block);
    gettimeofday(&end_time, NULL);
    printf("End performProofOfWork at %ld.%06ld\n", end_time.tv_sec, end_time.tv_usec);

    // 소요시간 출력 (초.마이크로초)
    double elapsed_time = (end_time.tv_sec - start_time.tv_sec) + 
                          ((end_time.tv_usec - start_time.tv_usec)/1000000.0);
    printf("Elapsed time for performProofOfWork: %f seconds\n", elapsed_time);

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
