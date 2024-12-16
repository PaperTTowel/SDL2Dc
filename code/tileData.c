#include "global.h"
#include <cJSON.h>
#include <stdio.h>

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
unsigned int *parseTileData(Map *map){
    map->layers = cJSON_GetObjectItem(map->mapJson, "layers");
    if(!cJSON_IsArray(map->layers)){
        printf("Error: No layers in map\n");
        return NULL;
    }

    cJSON *tileLayer = NULL;
    // 첫 번째 타일 레이어 검색
    for(int i = 0; i < cJSON_GetArraySize(map->layers); i++){
        cJSON *layer = cJSON_GetArrayItem(map->layers, i);
        cJSON *layerType = cJSON_GetObjectItem(layer, "type");
        if(cJSON_IsString(layerType) && strcmp(layerType->valuestring, "tilelayer") == 0){
            tileLayer = layer;
            break;
        }
    }

    if(tileLayer == NULL){
        printf("Error: No tile layer found\n");
        return NULL;
    }

    // Base64 인코딩된 타일 데이터 추출
    cJSON *dataItem = cJSON_GetObjectItem(tileLayer, "data");
    if(!cJSON_IsString(dataItem)){
        printf("Error: Tile layer data is not a string\n");
        return NULL;
    }

    const char *encodedData = dataItem->valuestring;
    size_t decodedLength;
    unsigned char *decodedData = base64_decode(encodedData, strlen(encodedData), &decodedLength);
    if(decodedData == NULL){
        printf("Error decoding base64 tile data\n");
        return NULL;
    }

    // 타일 ID로 변환
    size_t numTiles = decodedLength / 4;  // 각 타일이 4바이트
    map->tileData = (unsigned int *)malloc(numTiles * sizeof(unsigned int));
    if(map->tileData == NULL){
        free(decodedData);
        printf("Error allocating memory for tile data\n");
        return NULL;
    }

    // 타일 데이터 복사
    for(size_t i = 0; i < numTiles; i++){
        map->tileData[i] = ((unsigned int *)decodedData)[i];
    }

    free(decodedData);  // 디코딩 데이터 메모리 해제
    return map->tileData;  // map->tileData 반환
}

