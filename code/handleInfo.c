#include "global.h"

void handleInput(const Uint8* state, float deltaTime, TTF_Font *font){
    const float baseSpeed = 300.0f;  // 기본 속도 (픽셀/초)
    float playerSpeed = baseSpeed * deltaTime;  // 델타 타임을 반영한 속도 계산

    if(state[SDL_SCANCODE_LEFT] || state[SDL_SCANCODE_A]){ // 좌측 이동
        playerX -= playerSpeed;
        direction = -1;
        isMoving = 1;
    }
    else if(state[SDL_SCANCODE_RIGHT] || state[SDL_SCANCODE_D]){ // 우측 이동
        playerX += playerSpeed;
        direction = 1;
        isMoving = 1;
    }
    else{ // 정지
        isMoving = 0;
    }
    /*
    if(state[SDL_SCANCODE_SPACE] && !isJumping){ // 점프
        velocityY = -15;
        isJumping = 1;
    }
    */
    if(state[SDL_SCANCODE_E]){
        if(!eKeyPressed){ // E 키가 처음 눌린 경우
            checkInteractions(&playerRect);
            eKeyPressed = 1; // E 키가 눌린 상태로 설정
        }
    }
    else{
        eKeyPressed = 0; // E 키가 떼어졌을 경우 상태 초기화
    }
}

void handleTextInteraction(const Interaction *interaction){
    if(interaction->propertyText != NULL){
        activeTextDisplay.text = interaction->propertyText;  // 텍스트 복사
        activeTextDisplay.startTime = SDL_GetTicks();        // 표시 시작 시간 기록
        activeTextDisplay.duration = 3000;                   // 3초 동안 표시
    }
}

void handleShopInput(Shop *shop, int *playerGold) {
    static Uint8 previousState[SDL_NUM_SCANCODES] = {0};  // 이전 키 상태 저장
    const Uint8 *state = SDL_GetKeyboardState(NULL);      // 현재 키 상태 가져오기

    // UP 키 눌림 감지
    if(state[SDL_SCANCODE_UP] && !previousState[SDL_SCANCODE_UP]){
        shop->selectedItem = (shop->selectedItem - 1 + itemCount) % itemCount;
        printf("Selected item: %s\n", items[shop->selectedItem].name);
    }

    // DOWN 키 눌림 감지
    if(state[SDL_SCANCODE_DOWN] && !previousState[SDL_SCANCODE_DOWN]){
        shop->selectedItem = (shop->selectedItem + 1) % itemCount;
        printf("Selected item: %s\n", items[shop->selectedItem].name);
    }

    // Enter 키 눌림 감지
    if(state[SDL_SCANCODE_RETURN] && !previousState[SDL_SCANCODE_RETURN]){
        int price = getItemPrice(items[shop->selectedItem].name);
        if(*playerGold >= price && items[shop->selectedItem].stock > 0){
            *playerGold -= price;
            items[shop->selectedItem].stock--;
            printf("Purchased %s\n", items[shop->selectedItem].name);
        }
        else{
            printf("Not enough gold or item out of stock!\n");
        }
    }

    // ESC 키 눌림 감지
    if(state[SDL_SCANCODE_ESCAPE] && !previousState[SDL_SCANCODE_ESCAPE]){
        isShopVisible = SDL_FALSE;
        printf("Shop closed!\n");
    }

    // 현재 키 상태를 이전 키 상태로 복사
    memcpy(previousState, state, SDL_NUM_SCANCODES);
}