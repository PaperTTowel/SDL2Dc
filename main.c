#include <cJSON.h>
#include <SDL.h>
#include <SDL_image.h>
#include <stdio.h>
#include <stdlib.h>

#include "code\tileData.c"
#include "code\render.c"
#include "code\update.c"

// 마지막 시간 기록
Uint32 lastTime = 0;

// 플레이어 좌표 (물리엔진과 렌더링(SDL_Rect) 분리용)
float playerX = 165.0f;
float playerY = 500.0f;

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
int eKeyPressed = 0;

Map maps[100]; // MAX_MAPS는 최대 맵 수
int currentMapCount = 0; // 현재 로드된 맵 수

Platform platforms[100]; // 플랫폼 배열
int platformCount = 0; // 현재 플랫폼 수

Interaction interactions[100]; // 상호작용 배열
int interactionCount = 0;      // 현재 상호작용 수

float cameraX = 0.0f; // 카메라 좌표 (분리용)
SDL_Rect camera = { 0, 0, 800, 600 }; // 카메라 정보

SDL_Texture* spriteSheet = NULL; // 스프라이트 시트 텍스처

// JSON 데이터에서 추출한 맵 데이터 관련 정보
int mapWidth, mapHeight;
int tileWidth, tileHeight;
int *tileData; // 타일 데이터 배열

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
SDL_Texture *tilesetTexture = NULL;

void checkInteractions(SDL_Rect *playerRect);

void handleInput(const Uint8* state, float deltaTime){
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

    if(state[SDL_SCANCODE_SPACE] && !isJumping){ // 점프
        velocityY = -15;
        isJumping = 1;
    }
    if (state[SDL_SCANCODE_E]) {
        if (!eKeyPressed) { // E 키가 처음 눌린 경우
            checkInteractions(&playerRect);
            eKeyPressed = 1; // E 키가 눌린 상태로 설정
        }
    } else {
        eKeyPressed = 0; // E 키가 떼어졌을 경우 상태 초기화
    }
}

void handleInteraction(SDL_Rect *playerRect, Interaction *interactionZone){
    // 상호작용 이벤트 처리 (예: 문 열기, 대화 시작 등)
    printf("event! (name: %s, x: %.2f, y: %.2f)\n",interactionZone->name, interactionZone->x, interactionZone->y);
    // 추가적으로 상호작용 오브젝트에 따라 다른 동작을 처리할 수 있습니다.
}

void checkInteractions(SDL_Rect *playerRect){
    // 플레이어의 카메라 좌표 포함
    float playerXWithCamera = playerRect->x + camera.x;
    float playerYWithCamera = playerRect->y + camera.y;

    for (int i = 0; i < interactionCount; i++) {
        Interaction interactionZone = interactions[i];

        printf("Checking interaction with: %s (x=%.2f, y=%.2f)\n", 
               interactionZone.name, interactionZone.x, interactionZone.y);

        // 카메라 오프셋을 포함한 충돌 체크
        if ((playerXWithCamera < (interactionZone.x + interactionZone.width)) &&
            ((playerXWithCamera + playerRect->w) > interactionZone.x) &&
            (playerYWithCamera < (interactionZone.y + interactionZone.height)) &&
            ((playerYWithCamera + playerRect->h) > interactionZone.y)) {

            printf("Interaction triggered with: %s (x=%.2f, y=%.2f)\n", 
                   interactionZone.name, interactionZone.x, interactionZone.y);
            handleInteraction(playerRect, &interactionZone);
        }
    }
}

Uint32 fpsStartTime = 0;
int frameCount = 0;
float fps = 0.0f;

void updateFPS(){
    frameCount++;
    Uint32 currentTime = SDL_GetTicks();
    Uint32 elapsedTime = currentTime - fpsStartTime;

    if(elapsedTime >= 1000){ // 1초마다 FPS 계산
        fps = frameCount / (elapsedTime / 1000.0f); // 초당 프레임 수 계산
        fpsStartTime = currentTime; // 시간을 초기화
        frameCount = 0; // 프레임 카운트를 초기화
    }
}

char* readFile(const char* filename){
    FILE *file = fopen(filename, "rb");
    if(!file) return NULL;

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *data = (char*)malloc(length + 1);
    fread(data, 1, length, file);
    data[length] = '\0';

    fclose(file);
    return data;
}

