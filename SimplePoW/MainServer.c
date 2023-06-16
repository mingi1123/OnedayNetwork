#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <openssl/sha.h>
// 해시 값의 비트 단위로 표현된 난이도
#define DIFFICULTY_BITS 4
#define SHA256_BLOCK_SIZE 64
// 난이도에 해당하는 해시 값의 접두사
const char* TARGET_PREFIX = "0000";

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

// 블록 정보 출력
void printBlockInfo(Block* block) {
    printf("Block Index: %u\n", block->index);
    printf("Timestamp: %llu\n", block->timestamp);
    printf("Data: %s\n", block->data);
    printf("Previous Hash: %s\n", block->previousHash);
    printf("Hash: %s\n", block->hash);
    printf("Nonce: %u\n", block->nonce);
}

// 쓰레드 함수
void* serverThread(void * data) {
    Block block;
    int client_sockfd = *((int *) data);

    // 블록 정보 설정
    block.index = 0;
    block.timestamp = time(NULL);
    strcpy(block.data, "20211693 변은영");
    strcpy(block.previousHash, "0000000000000000000000000000000000000000000000000000000000000000");
    block.nonce = 0;

    // 블록 정보 출력
    printBlockInfo(&block);

    // 블록 전송
    if (send(client_sockfd, (void*)&block, sizeof(Block), 0) < 0) {
        perror("Error sending block");
        exit(1);
    }
    printf("send\n");
    // 작업 완료된 블록 수신
    if (recv(client_sockfd, (void*)&block, sizeof(Block), 0) < 0) {
        perror("Error receiving block");
        exit(1);
    }
    printf("Data: %s\n", block.data);
    printf("Block Hash: %s\n", block.hash);
    printf("Nonce: %u\n", block.nonce);
    
    // 클라이언트 소켓 종료
    close(client_sockfd);
}

int main() {
    int sockfd, client_sockfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len;
    pthread_t thread;
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
    server_addr.sin_port = htons(8080);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    // 소켓과 주소 바인딩
    if (bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error binding socket");
        exit(1);
    }

    // 클라이언트 연결 대기
    if (listen(sockfd, 5) < 0) {
        perror("Error listening");
        exit(1);
    }

    printf("Server started. Waiting for connections...\n");

    while (1) {
        // 클라이언트 연결 수락
        client_len = sizeof(client_addr);
        client_sockfd = accept(sockfd, (struct sockaddr*)&client_addr, &client_len);
        if (client_sockfd < 0) {
            perror("Error accepting connection");
            exit(1);
        }
        printf("Client connected.\n");

        // 쓰레드 생성
        pthread_create(&thread, NULL, serverThread, (void *) &client_sockfd);
        pthread_detach(thread);
    }

    // 서버 소켓 종료
    close(sockfd);

    return 0;
}
