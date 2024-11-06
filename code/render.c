#include "global.h"
#include <SDL.h>
#include <SDL_image.h>
#include <stdio.h>

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
// 타일을 렌더링하는 함수
void renderTileMap(SDL_Renderer* renderer, Map *map, int xOffset, int yOffset){
    int tilesPerRow = 120 / map->tileWidth;

    for(int y = 0; y < map->mapHeight; y++){
        for(int x = 0; x < map->mapWidth; x++){
            unsigned int tileDataValue = map->tileData[y * map->mapWidth + x];
            int tileIndex = (tileDataValue & 0x1FFFFFFF) - 1;
            if(tileIndex < 0) continue;

            int flipHorizontal = (tileDataValue & 0x80000000) != 0;
            int flipVertical = (tileDataValue & 0x40000000) != 0;
            int flipDiagonal = (tileDataValue & 0x20000000) != 0;

            int tileX = (tileIndex % tilesPerRow) * map->tileWidth;
            int tileY = (tileIndex / tilesPerRow) * map->tileHeight;

            SDL_Rect srcRect = { tileX, tileY, map->tileWidth, map->tileHeight };
            SDL_Rect destRect = { 
                x * map->tileWidth * 3 - camera.x + xOffset, 
                y * map->tileHeight * 3 - camera.y + yOffset, 
                map->tileWidth * 3, 
                map->tileHeight * 3 
            };

            SDL_RendererFlip flip = SDL_FLIP_NONE;
            double angle = 0.0;

            if(flipDiagonal && flipHorizontal && !flipVertical){ 
                angle = 90.0f;
                flip = SDL_FLIP_NONE;
            }
            else if(flipDiagonal && flipVertical && !flipHorizontal){
                angle = -90.0f;
                flip = SDL_FLIP_NONE;
            }
            else if(flipHorizontal && flipVertical){
                angle = 180.0f;
                flip = SDL_FLIP_NONE;
            }
            else if(flipDiagonal){
                angle = 90.0f;
                flip = SDL_FLIP_NONE;
            }
            else if(flipHorizontal){
                flip = SDL_FLIP_HORIZONTAL;
            }
            else if(flipVertical){
                flip = SDL_FLIP_VERTICAL;
            }

            SDL_RenderCopyEx(renderer, tilesetTexture, &srcRect, &destRect, angle, NULL, flip);
        }
    }
}

void render(SDL_Renderer* renderer, Map maps[], int mapCount){
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    for(int i = 0; i < mapCount; i++){
        int xOffset = i * 1152; // 72x72 기준
        int yOffset = 0;
        renderTileMap(renderer, &maps[i], xOffset, yOffset);
    }

    
    //디버그 용도 (충돌&상호작용 시각화)
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // 플랫폼 색상 설정 (빨간색)
    for(int i = 0; i < platformCount; i++){
        SDL_Rect platformRect = {platforms[i].x - camera.x, platforms[i].y - camera.y, platforms[i].width, platforms[i].height};
        SDL_RenderFillRect(renderer, &platformRect); // 플랫폼 사각형 그리기
    }
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    for(int i = 0; i < platformCount; i++){
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