int main(int argc, char* argv[]){
    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);
    if(SDL_Init(SDL_INIT_VIDEO) != 0){
        printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
        return 1;
    }

    if(IMG_Init(IMG_INIT_PNG) == 0){
        printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("SDL을 이용한 2D Platform C언어 공부연습용 게임", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    SDL_Surface* tempSurface = IMG_Load("resource\\walk and idle.png");
    printf("sprite loaded!\n");
    if(!tempSurface){
        printf("load failed: %s\n", IMG_GetError());
        return 1;
    }
    
    spriteSheet = SDL_CreateTextureFromSurface(renderer, tempSurface);
    SDL_FreeSurface(tempSurface);

    
    tilesetTexture = loadTexture("resource\\Tileset00.png", renderer);
    if (tilesetTexture == NULL) {
        printf("Failed to load tileset texture!\n");
        return -1;
    }

    // JSON 파일 읽기
    char *jsonData = readFile("tile\\roofTop.json");
    if(jsonData == NULL){
        printf("Error reading JSON file\n");
        return -1;
    }

    Map map;
    map.mapJson = cJSON_Parse(jsonData);
    if(map.mapJson == NULL){
        printf("Error parsing JSON\n");
        return -1;
    }

    // 맵의 크기와 타일 크기 추출
    cJSON *width = cJSON_GetObjectItem(map.mapJson, "width");
    cJSON *height = cJSON_GetObjectItem(map.mapJson, "height");
    cJSON *tileWidthItem = cJSON_GetObjectItem(map.mapJson, "tilewidth");
    cJSON *tileHeightItem = cJSON_GetObjectItem(map.mapJson, "tileheight");

    if(!cJSON_IsNumber(width) || !cJSON_IsNumber(height) || !cJSON_IsNumber(tileWidthItem) || !cJSON_IsNumber(tileHeightItem)){
        printf("Error in map dimensions\n");
        return -1;
    }

    map.mapWidth = width->valueint;
    map.mapHeight = height->valueint;
    map.tileWidth = tileWidthItem->valueint;
    map.tileHeight = tileHeightItem->valueint;

    printf("Map Width: %d\n", map.mapWidth);
    printf("Map Height: %d\n", map.mapHeight);
    printf("Tile Width: %d\n", map.tileWidth);
    printf("Tile Height: %d\n", map.tileHeight);

    parseObjectGroups(&map);

    // 타일 데이터 파싱
    unsigned int *tileData = parseTileData(&map);
    if(tileData == NULL){
        printf("Error parsing tile data\n");
        return -1;
    }

    // 타일 데이터를 출력 (디버깅용)
    for(int i = 0; i < map.mapWidth * map.mapHeight; i++){
        printf("Tile %d: %u\n", i, tileData[i]);
    }

    SDL_Event event;
    SDL_bool running = SDL_TRUE;
    const Uint8* state = SDL_GetKeyboardState(NULL);

    fpsStartTime = SDL_GetTicks(); // FPS 확인용

    // 디버깅용
    Uint32 debugLastTime = 0;  // 처음에는 0으로 초기화
    Uint32 currentTime = SDL_GetTicks();  // 현재 시간 가져오기

    while(running){
        while(SDL_PollEvent(&event)){
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
        render(renderer, &map);
        updateFPS();

        // FPS 제한 (120)
        Uint32 frameTicks = SDL_GetTicks() - currentTime;
        if(frameTicks < 8){
            SDL_Delay(8 - frameTicks);
        }

        // 디버깅용
        currentTime = SDL_GetTicks();  // 현재 시간 업데이트
        if (currentTime - debugLastTime > 1000) {  // 1000ms (1초) 이상 차이 나면
            printf("playerX / Y: %.3f / %.3f  |   camera.x: %.3f   |   FPS: %.2f\n", playerX, playerY, cameraX, fps);
            printf("playerRect.x / y / w: %d / %d / %d  |  platformCount: %d\n", playerRect.x, playerRect.y, playerRect.w, platformCount);
            printf("interaction1.x / y: x=%.2f, y=%.2f\n", 
            interactions[interactionCount - 2].x - camera.x, interactions[interactionCount - 2].y - camera.y);
            printf("interaction2.x / y: x=%.2f, y=%.2f\n\n", 
            interactions[interactionCount - 1].x - camera.x, interactions[interactionCount - 1].y - camera.y);
            debugLastTime = currentTime;  // 마지막 시간 업데이트
        }
    }

    SDL_DestroyTexture(spriteSheet);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    cJSON_Delete(map.mapJson);
    free(jsonData);
    if(tileData != NULL){
        free(tileData);
    }
    IMG_Quit();
    SDL_Quit();
    return 0;
}