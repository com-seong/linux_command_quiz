#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "quiz.h"

// 문자열 끝 엔터 제거
void trim_newline(char *str) {
    int len = strlen(str);
    if (len > 0 && str[len - 1] == '\n') {
        str[len - 1] = '\0';
    }
}

// [추가됨] 쉘 스크립트가 점수를 읽어갈 수 있도록 숫자만 저장하는 함수
void save_score_for_shell(int score) {
    FILE *fp = fopen("records/last_score_num.txt", "w");
    if (fp != NULL) {
        fprintf(fp, "%d", score);
        fclose(fp);
    }
}

// Git 자동화 및 결과 파일 저장 함수
void save_result_and_commit(char *topic, int score, int total, char *log_content) {
    char logpath[100] = "records/history_log.txt";    // Git에 기록할 로그
    
    // 1. Git 기록용 로그 파일 저장 (append 모드)
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

    // [추가됨] 쉘 스크립트용 점수 파일 생성 호출
    save_score_for_shell(score);

    // 2. Git 자동 커밋 실행 (System Call)
    printf("\n[System] 점수 기록을 Git에 저장 중...\n");
    char cmd[512];
    
    // [수정됨] git add 뒤에 -f 옵션을 붙여서 무시된 파일도 강제로 넣음!
    sprintf(cmd, "git add -f %s && git commit -m \"Record: %s Quiz Score %d/%d\"", 
            logpath, topic, score, total);
            
    int result = system(cmd);
    if (result == 0) {
        printf("[System] Git 저장 완료!\n");
    } else {
        printf("[System] Git 저장 실패 (Git이 설치되어 있지 않거나 충돌 발생)\n");
    }
}

int main(int argc, char *argv[]) {
    // [수정됨] 환경변수(getenv) 대신 실행 인자(argv)로 파일 경로를 받음
    // 쉘 스크립트 실행 명령: ./quiz_app [파일경로]
    if (argc < 2) {
        printf("[Error] 문제 파일 경로가 전달되지 않았습니다.\n");
        return 1;
    }

    char *file_path = argv[1]; // 첫 번째 인자가 파일 경로
    char *topic = "Quiz";      // 기본 주제명

    // 파일 경로에서 주제 추측 (로그 기록용)
    if (strstr(file_path, "git")) topic = "Git";
    else if (strstr(file_path, "linux")) topic = "Linux";

    FILE *fp = fopen(file_path, "r");
    if (fp == NULL) {
        printf("[Error] 문제 파일(%s)을 열 수 없습니다.\n", file_path);
        return 1;
    }

    // 문제 파싱
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

    // 퀴즈 진행
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

            // 힌트 처리
            if (strcmp(input, "hint") == 0) {
                printf("   [Hint] %s\n\n", quiz_list[i].hint);
                continue;
            }
            
            // 정답 확인
            if (strcmp(input, quiz_list[i].answer) == 0) {
                printf("   -> 정답입니다!\n\n");
                score++;
                char temp[1024];
                sprintf(temp, "Q%d: Correct\n", i+1);
                strcat(log_buffer, temp);
            } else {
                printf("   -> 오답입니다. (정답: %s)\n\n", quiz_list[i].answer);
                char temp[1024];
                sprintf(temp, "Q%d: Wrong\n", i+1);
                strcat(log_buffer, temp);
            }
            break; // 다음 문제로
        }
    }

    printf("==============================\n");
    printf(" 최종 점수: %d / %d\n", score, count);
    printf("==============================\n");

    // 결과 저장 및 Git 커밋 수행
    save_result_and_commit(topic, score, count, log_buffer);

    return 0;
}