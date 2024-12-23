#include "global.h"
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h> // ucrt에 있어서 나중에 정리해야함
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
    int tilesPerRow = 240 / map->tileWidth; // Tileset00.png의 크기가 변경될 경우 이 값을 수정할것

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
            else if(flipDiagonal && !flipVertical){
                angle = -90.0f;
                flip = SDL_FLIP_HORIZONTAL;
            }
            else if(flipDiagonal && flipHorizontal){
                angle = 90.0f;
                flip = SDL_FLIP_HORIZONTAL;
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

void renderAnimation(SDL_Renderer *renderer, tileAnimation *animation){
    if(!animation->isFinished){ 
        SDL_Rect destRect = { (int)animation->x - camera.x - 15, (int)animation->y - camera.y, maps->tileWidth * 3, maps->tileHeight * 3 };
        SDL_RenderCopy(renderer, animation->frames[animation->currentFrame], NULL, &destRect);
        // printf("Rendering frame %d at position (%d, %d)\n", animation->currentFrame, destRect.x, destRect.y);
    }
}

void displayText(SDL_Renderer *renderer, TTF_Font *font, int x, int y){
    // 텍스트가 없으면 아무것도 표시하지 않음
    if (activeTextDisplay.text == NULL) return;

    Uint32 currentTime = SDL_GetTicks();
    if (currentTime - activeTextDisplay.startTime >= activeTextDisplay.duration) {
        activeTextDisplay.text = NULL;  // 시간이 지나면 텍스트 숨기기
        return;
    }

    SDL_Color color = {255, 255, 255, 255}; // 흰색 텍스트
    SDL_Surface *surface = TTF_RenderUTF8_Blended(font, activeTextDisplay.text, color);
    if (surface == NULL) {
        printf("Failed to render text surface: %s\n", TTF_GetError());
        return;
    }

    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (texture == NULL) {
        printf("Failed to create texture from surface: %s\n", SDL_GetError());
        SDL_FreeSurface(surface);
        return;
    }

    if(activeTextDisplay.text == buffer){
        // 띵동대쉬 이벤트 텍스트 출력 위치
        SDL_Rect destRect = {270, 10, surface->w, surface->h};
        SDL_RenderCopy(renderer, texture, NULL, &destRect);
    }
    else{
        SDL_Rect destRect = {x, y, surface->w, surface->h};
        SDL_RenderCopy(renderer, texture, NULL, &destRect);
    }


    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
}

void renderShop(SDL_Renderer *renderer, Shop *shop, TTF_Font *font){
    // 상점 UI 배경
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 128);  // 어두운 반투명 배경
    SDL_Rect bgRect = {100, 100, 600, 400};  // 상점 UI 크기
    SDL_RenderFillRect(renderer, &bgRect);

    // 상점 제목
    renderText(renderer, "편의점", 150, 120, font, BasicColor);  // 상점 텍스트

    // 현재 골드 표시
    char goldText[64];  // 골드 값을 문자열로 변환할 버퍼
    sprintf(goldText, "현재 돈: %d", playerGold);  // playerGold 값을 문자열로 변환
    renderText(renderer, goldText, 150, 150, font, YelloColor);  // 변환된 골드 값 표시

    // 아이템 목록 렌더링
    for(int i = 0; i < itemCount; i++){
        int yPos = 200 + i * 40;  // 각 아이템의 y 위치

        // 선택된 아이템 강조
        SDL_Color itemColor = (i == shop->selectedItem) ? YelloColor : BasicColor;
        renderText(renderer, items[i].name, 150, yPos, font, itemColor);
        char priceText[64];
        sprintf(priceText, "가격: %d", getItemPrice(items[i].name));
        renderText(renderer, priceText, 400, yPos, font, itemColor);
        char stockText[64];
        sprintf(stockText, "재고: %d", items[i].stock);
        renderText(renderer, stockText, 600, yPos, font, itemColor);
    }

    // 구매 버튼
    renderText(renderer, "구매 (Enter)", 150, 350, font, BasicColor);

    // ESC 버튼
    renderText(renderer, "상점 닫기 (ESC)", 450, 350, font, BasicColor);
}

// 텍스트 렌더링 함수
void renderText(SDL_Renderer *renderer, const char *text, int x, int y, TTF_Font *font, SDL_Color color){
    SDL_Surface *textSurface = TTF_RenderUTF8_Blended(font, text, color);
    SDL_Texture *textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_Rect textRect = {x, y, textSurface->w, textSurface->h};
    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);
}

void renderChoice(SDL_Renderer *renderer, DialogueText *dialogue, int x, int y, int *selectedOption){
    TTF_Font *choiceFont = TTF_OpenFont("resource\\NanumGothic.ttf", fontSize);
    SDL_Color normalColor = {255, 255, 255};
    SDL_Color selectedColor = {255, 255, 0};

    for(int i = 0; i < dialogue->optionCount; i++){
        SDL_Color color = (i == *selectedOption) ? selectedColor : normalColor;
        renderText(renderer, dialogue->options[i], x, y + (i * 30), choiceFont, color);
    }
}

