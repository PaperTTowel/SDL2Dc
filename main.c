#include <SDL.h>
#include <SDL_image.h>
#include <stdio.h>

// 마지막 시간 기록
Uint32 lastTime = 0;

// 플레이어 좌표 (물리엔진과 렌더링(SDL_Rect) 분리용)
float playerX = 100.0f;
float playerY = 400.0f;

SDL_Rect playerRect = { 0, 0, 72, 72 }; // 렌더링할 플레이어 rect

int velocityY = 0;
int gravity = 1;
int direction = 1;
int isJumping = 0;
int isMoving = 0;

int frame = 0;
int idleFrameDelay = 500;  // 가만히 있을 때의 프레임 딜레이 (ms)
int movingFrameDelay = 70;  // 움직일 때의 프레임 딜레이 (ms)
int lastFrameTime = 0;

SDL_Rect platform = { 0, 450, 800, 25 }; // 플랫폼 정보(임시)

float cameraX = 0.0f; // 카메라 좌표 (분리용)
SDL_Rect camera = { 0, 0, 800, 600 }; // 카메라 정보

SDL_Texture* spriteSheet = NULL; // 스프라이트 시트 텍스처

// static Uint32 debugLastTime = 0; // 디버깅 렉걸릴 경우 사용

void updateFrame(){
    int currentTime = SDL_GetTicks();
    int currentFrameDelay = isMoving ? movingFrameDelay : idleFrameDelay;

    if(currentTime > lastFrameTime + currentFrameDelay){
        frame = (frame + 1) % 8; // 8프레임
        lastFrameTime = currentTime;
    }
}

void handleInput(const Uint8* state, float deltaTime){
    const float baseSpeed = 300.0f;  // 기본 속도 (픽셀/초)
    float playerSpeed = baseSpeed * deltaTime;  // 델타 타임을 반영한 속도 계산

    if(state[SDL_SCANCODE_LEFT]){ // 좌측 이동
        playerX -= playerSpeed;
        direction = -1;
        isMoving = 1;
    }
    else if(state[SDL_SCANCODE_RIGHT]){ // 우측 이동
        playerX += playerSpeed;
        direction = 1;
        isMoving = 1;
    }
    else{ // 정지
        isMoving = 0;
    }

    if(state[SDL_SCANCODE_SPACE] && !isJumping){ // 점프
        velocityY = -15;
        isJumping = 1;
    }
}

void updatePhysics(){
    velocityY += gravity;  // 중력 적용
    playerY += velocityY; // 캐릭터 y 좌표 변경

    if(SDL_HasIntersection(&playerRect, &platform)){ // 캐릭터가 플랫폼과 충돌할 때
        playerY = platform.y - playerRect.h; // 플랫폼의 y 좌표에서 캐릭터 높이만큼 뺌
        velocityY = 0; // 속도를 0으로
        isJumping = 0; // 점프 상태 해제
    }

    if(playerY > platform.y - playerRect.h){ // 바닥에 고정하는 부분
        playerY = platform.y - playerRect.h;
        velocityY = 0;
        isJumping = 0;
    }
}

void updateCamera(float deltaTime){
    const float cameraSpeed = 5.0f; // 부드러운 카메라 속도 (픽셀/초)

    // 카메라의 x 좌표를 플레이어의 x 좌표를 기준으로 부드럽게 조정
    cameraX += (playerX - cameraX - (camera.w / 2 - playerRect.w / 2)) * cameraSpeed * deltaTime;

    // 카메라가 화면의 경계를 넘지 않도록 제한
    if (cameraX < 0) cameraX = 0;
    // if (cameraX > 800 - camera.w) cameraX = 800 - camera.w; // 카메라 화면고정

    camera.x = (int)cameraX; // 카메라 rect의 x 값은 int로 변환 (분리용)
}

void render(SDL_Renderer* renderer){
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderFillRect(renderer, &platform);

    SDL_Rect srcRect;
    srcRect.w = 24;  // 원본 스프라이트 너비
    srcRect.h = 24;  // 원본 스프라이트 높이

    if(isMoving){
        srcRect.y = direction == -1 ? 24 : 48; // 움직일 때
        srcRect.x = frame * 24;
    }
    else{
        srcRect.y = 0;  // 가만히 있을 때
        srcRect.x = (direction == -1 ? 0 : 48) + (frame % 2) * 24;
    }

    // 렌더링할 캐릭터 크기
    SDL_Rect renderPlayer = { (int)playerX - camera.x, (int)playerY - camera.y, playerRect.w, playerRect.h }; // (72x72로 렌더링)
    SDL_RenderCopy(renderer, spriteSheet, &srcRect, &renderPlayer);

    SDL_RenderPresent(renderer);
}

int main(int argc, char* argv[]){
    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);

    SDL_Window* window = SDL_CreateWindow("SDL 2D Platform 그래픽 + 물리엔진 실험(연구)", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    SDL_Surface* tempSurface = IMG_Load("walk and idle.png");
    printf("sprite loaded!");
    if(!tempSurface){
        printf("load failed: %s\n", IMG_GetError());
        return 1;
    }

    spriteSheet = SDL_CreateTextureFromSurface(renderer, tempSurface);
    SDL_FreeSurface(tempSurface);

    SDL_Event event;
    SDL_bool running = SDL_TRUE;
    const Uint8* state = SDL_GetKeyboardState(NULL);

    while(running){
        while (SDL_PollEvent(&event)){
            if (event.type == SDL_QUIT) running = SDL_FALSE;
        }

        // 현재 시간과 마지막 시간을 기준으로 델타 타임 계산
        Uint32 currentTime = SDL_GetTicks();
        float deltaTime = (currentTime - lastTime) / 1000.0f; // 초 단위로 델타 타임 계산
        lastTime = currentTime;

        handleInput(state, deltaTime);
        updatePhysics();
        updateFrame();
        updateCamera(deltaTime);
        render(renderer);
        printf("player.x: %.3f   |   camera.x: %.3f\n", playerX, cameraX);
    }

    SDL_DestroyTexture(spriteSheet);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
    return 0;
}

/* 디버깅 렉걸릴경우 사용
        if (currentTime - debugLastTime > 200) {
            printf("player.x: %.3f   |   camera.x: %.3f\n", playerX, cameraX);
            debugLastTime = currentTime;
        }
        */