#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <openssl/sha.h>

#define MAX_WORKERS 10
#define CHALLENGE_SIZE 50
#define HASH_DIFFICULTY 8

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

    return nonce;
}

void measurePoWTime() {
    clock_t start, end;
    double cpu_time_used;

    start = clock();
    // Proof of Work 수행
    char challenge[CHALLENGE_SIZE];
    snprintf(challenge, CHALLENGE_SIZE, "학번||학번||학번||이름||이름||이름");
    int nonce = performPoW(challenge);
    printf("Proof of Work complete. Nonce: %d\n", nonce);

    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;

    printf("PoW Time: %.6f seconds\n", cpu_time_used);
}

int main() {
    int workers[MAX_WORKERS];
    int numWorkers = 0;

    // Working Server들과의 연결 설정
    struct sockaddr_in serverAddr;
    int serverPort = 8888; // 예시로 포트 번호 8888을 사용
    for (int i = 0; i < MAX_WORKERS; i++) {
        // Socket 생성
        int sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) {
            perror("Failed to create socket");
            exit(EXIT_FAILURE);
        }

        // 서버 주소 설정
        memset(&serverAddr, 0, sizeof(serverAddr));
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(serverPort);
        if (inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr) <= 0) {
            perror("Invalid address or address not supported");
            exit(EXIT_FAILURE);
        }

        // 연결 시도
        if (connect(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
            perror("Connection failed");
            exit(EXIT_FAILURE);
        }

        // 연결 성공
        workers[i] = sockfd;
        numWorkers++;

        // 포트 번호 증가 (다음 Worker와 다른 포트 번호 사용)
        serverPort++;
    }

    // challenge 생성
    char challenge[CHALLENGE_SIZE];
    snprintf(challenge, CHALLENGE_SIZE, "학번||학번||학번||이름||이름||이름");

    // distributeChallenge 함수를 통해 challenge 값 배포
    distributeChallenge(challenge, workers, numWorkers);

    // handleWorkerResults 함수를 통해 Working Server들의 결과 확인
    handleWorkerResults(workers, numWorkers);

    // Proof of Work 시간 측정
    measurePoWTime();

    // 연결 종료
    for (int i = 0; i < numWorkers; i++) {
        close(workers[i]);
    }

    return 0;
}
