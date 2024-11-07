#include "global.h"

void updateFrame(){
    int currentTime = SDL_GetTicks();
    int currentFrameDelay = isMoving ? movingFrameDelay : idleFrameDelay;

    if(currentTime > lastFrameTime + currentFrameDelay){
        frame = (frame + 1) % 8; // 8프레임
        lastFrameTime = currentTime;
    }
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
            playerY = platformRect.y - playerRect.h; // y좌표 조정
            velocityY = 0; // 속도 0으로 초기화
            isJumping = 0; // 점프 상태 해제
            break; // 첫 번째 충돌을 찾으면 루프 종료
        }
    }
    // 플레이어의 rect를 업데이트 (y좌표 조정 후)
    playerRect.x = playerX - camera.x;
    playerRect.y = playerY - camera.y;

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