#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "quiz.h"

// 문자열 끝 엔터 제거 (입력 처리용)
void trim_newline(char *str) {
    int len = strlen(str);
    if (len > 0 && str[len - 1] == '\n') {
        str[len - 1] = '\0';
    }
}

// Git 자동화 및 결과 파일 저장 함수 (요구사항 3, 4)
void save_result_and_commit(char *topic, int score, int total, char *log_content) {
    char filepath[100] = "records/current_score.txt"; // 파트너에게 넘겨줄 점수
    char logpath[100] = "records/history_log.txt";    // Git에 기록할 로그
    
    // 1. 파트너(Shell)가 읽을 점수 파일 저장
    FILE *fp_score = fopen(filepath, "w");
    if (fp_score != NULL) {
        fprintf(fp_score, "%d/%d", score, total);
        fclose(fp_score);
    }

    // 2. Git 기록용 로그 파일 저장 (append 모드)
    FILE *fp_log = fopen(logpath, "a");
    if (fp_log != NULL) {
        time_t t = time(NULL);
        struct tm tm = *localtime(&t);
        fprintf(fp_log, "\n[Date: %02d-%02d %02d:%02d] Topic: %s | Score: %d/%d\n", 
                tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, topic, score, total);
        fprintf(fp_log, "%s", log_content); // 상세 오답 노트 등 기록
        fprintf(fp_log, "--------------------------------------------------\n");
        fclose(fp_log);
    }

    // 3. Git 자동 커밋 실행 (System Call)
    printf("\n[System] 점수 기록을 Git에 저장 중...\n");
    char cmd[512];
    
    // 기록 파일을 스테이징하고 커밋 (Git이 설치된 환경이어야 함)
    sprintf(cmd, "git add %s && git commit -m \"Record: %s Quiz Score %d/%d\"", 
            logpath, topic, score, total);
            
    int result = system(cmd);
    if (result == 0) {
        printf("[System] Git 저장 완료!\n");
    } else {
        printf("[System] Git 저장 실패 (Git이 설치되어 있지 않거나 충돌 발생)\n");
    }
}

int main(int argc, char *argv[]) {
    // 인자 확인 (주제, 난이도 - 로깅용)
    char *topic = (argc > 1) ? argv[1] : "Unknown";
    // 난이도는 현재 로직에선 파일 경로로 구분되므로 참고용으로만 씀

    // 1. 환경변수에서 문제 파일 경로 가져오기 (요구사항 2)
    char *file_path = getenv("QUIZ_FILE");
    if (file_path == NULL) {
        printf("[Error] 환경변수 QUIZ_FILE이 설정되지 않았습니다.\n");
        return 1;
    }

    FILE *fp = fopen(file_path, "r");
    if (fp == NULL) {
        printf("[Error] 문제 파일(%s)을 열 수 없습니다.\n", file_path);
        return 1;
    }

    // 2. 문제 파싱
    QuizItem quiz_list[MAX_Q];
    int count = 0;
    char line[MAX_LINE];

    while (fgets(line, sizeof(line), fp) != NULL && count < MAX_Q) {
        trim_newline(line);
        // 포맷: 문제|정답|힌트
        char *ptr = strtok(line, "|");
        if (ptr != NULL) strcpy(quiz_list[count].question, ptr);
        
        ptr = strtok(NULL, "|");
        if (ptr != NULL) strcpy(quiz_list[count].answer, ptr);
        
        ptr = strtok(NULL, "|");
        if (ptr != NULL) strcpy(quiz_list[count].hint, ptr);
        
        count++;
    }
    fclose(fp);

    // 3. 퀴즈 진행
    int score = 0;
    char input[MAX_LINE];
    char log_buffer[4096] = ""; // Git 로그에 남길 상세 내용

    printf("\n>>> [%s] 퀴즈 시작! (총 %d문제) <<<\n", topic, count);
    printf(">>> 'hint'를 입력하면 힌트를 볼 수 있습니다.\n\n");

    for (int i = 0; i < count; i++) {
        while (1) { // 힌트 입력 시 재입력을 위해 루프
            printf("[Q%d] %s\n답 입력: ", i + 1, quiz_list[i].question);
            
            if (fgets(input, sizeof(input), stdin) == NULL) break;
            trim_newline(input);

            // 힌트 처리 (요구사항 1)
            if (strcmp(input, "hint") == 0) {
                printf("   [Hint] %s\n\n", quiz_list[i].hint);
                continue;
            }
            
            // 정답 확인
            if (strcmp(input, quiz_list[i].answer) == 0) {
                printf("   -> 정답입니다!\n\n");
                score++;
                // 로그용 데이터 축적
                char temp[1024];
                sprintf(temp, "Q%d: Correct (%s)\n", i+1, quiz_list[i].question);
                strcat(log_buffer, temp);
            } else {
                printf("   -> 오답입니다. (정답: %s)\n\n", quiz_list[i].answer);
                // 로그용 데이터 축적
                char temp[1024];
                sprintf(temp, "Q%d: Wrong (Input: %s / Ans: %s)\n", i+1, input, quiz_list[i].answer);
                strcat(log_buffer, temp);
            }
            break; // 다음 문제로
        }
    }

    printf("==============================\n");
    printf(" 최종 점수: %d / %d\n", score, count);
    printf("==============================\n");

    // 4. 결과 저장 및 Git 커밋 수행 (요구사항 3, 4)
    save_result_and_commit(topic, score, count, log_buffer);

    return 0;
}