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

                if (name != NULL && strcmp(name->valuestring, "roofExit") == 0) {
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