void parseObjectGroup(Map *map, cJSON *objectGroup, int xOffset, int yOffset){
    cJSON *objects = cJSON_GetObjectItem(objectGroup, "objects");
    if(!cJSON_IsArray(objects)){
        printf("Error: No objects in object group\n");
        return;
    }

    // 오브젝트 데이터를 순회하며 처리
    for(int j = 0; j < cJSON_GetArraySize(objects); j++){
        cJSON *object = cJSON_GetArrayItem(objects, j);
        cJSON *x = cJSON_GetObjectItem(object, "x");
        cJSON *y = cJSON_GetObjectItem(object, "y");
        cJSON *width = cJSON_GetObjectItem(object, "width");
        cJSON *height = cJSON_GetObjectItem(object, "height");
        cJSON *name = cJSON_GetObjectItem(object, "name");
        cJSON *properties = cJSON_GetObjectItem(object, "properties");

        if(cJSON_IsNumber(x) && cJSON_IsNumber(y) && cJSON_IsNumber(width) && cJSON_IsNumber(height)){
            float objectX = x->valuedouble + xOffset;
            float objectY = y->valuedouble + yOffset;
            printf("Object %s - x: %.3f, y: %.3f, width: %.3f, height: %.3f\n",
                   name ? name->valuestring : "Unnamed",
                   x->valuedouble, y->valuedouble, width->valuedouble, height->valuedouble);

            if(cJSON_IsArray(properties)){
                for(int k = 0; k < cJSON_GetArraySize(properties); k++){
                    cJSON *property = cJSON_GetArrayItem(properties, k);
                    cJSON *propName = cJSON_GetObjectItem(property, "name");
                    cJSON *propValue = cJSON_GetObjectItem(property, "value");

                    if(strcmp(propName->valuestring, "Text") == 0){
                        // interaction 객체를 찾기 전에 해당 인덱스를 확인합니다.
                        // interactions 배열에서 상호작용에 맞는 객체를 찾아서 텍스트 할당
                        if(interactions[interactionCount].propertyText != NULL){
                            free(interactions[interactionCount].propertyText);  // 기존 메모리 해제
                        }

                        // 특정 interaction 객체에 텍스트를 저장합니다.
                        interactions[interactionCount].propertyText = strdup(propValue->valuestring);
                        
                        printf("Loaded text: %s\n", interactions[interactionCount].propertyText);
                    }
                    else if(propName && cJSON_IsString(propName) && propValue && cJSON_IsNumber(propValue)){
                        if(strcmp(propName->valuestring, "eventID") != 0){
                            // Shop items 배열에 구매 제한 속성 저장
                            strncpy(items[itemCount].name, propName->valuestring, sizeof(items[itemCount].name) - 1);
                            items[itemCount].name[sizeof(items[itemCount].name) - 1] = '\0';  // Null-terminate
                            items[itemCount].value = propValue->valueint;
                            items[itemCount].stock = propValue->valueint;

                            printf("Loaded property: %s = %d\n", items[itemCount].name, items[itemCount].value);
                            printf("item stock has been saved: %s = %d\n", items[itemCount].name, items[itemCount].stock);
                            itemCount++;
                        }
                        else if(strcmp(propName->valuestring, "eventID") == 0){
                            // 특정 interaction 객체에 eventID를 저장
                            interactions[interactionCount].eventID = propValue->valueint;
                            printf("Loaded eventID: %d for interaction: %s\n", interactions[interactionCount].eventID, interactions[interactionCount].name);
                        }
                    }
                }
            }

            if(name != NULL){
                SDL_Rect newInteraction = { objectX, objectY, width->valuedouble, height->valuedouble };
                if(strcmp(name->valuestring, "floor") == 0 || strcmp(name->valuestring, "wall") == 0){
                    addPlatform(newInteraction);
                }
                else if(strcmp(name->valuestring, "roofDoor") == 0 || strcmp(name->valuestring, "blockedDoor") == 0 || 
                        strcmp(name->valuestring, "elevator") == 0 || strcmp(name->valuestring, "1F-3F") == 0 || 
                        strcmp(name->valuestring, "3F-4F") == 0 ||
                        strcmp(name->valuestring, "4F-roofF") == 0 || strcmp(name->valuestring, "1F-outDoor") == 0 ||
                        strcmp(name->valuestring, "otherWay") == 0 || strcmp(name->valuestring, "NotElevator") == 0 ||
                        strcmp(name->valuestring, "wrongWay") == 0 || strcmp(name->valuestring, "frontDoor") == 0 ||
                        strcmp(name->valuestring, "pyeonUijeom") == 0 || strcmp(name->valuestring, "buy") == 0 || strcmp(name->valuestring, "jinYeoldae") == 0 ||
                        strcmp(name->valuestring, "frige") == 0 || strcmp(name->valuestring, "bed") == 0){
                    addInteraction(newInteraction, name->valuestring);
                }
            }
        }
    }
}

// JSON에서 objectgroup 파싱
void parseObjectGroups(Map *map, int xOffset, int yOffset){
    cJSON *layers = cJSON_GetObjectItem(map->mapJson, "layers");
    if(!cJSON_IsArray(layers)){
        printf("Error: No layers in map\n");
        return;
    }

    // objectgroup 레이어 찾기
    for(int i = 0; i < cJSON_GetArraySize(layers); i++){
        cJSON *layer = cJSON_GetArrayItem(layers, i);
        cJSON *layerType = cJSON_GetObjectItem(layer, "type");
        if(cJSON_IsString(layerType) && strcmp(layerType->valuestring, "objectgroup") == 0){
            printf("Parsing objectgroup layer: %s\n", cJSON_GetObjectItem(layer, "name")->valuestring);
            parseObjectGroup(map, layer, xOffset, yOffset);
        }
    }
}

