#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <openssl/sha.h>

// 서버 포트
#define SERVER_PORT 8080

// 해시 값의 비트 단위로 표현된 난이도
#define DIFFICULTY_BITS 7 // 나중에 7이나 8로 변경!
#define SHA256_BLOCK_SIZE 64

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
    int sockfd, newsockfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len;
    char challenge[256];
    char receivedHash[SHA256_DIGEST_LENGTH];

    // 서버 소켓 생성
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Error creating socket");
        exit(1);
    }

    // 서버 주소 설정
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(SERVER_PORT);

    // 소켓에 주소 할당
    if (bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error binding socket");
        exit(1);
    }

    // 클라이언트의 연결 요청을 대기
    if (listen(sockfd, 1) < 0) {
        perror("Error listening for connections");
        exit(1);
    }

    printf("Server started. Waiting for connections...\n");

    // 클라이언트의 연결 수락
    client_len = sizeof(client_addr);
    newsockfd = accept(sockfd, (struct sockaddr*)&client_addr, &client_len);
    if (newsockfd < 0) {
        perror("Error accepting connection");
        exit(1);
    }

    printf("Client connected.\n");

    // challenge 값 생성 및 클라이언트에 전송
    const char* challengeValue = "202116932021167020210604"; // 학번
    strncpy(challenge, challengeValue, sizeof(challenge));
    if (send(newsockfd, challenge, sizeof(challenge), 0) < 0) {
        perror("Error sending challenge to client");
        exit(1);
    }

    printf("Challenge sent to client: %s\n", challenge);

    // 클라이언트로부터 해시 값을 받음
    if (recv(newsockfd, receivedHash, sizeof(receivedHash), 0) < 0) {
        perror("Error receiving hash from client");
        exit(1);
    }

    printf("Received hash from client.\n");
    printf("Hash: %s\n", receivedHash);

    // 해시 값 검증
    if (isValidHash(receivedHash, DIFFICULTY_BITS)) {
        printf("Hash is valid. Client successfully completed the challenge.\n");
        // 여기에 추가 작업 수행
    } else {
        printf("Hash is invalid. Client failed to complete the challenge.\n");
    }

    // 소켓 종료
    close(newsockfd);
    close(sockfd);

    return 0;
}
