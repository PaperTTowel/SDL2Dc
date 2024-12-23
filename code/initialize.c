#include "global.h"

// DialogueText 초기화 함수
void initializeDialogueText(DialogueText *dialogue) {
    if(dialogue == NULL) return;

    // 이름 초기화
    memset(dialogue->name, 0, sizeof(dialogue->name));

    // ID 초기화
    dialogue->ID = -1;             // -1은 초기화 상태를 나타냄
    dialogue->currentID = 0;      // 초기 상태
    dialogue->previousId = 0;     // 초기 상태

    // 텍스트 초기화
    for(int i = 0; i < 4; i++){
        memset(dialogue->text[i], 0, sizeof(dialogue->text[i]));
    }
    dialogue->textLineCount = 0;

    // 선택지 초기화
    dialogue->optionCount = 0;
    for(int i = 0; i < 4; i++){
        memset(dialogue->options[i], 0, sizeof(dialogue->options[i]));
        dialogue->nextIds[i] = 0;  // 선택지의 다음 ID는 초기 상태로 -1 설정
    }
}

// 모든 대화 목록 초기화
void initializeAllDialogues(DialogueText *dialogues, int count){
    if(dialogues == NULL || count <= 0) return;

    for(int i = 0; i < count; i++){
        initializeDialogueText(&dialogues[i]);
    }
}

// 애니메이션 프레임 메모리 해제 예제
void freeAnimations(tileAnimation *animations, int count){
    for(int i = 0; i < count; i++){
        if(animations[i].frames != NULL){
            free(animations[i].frames); // 동적 메모리 해제
            animations[i].frames = NULL;
        }
    }
}

void freeAnimationFrames(SDL_Texture **frames, int frameCount){
    if (frames) {
        for (int i = 0; i < frameCount; i++) {
            if (frames[i]) {
                SDL_DestroyTexture(frames[i]); // 텍스처 해제
            }
        }
        free(frames); // 프레임 배열 메모리 해제
    }
}