void addPlatform(SDL_Rect platform){
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

/* 이 함수는 머리가 아픈 이슈로 유기
void addPolygonPlatform(SDL_Point* points, int pointCount, SDL_Rect baseRect) {
    if(platformCount < 100){
        int minX = points[0].x, maxX = points[0].x;
        int minY = points[0].y, maxY = points[0].y;

        // 다각형의 최소/최대 x, y 계산
        for(int i = 1; i < pointCount; i++){
            if (points[i].x < minX) minX = points[i].x;
            if (points[i].x > maxX) maxX = points[i].x;
            if (points[i].y < minY) minY = points[i].y;
            if (points[i].y > maxY) maxY = points[i].y;
        }

        // 기준 rect의 x, y 오프셋을 추가하여 실제 위치 계산
        platforms[platformCount].x = (baseRect.x) * 3;  // 3배 확대
        platforms[platformCount].y = (baseRect.y + minY) * 3;
        platforms[platformCount].width = (maxX - minX) * 3;  // 폭 계산
        platforms[platformCount].height = (maxY - minY) * 3;  // 높이 계산
        platformCount++;

        // 디버그 출력
        printf("Added polygon platform: x=%.2f, y=%.2f, width=%.2f, height=%.2f\n",
               platforms[platformCount - 1].x, platforms[platformCount - 1].y,
               platforms[platformCount - 1].width, platforms[platformCount - 1].height);
    }
    else{
        printf("Maximum platform limit reached.\n");
    }
}
*/

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

int loadMapsFromDirectory(const char* directory, Map* maps, int maxMaps){
    DIR *dir;
    struct dirent *entry;
    int mapCount = 0;

    // 디렉토리 열기
    if((dir = opendir(directory)) == NULL){
        perror("opendir() error");
        return -1;
    }

    while((entry = readdir(dir)) != NULL && mapCount < maxMaps){
        // .json 파일만 처리
        if(strstr(entry->d_name, ".json") != NULL){
            char filePath[256];
            snprintf(filePath, sizeof(filePath), "%s/%s", directory, entry->d_name);

            // JSON 파일 읽기
            char *jsonData = readFile(filePath);
            if(jsonData == NULL){
                printf("Error reading JSON file: %s\n", filePath);
                continue;
            }

            // JSON 데이터 파싱
            maps[mapCount].mapJson = cJSON_Parse(jsonData);
            free(jsonData); // jsonData 메모리 해제
            if(maps[mapCount].mapJson == NULL){
                printf("Error parsing JSON file: %s\n", filePath);
                continue;
            }

            mapCount++; // 성공적으로 맵이 로드되면 증가
        }
    }
    closedir(dir);
    return mapCount; // 불러온 맵의 개수 반환
}

// 각 프레임 이미지를 텍스처로 불러오는 함수
int loadAnimationFrames(int eventID, SDL_Texture ***frames, SDL_Renderer *renderer){
    // 파일 경로 구성 (예: \resource\eventID\1.png)
    char filePath[256];
    snprintf(filePath, sizeof(filePath), "resource/eventID/%d.png", eventID);

    // 스프라이트 시트 불러오기
    SDL_Surface *spriteSheet = IMG_Load(filePath);
    if (!spriteSheet) {
        fprintf(stderr, "Failed to load sprite sheet: %s\n", IMG_GetError());
        return 0;
    }

    // 프레임 정보 설정
    const int frameWidth = 24;   // 각 프레임의 가로 크기
    const int frameHeight = 24;  // 각 프레임의 세로 크기
    const int totalWidth = spriteSheet->w; // 스프라이트 시트의 전체 가로 길이
    const int frameCount = totalWidth / frameWidth; // 총 프레임 개수 계산

    // 텍스처 배열 동적 할당
    *frames = malloc(sizeof(SDL_Texture *) * frameCount);
    if (!*frames) {
        fprintf(stderr, "Failed to allocate memory for frames.\n");
        SDL_FreeSurface(spriteSheet);
        return 0;
    }

    // 각 프레임을 텍스처로 변환
    SDL_Rect srcRect = { 0, 0, frameWidth, frameHeight }; // 잘라낼 영역
    for (int i = 0; i < frameCount; i++) {
        srcRect.x = i * frameWidth; // 현재 프레임의 x 좌표 설정

        // 프레임 추출
        SDL_Surface *frameSurface = SDL_CreateRGBSurface(0, frameWidth, frameHeight, 32, 
                                                          0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
        SDL_BlitSurface(spriteSheet, &srcRect, frameSurface, NULL); // 스프라이트 시트에서 프레임 추출

        // SDL_Texture로 변환
        (*frames)[i] = SDL_CreateTextureFromSurface(renderer, frameSurface);
        SDL_FreeSurface(frameSurface);

        if (!(*frames)[i]) {
            fprintf(stderr, "Failed to create texture for frame %d: %s\n", i, SDL_GetError());
            for (int j = 0; j < i; j++) { // 이전에 할당된 텍스처 해제
                SDL_DestroyTexture((*frames)[j]);
            }
            free(*frames);
            SDL_FreeSurface(spriteSheet);
            return 0;
        }
        printf("Frame %d loaded successfully.\n", i); // 디버깅용 메시지
    }

    SDL_FreeSurface(spriteSheet); // 원본 스프라이트 시트 해제
    return frameCount; // 총 프레임 수 반환
}

// NPC 대화 데이터를 불러오는 함수
void loadNPCDialogue(const char *fileName){
    // 파일 경로 만들기
    char filePath[256];
    snprintf(filePath, sizeof(filePath), "resource/eventID/%s.json", fileName);

    // 파일 읽기
    FILE *file = fopen(filePath, "r");
    if(file == NULL){
        printf("Failed to open dialogue file %s\n", filePath);
        return;
    }

    // JSON 파싱
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    rewind(file);

    char *jsonData = (char *)malloc(fileSize + 1);
    fread(jsonData, 1, fileSize, file);
    jsonData[fileSize] = '\0';

    // cJSON 라이브러리로 JSON 파싱
    cJSON *root = cJSON_Parse(jsonData);
    if(root == NULL){
        printf("Failed to parse JSON\n");
        free(jsonData);
        fclose(file);
        return;
    }

    // 필요한 데이터 가져오기
    cJSON *dialogues = cJSON_GetObjectItem(root, "dialogues");
    if(dialogues == NULL){
        printf("No dialogues found in JSON\n");
        cJSON_Delete(root);
        free(jsonData);
        fclose(file);
        return;
    }

    // 대화 배열 탐색
    int dialogueCount = cJSON_GetArraySize(dialogues);
    for(int i = 0; i < dialogueCount; i++){
        cJSON *dialogue = cJSON_GetArrayItem(dialogues, i);
        if (dialogue == NULL) continue;

        printf("NPC: %s\n", cJSON_GetObjectItem(dialogue, "name")->valuestring);
        // 대화 텍스트 출력
        cJSON *text = cJSON_GetObjectItem(dialogue, "text");
        if(cJSON_IsArray(text)){
            // 다중 줄 텍스트 처리
            int lineCount = cJSON_GetArraySize(text);
            for(int j = 0; j < lineCount && j < 4; j++){ // 최대 4줄 대사만 처리
                strncpy(currentDialogue.text[j], cJSON_GetArrayItem(text, j)->valuestring, sizeof(currentDialogue.text[j]) - 1); 
                currentDialogue.text[j][sizeof(currentDialogue.text[j]) - 1] = '\0'; // null-terminate
                printf("Dialogue (multi-line)%d: %s\n", j + 1, currentDialogue.text[j]);
            }
        }
        else{
            // 단일 텍스트 처리
            strncpy(currentDialogue.text[0], text->valuestring, sizeof(currentDialogue.text[0]) - 1);
            currentDialogue.text[0][sizeof(currentDialogue.text[0]) - 1] = '\0';
            printf("Dialogue: %s\n", currentDialogue.text[0]);
        }

        // 선택지 출력
        cJSON *options = cJSON_GetObjectItem(dialogue, "options");
        int optionCount = cJSON_GetArraySize(options);
        currentDialogue.optionCount = optionCount;
        for(int j = 0; j < optionCount && j < 4; j++){
            cJSON *option = cJSON_GetArrayItem(options, j);
            strncpy(currentDialogue.options[j], cJSON_GetObjectItem(option, "text")->valuestring, sizeof(currentDialogue.options[j]) - 1);
            currentDialogue.options[j][sizeof(currentDialogue.options[j]) - 1] = '\0'; // null-terminate
            currentDialogue.nextIds[j] = cJSON_GetObjectItem(option, "nextId")->valueint;
            printf("Option %d: %s (nextId: %d)\n", j + 1, currentDialogue.options[j], currentDialogue.nextIds[j]);
        }

        // 대화 ID 출력
        cJSON *nextId = cJSON_GetObjectItem(dialogue, "nextId");
        if(nextId && !cJSON_IsNull(nextId)){
            currentDialogue.nextIds[0] = nextId->valueint; // 첫 번째 선택지에만 적용
            printf("Next Dialogue ID: %d\n", currentDialogue.nextIds[0]);
        }
        else{
            currentDialogue.nextIds[0] = -1; // 대화 종료
            printf("No Next Dialogue ID (end of conversation).\n");
        }

        // 대화의 끝 구분선
        printf("-------------------------------\n");
    }

    // 파일과 메모리 해제
    free(jsonData);
    fclose(file);
    cJSON_Delete(root);
}