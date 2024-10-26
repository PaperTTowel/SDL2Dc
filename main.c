#include <cJSON.h>
#include <SDL.h>
#include <SDL_image.h>
#include <stdio.h>
#include <stdlib.h>

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
static int eKeyPressed = 0;

typedef struct {
    float x, y, width, height;
} Platform;
Platform platforms[100]; // 플랫폼 배열
int platformCount = 0; // 현재 플랫폼 수

typedef struct {
    float x, y, width, height;
    char name[32];
} Interaction;

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

void addPlatform(SDL_Rect platform) {
    // 최대 플랫폼 수를 초과하지 않도록 체크
    if (platformCount < 100) {
        platforms[platformCount].x = platform.x * 3;
        platforms[platformCount].y = platform.y * 3;
        platforms[platformCount].width = platform.w * 3;  // 너비를 3배로 증가
        platforms[platformCount].height = platform.h * 3; // 높이를 3배로 증가
        platformCount++; // 플랫폼 수 증가
        printf("Added platform: x=%.2f, y=%.2f, width=%.2f, height=%.2f\n", 
            platforms[platformCount - 1].x, platforms[platformCount - 1].y, 
            platforms[platformCount - 1].width, platforms[platformCount - 1].height);
    } else {
        printf("Maximum platform limit reached.\n");
    }
}

void addInteraction(SDL_Rect interactionZone, const char* name){
    // 최대 상호작용 수를 초과하지 않도록 체크
    if (interactionCount < 100) {
        interactions[interactionCount].x = interactionZone.x * 3;
        interactions[interactionCount].y = interactionZone.y * 3;
        interactions[interactionCount].width = interactionZone.w * 3;  // 너비를 3배로 증가
        interactions[interactionCount].height = interactionZone.h * 3; // 높이를 3배로 증가
        
        strncpy(interactions[interactionCount].name, name, sizeof(interactions[interactionCount].name) - 1);
        interactions[interactionCount].name[sizeof(interactions[interactionCount].name) - 1] = '\0'; // 안전하게 문자열 종료// 안전하게 문자열 종료
        interactionCount++; // 상호작용 수 증가
        printf("Added interaction: x=%.2f, y=%.2f, width=%.2f, height=%.2f\n", 
           interactions[interactionCount - 1].x, interactions[interactionCount - 1].y, 
           interactions[interactionCount - 1].width, interactions[interactionCount - 1].height);
    } else {
        printf("Maximum interaction limit reached.\n");
    }
}

void handleInteraction(SDL_Rect *playerRect, Interaction *interactionZone) {
    // 상호작용 이벤트 처리 (예: 문 열기, 대화 시작 등)
    printf("event! (name: %s, x: %.2f, y: %.2f)\n",interactionZone->name, interactionZone->x, interactionZone->y);
    // 추가적으로 상호작용 오브젝트에 따라 다른 동작을 처리할 수 있습니다.
}

