#include "global.h"

void updateFrame(){
    int currentTime = SDL_GetTicks();
    int currentFrameDelay = isMoving ? movingFrameDelay : idleFrameDelay;

    if(currentTime > lastFrameTime + currentFrameDelay){
        frame = (frame + 1) % 8; // 8프레임
        lastFrameTime = currentTime;
    }
}

void updateAnimation(tileAnimation *animation){
    if(!animation->isActive || animation->isFinished){
        return; // 활성화되지 않았거나 이미 종료된 애니메이션은 업데이트하지 않음
    }
    else if(animation->isActive && !animation->isFinished){
        Uint32 currentTime = SDL_GetTicks();

        // 프레임 갱신
        if(currentTime - animation->lastFrameTime >= animation->frameDuration){
            animation->lastFrameTime = currentTime;
            animation->currentFrame++;

            // 마지막 프레임에 도달하면 멈춤
            if(animation->currentFrame >= animation->frameCount){
                animation->currentFrame = animation->frameCount - 1; // 마지막 프레임에서 멈춤
                animation->isFreezed = SDL_TRUE;                     // 일시정지
            }
        }
    }
}

void updateMiniGame(TTF_Font *font){
    if (!isMiniGameActive) return;
    static Uint8 previousState[SDL_NUM_SCANCODES] = {0};  // 이전 키 상태 저장
    const Uint8 *state = SDL_GetKeyboardState(NULL);      // 현재 키 상태 가져오기

    Uint32 currentTime = SDL_GetTicks();
    Uint32 elapsedTime = (currentTime - miniGameStartTime) / 1000; // 초 단위

    // 스페이스바가 눌린 상태인지 확인
    if(state[SDL_SCANCODE_SPACE] && !previousState[SDL_SCANCODE_SPACE]){
        spaceBarCount++;
    }
    memcpy(previousState, state, SDL_NUM_SCANCODES);

    // 5초가 지나면 미니게임 종료
    if(elapsedTime >= 5){
        isMiniGameActive = SDL_FALSE;

        // 종료 메시지 설정
        activeTextDisplay.text = buffer;
        activeTextDisplay.startTime = currentTime;
        activeTextDisplay.duration = 2000; // 2초 동안 표시

        // 관련 애니메이션 활성화
        for(int a = 0; a < animationCount; a++){
            if(animations[a].eventID == 1 || animations[a].eventID == 2){  // 특정 eventID 확인 (예: 1번 이벤트)
                animations[a].isActive = SDL_TRUE;  // 애니메이션 재생 시작
                printf("Animation with eventID %d activated.\n", animations[a].eventID);
            }
        }
        isDialogueActive = SDL_TRUE;
    }

    // 텍스트 효과 활성화 (10단위 카운트마다)
    if(spaceBarCount % 10 == 0 && spaceBarCount > 0 && !isEffectActive){
        isEffectActive = SDL_TRUE;
        effectStartTime = currentTime;
    }

    // 텍스트 효과: 크기와 색상 변경
    fontSize = 24;
    textEventColor = BasicColor;

    if(isEffectActive){
        Uint32 effectElapsedTime = currentTime - effectStartTime;

        if(effectElapsedTime < 165){ // 0~300ms 동안 효과 적용
            fontSize = 32; // 텍스트 크기를 키움
            textEventColor = YelloColor; // 노란색으로 변경
        }
        else{
            isEffectActive = SDL_FALSE; // 효과 종료
        }
    }

    sprintf(buffer, "띵동대쉬: %d, 초당 %d연타!", spaceBarCount, spaceBarCount / 5);
}

void updatePhysics(){
    // 중력 적용
    velocityY += gravity;
    playerY += velocityY; // y좌표 변경

    // 플레이어의 rect를 업데이트
    playerRect.x = playerX - camera.x; 
    playerRect.y = playerY - camera.y;

    // 모든 플랫폼과 충돌 체크
    for(int i = 0; i < platformCount; i++){
        SDL_Rect platformRect = {platforms[i].x - camera.x, platforms[i].y - camera.y, platforms[i].width, platforms[i].height};
        // y축 충돌 체크
        if(SDL_HasIntersection(&playerRect, &platformRect)){
            if(velocityY > 0){
                int dontMovePlayerY = 0;
                if(playerRect.x < platformRect.x){
                    playerX = platformRect.x - playerRect.w + camera.x;
                }
                if(playerRect.x + playerRect.w > platformRect.x + platformRect.w) {
                    playerX = platformRect.x + platformRect.w + camera.x - 72;
                }
                playerY = platformRect.y - playerRect.h; // y좌표 조정
                velocityY = 0; // 속도 0으로 초기화
                isJumping = 0; // 점프 상태 해제
            }
            break; // 첫 번째 충돌만 처리
        }
    }
    // 플레이어의 rect를 업데이트 (y좌표 조정 후)
    playerRect.y = playerY - camera.y;
    playerRect.x = playerX - camera.x;

    // x축 충돌
    for(int i = 0; i < platformCount; i++){
        SDL_Rect platformRect = {platforms[i].x - camera.x, platforms[i].y - camera.y, platforms[i].width, platforms[i].height};
        //printf("%d  %d  %d\n", platformRect.x, platformRect.y, platformRect.w);
        if(SDL_HasIntersection(&playerRect, &platformRect)){
            if(playerRect.x < platformRect.x){ // 플레이어가 플랫폼의 왼쪽에 있을 때
                playerX = platformRect.x - playerRect.w + camera.x; // 카메라 좌표 반영
            }
            else{ // 플레이어가 플랫폼의 오른쪽에 있을 때
                playerX = platformRect.x + platformRect.w + camera.x; // 카메라 좌표 반영
            }
            break; // 첫 번째 충돌을 찾으면 루프 종료
        }
    }
    playerRect.x = playerX - camera.x;
}

void updateCamera(float deltaTime){
    const float cameraSpeed = 5.0f; // 부드러운 카메라 속도 (픽셀/초)

    // 카메라의 x, y 좌표를 플레이어의 x 좌표를 기준으로 부드럽게 조정
    cameraX += (playerX - cameraX - (camera.w / 2 - playerRect.w / 2)) * cameraSpeed * deltaTime;
    //camera.y = (int)(playerY - (camera.h / 2 - playerRect.h / 2));

    // 카메라가 화면의 경계를 넘지 않도록 제한
    if (cameraX < 0) cameraX = 0;

    camera.x = (int)cameraX; // 카메라 rect의 x 값은 int로 변환 (분리용)
}