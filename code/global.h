#ifndef GLOBALS_H
#define GLOBALS_H

#include <SDL.h>

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


typedef struct { // tileData.c 와 연결됨
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

typedef struct { // tileData.c 와 연결됨
    float x, y, width, height;
    float xOffset, yOffset;
} Platform;

extern Platform platforms[100];
extern int platformCount;

typedef struct { // tileData.c 와 연결됨
    float x, y, width, height;
    char name[32];
} Interaction;

extern Interaction interactions[100];
extern int interactionCount;

extern float cameraX;
extern SDL_Rect camera;

extern SDL_Texture* spriteSheet;

extern int mapWidth;
extern int mapHeight;
extern int tileWidth;
extern int tileHeight;
extern int *tileData;

extern SDL_Window *window;
extern SDL_Renderer *renderer;
extern SDL_Texture *tilesetTexture;

void addPlatform(SDL_Rect platform);
void addInteraction(SDL_Rect interactionZone, const char* name);

#endif // GLOBALS_H