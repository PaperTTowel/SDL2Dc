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
    char *SE;
    char *propertyText;
    int eventID;
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

typedef struct ShopItem{
    char name[32];
    int value;     // 구매 제한
    int stock;
    int selectedItem;
} Shop;

Shop items[10];    // 최대 10개 적재 가능
int itemCount = 0; // 현재 상점의 아이템 개수

SDL_bool isShopVisible = SDL_FALSE;  // 상점 UI 상태
Shop shop = {0};                     // 상점 데이터 구조체

typedef struct Inventory{
    char name[32];  // 소지 아이템 이름
    int quantity;   // 소지 아이템 개수
} Inventory;

extern float cameraX;
extern SDL_Rect camera;

extern SDL_Texture* spriteSheet;

extern SDL_Window *window;
extern SDL_Renderer *renderer;
extern SDL_Texture *tilesetTexture;

extern int playerGold;

// 띵동대쉬 초당 16연타 이벤트 변수 (구조체로 하기 귀찮음;;)
SDL_Color BasicColor = {255, 255, 255, 255}; // 흰색 텍스트
SDL_Color YelloColor = {255, 255, 0, 255};   // 노란색 텍스트
SDL_Color textEventColor;
int fontSize = 24;
char buffer[64];
SDL_bool isMiniGameActive = SDL_FALSE;
Uint32 miniGameStartTime = 0;
int spaceBarCount = 0;
Uint32 effectStartTime = 0; // 텍스트 효과 시작 시간
SDL_bool isEffectActive = SDL_FALSE;

typedef struct tileAnimation {
    int eventID;           // 애니메이션과 관련된 eventID
    float x, y;            // 렌더링 좌표
    int frameCount;        // 총 프레임 수
    int currentFrame;      // 현재 프레임
    Uint32 frameDuration;  // 프레임 지속 시간
    Uint32 lastFrameTime;  // 마지막 프레임 갱신 시간
    SDL_Texture **frames;  // 프레임 이미지 배열
    SDL_bool isActive;     // 활성화 여부
    SDL_bool isFreezed;    // 일시정지 여부
    SDL_bool isFinished;   // 완료 여부
} tileAnimation;

// 애니메이션 배열
tileAnimation animations[10];
int animationCount = 0;

typedef struct DialogueText {
    char name[32];          // 이름
    int ID;                 // 텍스트 ID
    int currentID;          // 현재 ID
    int previousId;         // 이전 ID
    char text[4][256];      // 대화 텍스트 (최대 4줄)
    int textLineCount;      // 지정 텍스트 배열
    int optionCount;        // 선택지 개수 (각 대화마다 다를 수 있음)
    char options[4][128];   // 선택지 텍스트 (최대 4개의 선택지)
    int nextIds[4];         // 선택 후 이동할 다음 대화 ID (최대 4개)
    char SE[16];
} DialogueText;

Uint32 textTime = 0;
DialogueText dialogues[10];             // 대화 목록
SDL_bool isTextComplete = SDL_FALSE;   // 텍스트 출력 완료 여부
SDL_bool isDialogueActive = SDL_FALSE; // 텍스트 활성화 여부
int selectedOption = 0;                // 초기 선택지 인덱스
int currentLine = 0;                   // 현재 출력할 줄 인덱스

typedef struct soundEffect {
    Mix_Chunk *chunk;   // SDL_Mixer에서 사용할 사운드 데이터
    char name[32];      // 사운드 이름 (재생 시 키로 사용)
    int volume;         // 사운드 볼륨 (0~128)
} SoundEffect;

typedef struct {
    SoundEffect effects[16];  // 최대 32개의 SoundEffect 관리
    int effectCount;          // 현재 로드된 사운드 효과 개수
} SoundManager;

// SoundManager 초기화
SoundManager soundManager = { .effectCount = 0 };

SDL_bool running = SDL_TRUE;

void addPlatform(SDL_Rect platform);
char* readFile(const char* filename);
int getItemPrice(const char *itemName);
void checkInteractions(SDL_Rect *playerRect);
void freeAnimations(tileAnimation *animations, int count);
void freeAnimationFrames(SDL_Texture **frames, int frameCount);
void initializeAllDialogues(DialogueText *dialogues, int count);
void addInteraction(SDL_Rect interactionZone, const char* name);
void showErrorAndExit(const char* title, const char* errorMessage);
int loadAnimationFrames(int eventID, SDL_Texture ***frames, SDL_Renderer *renderer);
void renderText(SDL_Renderer *renderer, const char *text, int x, int y, TTF_Font *font, SDL_Color color);

#endif // GLOBALS.H