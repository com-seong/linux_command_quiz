// src/quiz.h
#ifndef QUIZ_H
#define QUIZ_H

#define MAX_LINE 256
#define MAX_Q 50

typedef struct {
    char question[MAX_LINE];
    char answer[MAX_LINE];
    char hint[MAX_LINE];
} QuizItem;

// 문자열 끝의 개행문자 제거 함수
void trim_newline(char *str);

// Git 자동 커밋 함수
void save_result_and_commit(char *topic, int score, int total, char *log_content);

#endif