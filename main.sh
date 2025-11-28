#!/bin/bash

# ==============================================================================
# 0. 설정 및 변수 초기화
# ==============================================================================
# C 실행 파일 경로 (컴파일 후 생성됨)
QUIZ_EXEC="./src/quiz" 
# C 소스 파일 경로
QUIZ_SOURCE="./src/quiz.c"
# 점수 기록 로그 파일 경로
RECORD_FILE="./records/current_score.txt" 

# 폴더가 없으면 생성 (스크립트 실행의 안정성 확보)
mkdir -p records data



# ------------------------------------------------------------------------------
# Git 최고점 기록 함수 (F3의 Git 연동 부분)
# 최고 점수 달성 시 Git 커밋을 실행합니다.
# ------------------------------------------------------------------------------
git_commit_score() {
    local TOPIC=$1
    local SCORE=$2
    local DATE=$(date +%Y-%m-%d)
    
    echo "==================================="
    echo "🎉 New High Score! Git Commit 시작..."
    
    # 1. 최고 점수 정보를 BEST_SCORE.txt 파일에 저장합니다.
    echo "$DATE|$TOPIC|$SCORE" > best_score.txt
    
    # 2. Git에 기록하고 푸시합니다.
    #    주의: 실제 프로젝트에서는 git push 전에 원격 연결과 인증이 필요합니다.
    git add best_score.txt
    # 커밋 메시지: [F3] 최고점 달성 [주제] XX점 기록
    git commit -m "F3: High Score - [$TOPIC] $SCORE점 달성"
    # git push origin main # 실제 팀 프로젝트 시 활성화
    
    echo "✅ Git Commit 완료. 최고점: $SCORE점"
    echo "==================================="
}


# ==============================================================================
# 1. C 코드 컴파일
# ==============================================================================
echo "--- 퀴즈 프로그램 컴파일 중... ---"

# gcc를 사용하여 src/quiz.c를 src/quiz로 컴파일
gcc "$QUIZ_SOURCE" -o "$QUIZ_EXEC"

# 컴파일 성공 여부 확인
if [ $? -ne 0 ]; then
    echo "❌ [오류] C 컴파일에 실패했습니다. src/quiz.c 파일을 확인하세요."
    exit 1
fi
echo "✅ 컴파일 성공. 퀴즈 실행 파일: $QUIZ_EXEC"


# ==============================================================================
# 2. 퀴즈 주제 선택 (F2)
# ==============================================================================
echo ""
echo "--- 퀴즈 주제를 선택해 주세요 ---"
TOPIC_NAME=""

# 2-1. 주제 선택 메뉴
select topic_choice in "Linux 명령어 퀴즈" "Git 명령어 퀴즈" "종료"; do
    case $topic_choice in
        "Linux 명령어 퀴즈")
            TOPIC_NAME="linux"
            break
            ;;
        "Git 명령어 퀴즈")
            TOPIC_NAME="git"
            break
            ;;
        "종료")
            echo "프로그램을 종료합니다."
            exit 0
            ;;
        *)
            echo "잘못된 선택입니다. 다시 선택해 주세요."
            ;;
    esac
done

# 2-2. 난이도 선택 메뉴
echo ""
echo "--- 난이도를 선택해 주세요 ---"
select level_choice in "Easy" "Hard"; do
    case $level_choice in
        "Easy")
            LEVEL_NAME="easy"
            break
            ;;
        "Hard")
            LEVEL_NAME="hard"
            break
            ;;
        *)
            echo "잘못된 선택입니다. 다시 선택해 주세요."
            ;;
    esac
done

# 2-3. 최종 퀴즈 파일 경로 결정 (환경 변수 설정)
# 예: ./data/linux_easy.txt 또는 ./data/git_hard.txt
QUIZ_PATH="./data/${TOPIC_NAME}_${LEVEL_NAME}.txt" 
export QUIZ_FILE_PATH="$QUIZ_PATH" # (선택사항: 환경 변수 설정)

echo "✅ 퀴즈 설정 완료: 주제=$TOPIC_NAME, 난이도=$LEVEL_NAME"

# 선택된 퀴즈 데이터 파일이 존재하는지 최종 확인
if [ ! -f "$QUIZ_PATH" ]; then
    echo "❌ [오류] 데이터 파일($QUIZ_PATH)이 없습니다. data 폴더와 파일명을 확인하세요."
    exit 1
fi


# ==============================================================================
# 3. C 프로그램 실행 및 점수 처리 (F1 실행 & F3 기록)
# ==============================================================================
echo ""
echo "--- 퀴즈 시작: $TOPIC_NAME ---"

# C 실행 파일 호출 및 표준 출력(점수)을 FINAL_SCORE 변수에 저장
# C 프로그램은 quiz.c에서 최종 점수만 printf로 출력해야 합니다.

# 1. 그냥 실행합니다 (화면에 문제 보이게)
"$QUIZ_EXEC" "$QUIZ_PATH"

# 2. 실행 끝난 후, C가 몰래 적어두고 간 점수 쪽지를 읽습니다.
if [ -f "records/last_score_num.txt" ]; then
    FINAL_SCORE=$(cat records/last_score_num.txt)
else
    FINAL_SCORE=0
fi


echo "==================================="
echo "⭐ 획득 점수: $FINAL_SCORE점"
echo "==================================="

# ==============================================================================
# 4. 최고 점수 기록 및 비교 (F3)
# ==============================================================================

DATE_TIME=$(date +%Y%m%d_%H%M%S)

# 4-1. 현재 점수를 로그 파일에 추가 (날짜|점수|주제 형식)
echo "$DATE_TIME|$FINAL_SCORE|$TOPIC_NAME" >> "$RECORD_FILE"
echo "✅ 점수가 기록되었습니다: $RECORD_FILE"

# 4-2. 최고 점수 추출
# 1. 기록 파일의 내용을 읽고 (cat)
# 2. 구분자('|')를 기준으로 두 번째 필드(점수)만 잘라내어 (cut -d'|' -f2)
# 3. 내림차순 숫자 정렬 후 (sort -rn)
# 4. 가장 높은 점수(첫 번째 줄)만 추출 (head -n 1)
HIGHEST_SCORE=$(cat "$RECORD_FILE" | cut -d'|' -f2 | sort -rn | head -n 1)

echo "📊 역대 최고 점수: $HIGHEST_SCORE점"

# 4-3. 최고 점수 달성 확인 및 Git 커밋
# 획득 점수와 최고 점수가 같거나 크다면 (숫자 비교: -ge)
if [ "$FINAL_SCORE" -ge "$HIGHEST_SCORE" ]; then
    git_commit_score "$TOPIC_NAME" "$FINAL_SCORE"
else
    echo "다음 기회에 최고 기록에 도전하세요!"
fi

# ==============================================================================