void renderTypingEffect(SDL_Renderer *renderer, DialogueText *dialogue, int x, int y, int *selectedOption, Uint32 startTime){
    SDL_Color normalColor = {255, 255, 255};
    SDL_Color selectedColor = {255, 255, 0};
    TTF_Font *choiceFont = TTF_OpenFont("resource\\NanumGothic.ttf", fontSize);
    renderText(renderer, dialogue->name, 110, 70, choiceFont, normalColor);

    // text가 배열인지 단일 문자열인지 확인
    int lineLength = 0;

    if(dialogue->text[currentLine] != NULL){
        lineLength = strlen(dialogue->text[currentLine]);
    }
    else{ // text가 단일 문자열일 경우 길이를 계산
        lineLength = strlen(dialogue->text[0]);
    }

    // 한 글자씩 출력
    Uint32 elapsedTime = SDL_GetTicks() - startTime;
    if(elapsedTime < 25){
        elapsedTime = 25;  // 첫 번째 프레임에서는 약간의 지연을 추가
    }

    int charsToShow = elapsedTime / 25;  // 50ms마다 한 글자 출력
    if(charsToShow > lineLength){
        charsToShow = lineLength;  // 텍스트 전부 출력 완료
        isTextComplete = SDL_TRUE;
    }

    // 현재까지 출력된 모든 줄 렌더링
    for(int t = 0; t <= currentLine; t++){
        char visibleText[256];

        if(t < currentLine){ // 이미 출력 완료된 줄은 전체 텍스트 렌더링
            strncpy(visibleText, dialogue->text[t], sizeof(visibleText) - 1);
            visibleText[sizeof(visibleText) - 1] = '\0';
        }
        else{ // 현재 줄은 타이핑 효과 적용
            strncpy(visibleText, dialogue->text[currentLine], charsToShow);
            visibleText[charsToShow] = '\0';
        }

        renderText(renderer, visibleText, x, y + (t * 30), choiceFont, normalColor);  // i * 30: 줄 간격
    }

    // 텍스트 제한 & 줄 넘기기
    if(isTextComplete && charsToShow == lineLength){
        currentLine++;
        if(currentLine >= dialogue->textLineCount){
            currentLine--;
            isTextComplete = SDL_TRUE;
        }
        else{
            textTime = SDL_GetTicks();
            isTextComplete = SDL_FALSE;
        }
    }
    // 대화 텍스트가 모두 출력되었으면 선택지를 출력
    if(isTextComplete && currentLine >= 0 && currentLine < 4){
        for(int i = 0; i < dialogue->optionCount; i++){
            isTextComplete = SDL_FALSE;
            SDL_Color color = (i == *selectedOption) ? selectedColor : normalColor;
            renderText(renderer, dialogue->options[i], x, (y + 100) + (i * 30), choiceFont, color);
        }
    }
}

// 이벤트 전용 텍스트 렌더링 함수
void renderEventText(SDL_Renderer *renderer, TTF_Font *font, const char *text, int x, int y, int fontSize, SDL_Color color) {
    TTF_Font *scaledFont = TTF_OpenFont("resource\\NanumGothic.ttf", fontSize); // 동적으로 크기 조정
    if (!scaledFont) {
        printf("Failed to load font: %s\n", TTF_GetError());
        return;
    }

    SDL_Surface *textSurface = TTF_RenderUTF8_Blended(scaledFont, text, color);
    SDL_Texture *textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_Rect textRect = {x, y, textSurface->w, textSurface->h};
    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);

    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);
    TTF_CloseFont(scaledFont);
}

void render(SDL_Renderer* renderer, Map maps[], int mapCount, const char *activeText, TTF_Font *font){
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    for(int i = 0; i < mapCount; i++){
        int xOffset = i * 2952; // 72x72 기준
        int yOffset = 0;
        renderTileMap(renderer, &maps[i], xOffset, yOffset);
    }

    /*
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
    */
    
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

    // 텍스트 렌더링 (activeText가 NULL이 아닐 경우 출력)
    if(activeText != NULL){
        displayText(renderer, font, playerX - camera.x - 12, playerY - camera.y - 24);
    }

    if(isShopVisible == SDL_TRUE){
        renderShop(renderer, &shop, font);  // 상점 UI를 렌더링
    }

    if(isMiniGameActive == SDL_TRUE){
        renderEventText(renderer, font, buffer, 270, 10, fontSize, textEventColor);
    }

    if(isDialogueActive == SDL_TRUE){
        if(textTime == 0){ // 첫 번째 대화일 때만 startTime 초기화
            textTime = SDL_GetTicks();  // 타이핑 시작 시간
            printf("startTime initialized: %u\n", textTime);  // 디버깅: startTime 값 확인
        }
        // 대화가 넘어갔을 때 textTime을 초기화
        else if(dialogues[dialogues->currentID].nextIds[0] != dialogues->previousId){
            currentLine = 0;
            textTime = SDL_GetTicks();  // 대화가 시작되거나 nextID가 바뀌면 타이핑 시작 시간 초기화
            dialogues->previousId = dialogues[dialogues->currentID].nextIds[0];  // previousNextId 갱신
            printf("startTime reinitialized: %u\n", textTime);  // 디버깅: 새로 초기화된 time 값 확인
        }
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 128);
        SDL_Rect bgRect = {100, 100, 600, 95};
        SDL_Rect nameRect = {100, 70, 200, 30};
        SDL_RenderFillRect(renderer, &bgRect);
        SDL_RenderFillRect(renderer, &nameRect);
        renderTypingEffect(renderer, &dialogues[dialogues->currentID], 110, 100, &selectedOption , textTime);
    }

    for(int i = 0; i < animationCount; i++){
        if(animations[i].isActive == SDL_TRUE){
            renderAnimation(renderer, &animations[i]);
        }
    }

    // 렌더링할 캐릭터 크기
    SDL_Rect renderPlayer = { (int)playerX - camera.x, (int)playerY - camera.y, playerRect.w, playerRect.h };
    SDL_RenderCopy(renderer, spriteSheet, &srcRect, &renderPlayer);
    SDL_RenderPresent(renderer);
}