void checkInteractions(SDL_Rect *playerRect) {
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

void updateCamera(float deltaTime) {
    const float cameraSpeed = 5.0f; // 부드러운 카메라 속도 (픽셀/초)

    // 카메라의 x, y 좌표를 플레이어의 x 좌표를 기준으로 부드럽게 조정
    cameraX += (playerX - cameraX - (camera.w / 2 - playerRect.w / 2)) * cameraSpeed * deltaTime;
    //camera.y = (int)(playerY - (camera.h / 2 - playerRect.h / 2));

    // 카메라가 화면의 경계를 넘지 않도록 제한
    if (cameraX < 0) cameraX = 0;
    /*
    if (cameraX > (mapWidth * tileWidth * 3) - camera.w) {
        cameraX = (mapWidth * tileWidth * 3) - camera.w;
    }
    if (camera.y < 0) camera.y = 0;
    if (camera.y > (mapHeight * tileHeight * 3) - camera.h) {
        camera.y = (mapHeight * tileHeight * 3) - camera.h;
    }
    */

    camera.x = (int)cameraX; // 카메라 rect의 x 값은 int로 변환 (분리용)
}

unsigned char *base64_decode(const char *input, size_t len, size_t *out_len){
    static const unsigned char base64_table[65] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    
    unsigned char dtable[256], *out, *pos, block[4], tmp;
    size_t i, count, olen;
    int pad = 0;
    
    memset(dtable, 0x80, 256);
    for(i = 0; i < 64; i++)
        dtable[base64_table[i]] = (unsigned char) i;
    dtable['='] = 0;
    
    count = 0;
    for(i = 0; i < len; i++){
        if (dtable[(unsigned char)input[i]] != 0x80)
            count++;
    }
    
    if(count == 0 || count % 4)
        return NULL;
    
    olen = count / 4 * 3;
    pos = out = (unsigned char *)malloc(olen);
    if (out == NULL)
        return NULL;
    
    count = 0;
    for(i = 0; i < len; i++){
        tmp = dtable[(unsigned char)input[i]];
        if (tmp == 0x80)
            continue;
        
        block[count] = tmp;
        count++;
        
        if(count == 4){
            *pos++ = (block[0] << 2) | (block[1] >> 4);
            *pos++ = (block[1] << 4) | (block[2] >> 2);
            *pos++ = (block[2] << 6) | block[3];
            count = 0;
        }
    }

    
    
    if(count){
        *pos++ = (block[0] << 2) | (block[1] >> 4);
        if (count > 2)
            *pos++ = (block[1] << 4) | (block[2] >> 2);
        if (count > 3)
            *pos++ = (block[2] << 6) | block[3];
    }
    
    *out_len = pos - out;
    return out;
}

void parseTileLayer(cJSON *layer){
    cJSON *dataItem = cJSON_GetObjectItem(layer, "data");
    const char *encodedData = dataItem->valuestring;  // base64로 인코딩된 문자열

    // base64 디코딩
    size_t decodedLength;
    unsigned char *decodedData = base64_decode(encodedData, strlen(encodedData), &decodedLength);

    if(decodedData == NULL){
        printf("Error decoding base64 tile data\n");
        return;
    }

    // 타일 데이터를 처리하는 코드
    for(size_t i = 0; i < decodedLength / 4; i++){  // 타일 데이터는 4바이트씩 처리됩니다.
        unsigned int tileID = ((unsigned int*)decodedData)[i];  // 타일 ID 가져오기
        printf("Tile %zu: %u\n", i, tileID);
    }

    free(decodedData);  // 디코딩된 데이터를 메모리에서 해제
}

// JSON에서 맵 데이터를 파싱하는 함수
// Tile data를 디코딩하고 배열로 반환하는 함수
unsigned int *parseTileData(cJSON *map){
    cJSON *layers = cJSON_GetObjectItem(map, "layers");
    if(!cJSON_IsArray(layers)){
        printf("Error: No layers in map\n");
        return NULL;
    }

    cJSON *tileLayer = NULL;
    // 타일 레이어를 찾아서 처리 (첫 번째 레이어가 타일 레이어라고 가정)
    for(int i = 0; i < cJSON_GetArraySize(layers); i++){
        cJSON *layer = cJSON_GetArrayItem(layers, i);
        cJSON *layerType = cJSON_GetObjectItem(layer, "type");
        if (cJSON_IsString(layerType) && strcmp(layerType->valuestring, "tilelayer") == 0) {
            tileLayer = layer;
            break;
        }
    }

    if(tileLayer == NULL){
        printf("Error: No tile layer found\n");
        return NULL;
    }

    // 타일 데이터 추출 (base64로 인코딩된 데이터)
    cJSON *dataItem = cJSON_GetObjectItem(tileLayer, "data");
    const char *encodedData = dataItem->valuestring;

    // Base64 디코딩
    size_t decodedLength;
    unsigned char *decodedData = base64_decode(encodedData, strlen(encodedData), &decodedLength);
    if(decodedData == NULL){
        printf("Error decoding base64 tile data\n");
        return NULL;
    }

    // 디코딩된 데이터를 타일 ID 배열로 변환
    size_t numTiles = decodedLength / 4;  // 각 타일이 4바이트이므로
    tileData = (unsigned int *)malloc(numTiles * sizeof(unsigned int));
    if(tileData == NULL){
        free(decodedData);
        printf("Error allocating memory for tile data\n");
        return NULL;
    }

    // 디코딩된 데이터를 타일 데이터로 복사
    for(size_t i = 0; i < numTiles; i++){
        tileData[i] = ((unsigned int*)decodedData)[i];  // 4바이트씩 읽어서 타일 ID로 변환
    }

    free(decodedData);  // 메모리 해제
    return tileData;     // 타일 데이터 반환
}

void parseObjectGroup(cJSON *map){
    cJSON *layers = cJSON_GetObjectItem(map, "layers");
    if(!cJSON_IsArray(layers)){
        printf("Error: No layers in map\n");
        return;
    }

    cJSON *objectGroup = NULL;

    // 레이어를 순회하며 objectgroup을 찾음
    for(int i = 0; i < cJSON_GetArraySize(layers); i++){
        cJSON *layer = cJSON_GetArrayItem(layers, i);
        cJSON *layerType = cJSON_GetObjectItem(layer, "type");

        if(cJSON_IsString(layerType)){
            // 디버깅용
            printf("Layer %d: type = %s\n", i, layerType->valuestring);

            // "type"이 "objectgroup"인 레이어를 찾음
            if(strcmp(layerType->valuestring, "objectgroup") == 0){
                objectGroup = layer;
                break;
            }
        }
    }

    if(objectGroup == NULL){
        printf("Error: No object group found\n");
        return;
    }

    // objectgroup 데이터를 처리하는 코드
    cJSON *objects = cJSON_GetObjectItem(objectGroup, "objects");
    if(!cJSON_IsArray(objects)){
        printf("Error: No objects in object group\n");
        return;
    }

    // 오브젝트 데이터를 순회하며 처리
    // 오브젝트 그룹을 순회하며 각 그룹의 오브젝트를 처리
    for (int i = 0; i < cJSON_GetArraySize(layers); i++) {
        cJSON *layer = cJSON_GetArrayItem(layers, i);
        cJSON *objects = cJSON_GetObjectItem(layer, "objects");

        // 오브젝트가 있는 경우에만 처리
        if (cJSON_IsArray(objects)) {
            for (int j = 0; j < cJSON_GetArraySize(objects); j++) {
                cJSON *object = cJSON_GetArrayItem(objects, j);
                cJSON *x = cJSON_GetObjectItem(object, "x");
                cJSON *y = cJSON_GetObjectItem(object, "y");
                cJSON *width = cJSON_GetObjectItem(object, "width");
                cJSON *height = cJSON_GetObjectItem(object, "height");
                cJSON *name = cJSON_GetObjectItem(object, "name");

                if (cJSON_IsNumber(x) && cJSON_IsNumber(y) && cJSON_IsNumber(width) && cJSON_IsNumber(height)) {
                    printf("Object %s - x: %.3f, y: %.3f, width: %.3f, height: %.3f\n", name->valuestring, x->valuedouble, y->valuedouble, width->valuedouble, height->valuedouble);
                }

                if (name != NULL && strcmp(name->valuestring, "floor") == 0) {
                    SDL_Rect platform = {
                        x->valuedouble,
                        y->valuedouble,
                        width->valuedouble,
                        height->valuedouble
                    };
                    addPlatform(platform);
                }

                if (name != NULL && strcmp(name->valuestring, "wall") == 0) {
                    SDL_Rect platform = {
                        x->valuedouble,
                        y->valuedouble,
                        width->valuedouble,
                        height->valuedouble
                    };
                    addPlatform(platform);
                }

                if (name != NULL && strcmp(name->valuestring, "smallDoor") == 0) {
                    SDL_Rect newInteraction = {
                        x->valuedouble,
                        y->valuedouble,
                        width->valuedouble,
                        height->valuedouble
                    };
                    addInteraction(newInteraction, name->valuestring);
                }

                if (name != NULL && strcmp(name->valuestring, "normalDoor") == 0) {
                    SDL_Rect newInteraction = {
                        x->valuedouble,
                        y->valuedouble,
                        width->valuedouble,
                        height->valuedouble
                    };
                    addInteraction(newInteraction, name->valuestring);
                }
            }
        }
    }
}

// JSON에서 objectgroup 파싱
void parseObjectGroups(cJSON *map){
    cJSON *layers = cJSON_GetObjectItem(map, "layers");
    if(!cJSON_IsArray(layers)){
        printf("Error: No layers in map\n");
        return;
    }

    // objectgroup 레이어 찾기
    for(int i = 0; i < cJSON_GetArraySize(layers); i++){
        cJSON *layer = cJSON_GetArrayItem(layers, i);
        cJSON *layerType = cJSON_GetObjectItem(layer, "type");
        if (cJSON_IsString(layerType) && strcmp(layerType->valuestring, "objectgroup") == 0) {
            printf("Parsing objectgroup layer: %s\n", cJSON_GetObjectItem(layer, "name")->valuestring);
            parseObjectGroup(layer);
        }
    }
}

// 타일을 렌더링하는 함수
void renderTileMap(SDL_Renderer* renderer){
    int tilesPerRow = 120 / tileWidth;

    for (int y = 0; y < mapHeight; y++){
        for (int x = 0; x < mapWidth; x++){
            int tileIndex = tileData[y * mapWidth + x] - 1;  // 타일 ID는 1부터 시작하므로 0 기반으로 조정
            if (tileIndex < 0) continue; // 비어있는 타일은 건너뛰기

            // 타일셋에서 타일 위치 계산
            int tileX = (tileIndex % tilesPerRow) * tileWidth;
            int tileY = (tileIndex / tilesPerRow) * tileHeight;

            // 타일셋에서 해당 타일을 클리핑
            SDL_Rect srcRect = { tileX, tileY, tileWidth, tileHeight };
            SDL_Rect destRect = { x * tileWidth * 3 - camera.x, y * tileHeight * 3 - camera.y, tileWidth * 3, tileHeight * 3 }; // 타일을 그릴 위치

            // 타일 그리기
            SDL_RenderCopy(renderer, tilesetTexture, &srcRect, &destRect);
            /* 렌더링 디버깅 용도
            if (SDL_RenderCopy(renderer, tilesetTexture, &srcRect, &destRect) != 0) {
                printf("Error rendering tile at (%d, %d): %s\n", x, y, SDL_GetError());
            } else {
                printf("Rendered tile at (%d, %d) from tileset (%d, %d)\n", x, y, tileX, tileY);
            }
            */
        }
    }
}

SDL_Texture* loadTexture(const char* path, SDL_Renderer* renderer) {
    SDL_Surface* tempSurface = IMG_Load(path);
    if (tempSurface == NULL) {
        printf("Failed to load image %s! SDL Error: %s\n", path, SDL_GetError());
        return NULL;
    }
    
    SDL_Texture* newTexture = SDL_CreateTextureFromSurface(renderer, tempSurface);
    SDL_FreeSurface(tempSurface);
    
    return newTexture;
}

void render(SDL_Renderer* renderer){
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    renderTileMap(renderer);  // 타일 맵 렌더링 호출

    //SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    // SDL_RenderFillRect(renderer, &platform);

    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // 플랫폼 색상 설정 (빨간색)
    for (int i = 0; i < platformCount; i++) {
        SDL_Rect platformRect = {platforms[i].x - camera.x, platforms[i].y - camera.y, platforms[i].width, platforms[i].height};
        SDL_RenderFillRect(renderer, &platformRect); // 플랫폼 사각형 그리기
    }
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    for (int i = 0; i < platformCount; i++) {
        SDL_Rect interaction = {interactions[i].x - camera.x, interactions[i].y - camera.y, interactions[i].width, interactions[i].height};
        SDL_RenderFillRect(renderer, &interaction); // 플랫폼 사각형 그리기
    }

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
    SDL_Rect renderPlayer = { (int)playerX - camera.x, (int)playerY - camera.y, playerRect.w, playerRect.h };
    SDL_RenderCopy(renderer, spriteSheet, &srcRect, &renderPlayer);
    SDL_RenderPresent(renderer);
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

    SDL_Window* window = SDL_CreateWindow("SDL 2D Platform 게임", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    SDL_Surface* tempSurface = IMG_Load("resource\\walk and idle.png");
    printf("sprite loaded!\n");
    if(!tempSurface){
        printf("load failed: %s\n", IMG_GetError());
        return 1;
    }
    
    spriteSheet = SDL_CreateTextureFromSurface(renderer, tempSurface);
    SDL_FreeSurface(tempSurface);

    
    tilesetTexture = loadTexture("resource\\Sample.png", renderer);
    if (tilesetTexture == NULL) {
        printf("Failed to load tileset texture!\n");
        return -1;
    }

    // JSON 파일 읽기
    char *jsonData = readFile("SampleMap.json");
    if(jsonData == NULL){
        printf("Error reading JSON file\n");
        return -1;
    }

    cJSON *map = cJSON_Parse(jsonData);
    if(map == NULL){
        printf("Error parsing JSON\n");
        return -1;
    }

    // 맵의 크기와 타일 크기 추출
    cJSON *width = cJSON_GetObjectItem(map, "width");
    cJSON *height = cJSON_GetObjectItem(map, "height");
    cJSON *tileWidthItem = cJSON_GetObjectItem(map, "tilewidth");
    cJSON *tileHeightItem = cJSON_GetObjectItem(map, "tileheight");

    if(!cJSON_IsNumber(width) || !cJSON_IsNumber(height) || !cJSON_IsNumber(tileWidthItem) || !cJSON_IsNumber(tileHeightItem)){
        printf("Error in map dimensions\n");
        return -1;
    }

    mapWidth = width->valueint;
    mapHeight = height->valueint;
    tileWidth = tileWidthItem->valueint;
    tileHeight = tileHeightItem->valueint;

    printf("Map Width: %d\n", mapWidth);
    printf("Map Height: %d\n", mapHeight);
    printf("Tile Width: %d\n", tileWidth);
    printf("Tile Height: %d\n", tileHeight);

    parseObjectGroup(map);

    // 타일 데이터 파싱
    unsigned int *tileData = parseTileData(map);
    if(tileData == NULL){
        printf("Error parsing tile data\n");
        return -1;
    }

    // 타일 데이터를 출력 (디버깅용)
    //for(int i = 0; i < mapWidth * mapHeight; i++){
    //    printf("Tile %d: %u\n", i, tileData[i]);
    //}

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
        render(renderer);
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
    cJSON_Delete(map);
    free(jsonData);
    if(tileData != NULL){
        free(tileData);
    }
    IMG_Quit();
    SDL_Quit();
    return 0;
}