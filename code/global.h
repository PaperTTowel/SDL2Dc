#ifndef GLOBALS_H
#define GLOBALS_H

#include <SDL.h>
#include <SDL_ttf.h>

extern float playerX;
extern float playerY;
extern SDL_Rect playerRect;

extern int velocityY;
extern int gravity;
extern int direction;
extern int isJumping;
extern int isMoving;

extern int frame;
extern int idleFrameDelay;
extern int movingFrameDelay;
extern int lastFrameTime;
extern int eKeyPressed;
extern char *propertyText;

typedef struct Map{ // tileData.c 와 연결됨
    int mapWidth;
    int mapHeight;
    int tileWidth;
    int tileHeight;
    unsigned int *tileData;
    cJSON *mapJson;
    cJSON *layers;
} Map;

extern Map maps[100];
extern int currentMapCount;

typedef struct Platform{ // tileData.c 와 연결됨
    float x, y, width, height;
    SDL_Point polygon[100];  // 다각형 플랫폼의 점들
    int pointCount;
} Platform;

extern Platform platforms[100];
extern int platformCount;

typedef struct Interaction{ // tileData.c 와 연결됨
    float x, y, width, height;
    char name[32];
    char *propertyText;
} Interaction;

extern Interaction interactions[100];
extern int interactionCount;

// 각 상호작용별로 마지막 상호작용 위치를 저장
typedef struct LastInteraction{
    float x;
    float y;
    char name[100];
} LastInteraction;

LastInteraction lastInteractions[100] = {0};  // 최대 상호작용 개수만큼 배열 할당

typedef struct TextDisplay{
    const char *text;  // 출력할 텍스트
    Uint32 startTime;  // 텍스트가 표시된 시작 시간
    Uint32 duration;   // 텍스트 표시 지속 시간
} TextDisplay;

TextDisplay activeTextDisplay = {NULL, 0, 0};

extern float cameraX;
extern SDL_Rect camera;

extern SDL_Texture* spriteSheet;

extern SDL_Window *window;
extern SDL_Renderer *renderer;
extern SDL_Texture *tilesetTexture;

void addPlatform(SDL_Rect platform);
void addInteraction(SDL_Rect interactionZone, const char* name);
void checkInteractions(SDL_Rect *playerRect);

#endif // GLOBALS_H