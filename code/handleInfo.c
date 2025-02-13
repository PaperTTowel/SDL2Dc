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

void handleShopInput(Shop *shop, int *playerGold){
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

    // Z 키 눌림 감지
    if(state[SDL_SCANCODE_Z] && !previousState[SDL_SCANCODE_Z]){
        int price = getItemPrice(items[shop->selectedItem].name);
        if(*playerGold >= price && items[shop->selectedItem].stock > 0){
            *playerGold -= price;
            items[shop->selectedItem].stock--;
            printf("Purchased %s\n", items[shop->selectedItem].name);
            playSoundEffect("cash");
        }
        else{
            playSoundEffect("cansel");
            printf("Not enough gold or item out of stock!\n");
        }
    }

    // ESC 키 눌림 감지
    if(state[SDL_SCANCODE_ESCAPE] && !previousState[SDL_SCANCODE_ESCAPE]){
        isShopVisible = SDL_FALSE;
        playSoundEffect("cansel");
        printf("Shop closed!\n");
    }

    // 현재 키 상태를 이전 키 상태로 복사
    memcpy(previousState, state, SDL_NUM_SCANCODES);
}

// 띵동대쉬 초당 16연타 이벤트!
void startDingDongDashMiniGame(){
    isMiniGameActive = SDL_TRUE;
    miniGameStartTime = SDL_GetTicks();
    spaceBarCount = 0;
}

void handleChoiceInput(DialogueText *dialogue, int *selectedOption){
    static Uint8 previousState[SDL_NUM_SCANCODES] = {0};  // 이전 키 상태 저장
    const Uint8 *state = SDL_GetKeyboardState(NULL);      // 현재 키 상태 가져오기
    int optionCount = dialogue->optionCount;              // 현재 대화의 선택지 개수

    // UP 키 눌림 감지
    if(state[SDL_SCANCODE_UP] && !previousState[SDL_SCANCODE_UP] && optionCount != 0){
        *selectedOption = (*selectedOption - 1 + optionCount) % optionCount;
        printf("Choosing Option: %d\n", *selectedOption);
    }

    // DOWN 키 눌림 감지
    if(state[SDL_SCANCODE_DOWN] && !previousState[SDL_SCANCODE_DOWN] && optionCount != 0){
        *selectedOption = (*selectedOption + 1) % optionCount;
        printf("Choosing Option: %d\n", *selectedOption);
    }

    // Z 키 눌림 감지
    if(state[SDL_SCANCODE_Z] && !previousState[SDL_SCANCODE_Z]){
        int nextId = dialogue->nextIds[*selectedOption];

        // 디버깅 존
        printf("Selected option: %s\n", dialogue->options[*selectedOption]);
        printf("All nextIds for current dialogue:\n");
        for(int i = 0; i < dialogue->optionCount; i++){
            printf("Option %d: %s -> nextId: %d\n", i, dialogue->options[i], dialogue->nextIds[i]);
        }
        // 디버깅 존 끝

        if(nextId == -1){
            // 대화가 종료될때 SE 재생
            if(strlen(dialogues[dialogues->currentID].SE) > 0){
                playSoundEffect(dialogues[dialogues->currentID].SE);
            }
            
            // 대화 종료
            isDialogueActive = SDL_FALSE;
            animations->isActive = SDL_FALSE;
            animations->isFreezed = SDL_FALSE;
            animations->isFinished = SDL_TRUE;
            initializeAllDialogues(dialogues, 5);
            freeAnimations(animations, 10);
            freeAnimationFrames(animations->frames, animations->frameCount);


            printf("Dialogue ended.\n");
        }
        else if(nextId == -2) running = SDL_FALSE;
        else{
            // 다음 대화를 구조체에 로드
            dialogues->currentID = nextId;
            DialogueText *debugDialogue = &dialogues[nextId];

            printf("Dialogue id changed to %d\n", dialogues->currentID);
            for(short t = 0; t <= debugDialogue->textLineCount; t++){
                printf("Dialogue Text in [%d]\n: %s\n", t, debugDialogue->text[t]);
            }
        }
    }
    // 현재 키 상태를 이전 키 상태로 복사
    memcpy(previousState, state, SDL_NUM_SCANCODES);
}

// 이벤트 구분
void handleEvent(int eventID){
    switch(eventID){
        case 1:
            startDingDongDashMiniGame();
            break;
        case 2:
            startDingDongDashMiniGame();
            break;
        case 3:
            startDingDongDashMiniGame();
            break;
        case 4:
            for(int a = 0; a < animationCount; a++){
                animations[a].isActive = SDL_TRUE;
            }
            isDialogueActive = SDL_TRUE;
            break;
        case 5:
            isDialogueActive = SDL_TRUE;
            break;
        case 6:
            isDialogueActive = SDL_TRUE;
            break;
        case 7:
            isDialogueActive = SDL_TRUE;
            break;
    }
}