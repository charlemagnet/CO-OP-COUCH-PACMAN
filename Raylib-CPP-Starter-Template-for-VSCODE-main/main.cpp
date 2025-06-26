#include "raylib.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#define SCREEN_SIZE 1024
#define CELL_SIZE 32
#define MAP_COLS (SCREEN_SIZE / CELL_SIZE)
#define MAP_ROWS (SCREEN_SIZE / CELL_SIZE)
// #define MOVE_SPEED 5
#define GREEN_FEED_COUNT 2
#define BLUE_FEED_DURATION 5.0f

int MOVE_SPEED = 5;
int G_MOVE_SPEED = 5;

int lives = 3;  
int score = 0;
int highScore = 0;
int lastMinute = 5;
float remainingTime = 300.0f;
bool isTimeOver = false;
bool isBlueFeedActive = false;
float blueFeedTimer = 0.0f;
Sound gameOverSound;
Sound coinSound;
Sound death;

// Menu
bool showMenu = true;
bool soundOn = true;
Rectangle playButton = {SCREEN_SIZE / 2 - 100, 300, 220, 50};
Rectangle quitButton = {SCREEN_SIZE / 2 - 100, 400, 220, 50};
Rectangle soundButton = {SCREEN_SIZE / 2 - 100, 500, 220, 50};



void DrawMainMenu() {
    ClearBackground(BLACK);
    
    DrawText("PAC-MAN CO-OP", SCREEN_SIZE / 2 - MeasureText("PAC-MAN CO-OP", 40) / 2, 150, 40, YELLOW);

    DrawRectangleRec(playButton, DARKGRAY);
    DrawText("PLAY", playButton.x + 70, playButton.y + 10, 30, WHITE);

    DrawRectangleRec(quitButton, DARKGRAY);
    DrawText("QUIT", quitButton.x + 70, quitButton.y + 10, 30, WHITE);

    DrawRectangleRec(soundButton, DARKGRAY);
    DrawText(soundOn ? "SOUND: OPEN" : "SOUND: CLOSE", soundButton.x + 6, soundButton.y + 10, 30, WHITE);
}



void LoadHighScore() {
    FILE *file = fopen("highscore.txt", "r");
    if (file != NULL) {
        fscanf(file, "%d", &highScore);
        fclose(file);
    }
}

void SaveHighScore() {
    if (score > highScore) {
        FILE *file = fopen("highscore.txt", "w");
        if (file != NULL) {
            fprintf(file, "%d", score);
            fclose(file);
        }
    }
}

typedef struct {
    int x, y;
    bool active;
} GreenFeed;

/*
typedef struct {
    int x;
    int y;
    bool active;
} BlueFeed;
*/


// BlueFeed bluefeed = {15, 12, true};  // Feed location

GreenFeed greenFeeds[GREEN_FEED_COUNT] = {
    {16, 15, true},
    {5, 25, true}
};


typedef struct {
    int x, y;
    int targetX, targetY;
    float moveTimer;
    int speed = 5.0f;
} Player;



Player pacman = {1, 1, 1, 1, 0.0f};
Player ghost  = {30, 30, 30, 30, 0.0f};

typedef struct {
    int x, y;
    bool active;
} Dot;

typedef struct {
    int x, y;
    float speed = 5.0f;
} Ghost;



#define MAX_DOTS 1024
Dot dots[MAX_DOTS];
int dotCount = 0;

    int map[32][32] = {
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0},
        {0,1,1,1,1,0,1,1,1,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,1,0},
        {0,1,1,1,0,0,0,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,0},
        {0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,0},
        {0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,1,1,0,0,0,1,0,1,0},
        {0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,0},
        {0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,0},
        {0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,1,0},
        {0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,0},
        {0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,1,0,1,0,1,0},
        {0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,1,0,1,0,1,0},
        {0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,0,0,0,0,0,0,0,0,1,0,1,0,1,0},
        {0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,0,0,0,0,0,0,0,0,1,0,1,1,1,0},
        {0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,0,0,0,0,0,0,0,0,1,0,1,0,1,0},
        {0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,0,0,0,0,0,0,0,0,1,0,1,0,1,0},
        {0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,0,0,0,0,0,0,0,0,1,0,1,0,1,0},
        {0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,0,0,0,0,0,0,0,0,1,0,1,0,1,0},
        {0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,0},
        {0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1,0},
        {0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,0},
        {0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1,0},
        {0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,0},
        {0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1,0},
        {0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,0},
        {0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1,0},
        {0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,0},
        {0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1,0},
        {0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,0},
        {0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1,0},
        {0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
    };


    

void InitDots() {
    dotCount = 0;
    for (int y = 0; y < MAP_ROWS; y++) {
        for (int x = 0; x < MAP_COLS; x++) {
            if (map[y][x] == 1) {
                dots[dotCount].x = x;
                dots[dotCount].y = y;
                dots[dotCount].active = true;
                dotCount++;
            }
        }
    }
}

void ResetGame() {
    pacman.speed = 5.0f;
    pacman.x = 1;
    pacman.y = 1;
    pacman.targetX = 1;
    pacman.targetY = 1;
    pacman.moveTimer = 0.0f;

    ghost.speed = 5.0f;
    ghost.x = 30;
    ghost.y = 30;
    ghost.targetX = 30;
    ghost.targetY = 30;
    ghost.moveTimer = 0.0f;

    InitDots();
}

bool CheckGhostCollision(Player *pacman, Player *ghost) {
    MOVE_SPEED = 5;
    return pacman->x == ghost->x && pacman->y == ghost->y;
}

void DrawGreenFeed(Texture2D texture) {
    for (int i = 0; i < GREEN_FEED_COUNT; i++) {
        if (greenFeeds[i].active) {
            DrawTexture(texture, greenFeeds[i].x * CELL_SIZE, greenFeeds[i].y * CELL_SIZE, WHITE);
        }
    }
}
/*
void DrawBlueFeed(Texture2D texture) {
    if (bluefeed.active) {
       DrawTexture(texture, bluefeed.x, bluefeed.y, WHITE);
    }
}

void CheckBlueFeedCollision(Ghost *ghost, Player *pacman) {
    if (bluefeed.active && ghost->x == bluefeed.x && ghost->y == bluefeed.y) {
        bluefeed.active = false;
        isBlueFeedActive = true;
        blueFeedTimer = 0.0f;
        MOVE_SPEED = 0;  // Stop's the pac-man
    }
}


void UpdateBlueFeedTimer(float deltaTime) {
    if (isBlueFeedActive) {
        blueFeedTimer += deltaTime;
        if (blueFeedTimer >= BLUE_FEED_DURATION) {
            isBlueFeedActive = false;
            MOVE_SPEED = 5;  // get back the speed
        }
    }
}
*/



void CheckGreenFeedCollision(Player *pacman) {
    for (int i = 0; i < GREEN_FEED_COUNT; i++) {
        if (greenFeeds[i].active && pacman->x == greenFeeds[i].x && pacman->y == greenFeeds[i].y) {
            pacman->speed += 3.0f;
            score +=100;
            greenFeeds[i].active = false;
        }
    }
}



void DrawDots() {
    for (int i = 0; i < dotCount; i++) {
        if (dots[i].active) {
            DrawCircle(dots[i].x * CELL_SIZE + CELL_SIZE/2, dots[i].y * CELL_SIZE + CELL_SIZE/2, 5, YELLOW);
        }
    }
}

void CheckDotCollision(Player *pacman) {
    for (int i = 0; i < dotCount; i++) {
        if (dots[i].active && pacman->x == dots[i].x && pacman->y == dots[i].y) {
            dots[i].active = false;
            score += 10;
            PlaySound(coinSound);
        }
    }
}


void DrawMap() {
    for (int y = 0; y < MAP_ROWS; y++) {
        for (int x = 0; x < MAP_COLS; x++) {
            if (map[y][x] == 0) {
                DrawRectangleLines(x * CELL_SIZE, y * CELL_SIZE, CELL_SIZE, CELL_SIZE, BLUE);
            }
        }
    }
}

void DrawGhost(Ghost *ghost) {
    DrawCircle(ghost->x * CELL_SIZE + CELL_SIZE / 2, ghost->y * CELL_SIZE + CELL_SIZE / 2, CELL_SIZE / 2 - 2, BLUE);
}


bool IsWalkable(int x, int y) {
    if (x < 0 || y < 0 || x >= MAP_COLS || y >= MAP_ROWS) return false;
    return map[y][x] == 1;
}

int main() {
    InitWindow(SCREEN_SIZE, SCREEN_SIZE, "Pac-Man - COOP");
    SetTargetFPS(60);
    LoadHighScore();
    InitAudioDevice();

    InitDots();
    // Pacman
    
    Texture2D pacmanTexture1 = LoadTexture("pacman1.png");
    Texture2D pacmanTexture2 = LoadTexture("pacman2.png");
    float frameTimer = 0.0f;
    int currentFrame = 0;
    float frameSpeed = 0.2f;

    // Ghost
    Texture2D ghostTexture1 = LoadTexture("ghost1.png");
    Texture2D ghostTexture2 = LoadTexture("ghost2.png");
    Texture2D greenFeed = LoadTexture("greenfeed.png");
    Texture2D bluefeed = LoadTexture("bluefeed.png");
    float ghostFrameTimer = 0.0f;
    int ghostCurrentFrame = 0;
    float ghostFrameSpeed = 0.3f; 

    Texture2D heartTexture = LoadTexture("heart.png");
    gameOverSound = LoadSound("gameover.wav");
    coinSound = LoadSound("coin.wav");
    death = LoadSound("death.wav");

    





    while (!WindowShouldClose()) {
        float delta = GetFrameTime();
        pacman.moveTimer += delta;
        frameTimer += delta;
        
        BeginDrawing();

    if (showMenu) {
        DrawMainMenu();

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            Vector2 mousePos = GetMousePosition();

            if (CheckCollisionPointRec(mousePos, playButton)) {
                showMenu = false; // Game start
            }
            if (CheckCollisionPointRec(mousePos, quitButton)) {
                break; 
            }
            if (CheckCollisionPointRec(mousePos, soundButton)) {
                soundOn = !soundOn;
                if (soundOn) {
                    SetMasterVolume(1.0f);
                } else {
                    SetMasterVolume(0.0f);
                }
            }
        }

        EndDrawing();
        continue; // Stop the game in menu screen
    }

        

        
        if (!isTimeOver) {
            remainingTime -= GetFrameTime();
            int currentMinute = (int)(remainingTime) / 60;
            if (currentMinute < lastMinute) {
                ghost.speed += 2.0f;  // Ghost speed-up
                lastMinute = currentMinute;  // minute update
            } 
            if (remainingTime <= 0) {
                remainingTime = 0;
                isTimeOver = true;
                PlaySound(gameOverSound);
                BeginDrawing();
                ClearBackground(BLACK);
                DrawText("GAME OVER", SCREEN_SIZE/2 - MeasureText("GAME OVER", 60)/2, SCREEN_SIZE/2 - 30, 60, RED);
                DrawText("Press R to Restart", SCREEN_SIZE/2 - MeasureText("Press R to Restart", 30)/2, SCREEN_SIZE/2 + 40, 30, WHITE);
                EndDrawing();
            }
        }

        
        if (frameTimer >= frameSpeed) {
            frameTimer = 0.0f;
            currentFrame = (currentFrame + 1) % 2; // passing between 0 and 1 
        }

        ghostFrameTimer += delta;
        if (ghostFrameTimer >= ghostFrameSpeed) {
            ghostFrameTimer = 0.0f;
            ghostCurrentFrame = (ghostCurrentFrame + 1) % 2;
        }


        if (pacman.moveTimer >= 1.0f / pacman.speed) {
            pacman.moveTimer = 0.0f;

            int moveX = 0, moveY = 0;
            if (IsKeyDown(KEY_RIGHT)) moveX = 1;
            if (IsKeyDown(KEY_LEFT))  moveX = -1;
            if (IsKeyDown(KEY_UP))    moveY = -1;
            if (IsKeyDown(KEY_DOWN))  moveY = 1;

            int targetX = pacman.x + moveX;
            int targetY = pacman.y + moveY;

            if (IsWalkable(targetX, pacman.y)) pacman.x = targetX;
            if (IsWalkable(pacman.x, targetY)) pacman.y = targetY;
}

// ----------------- Ghost Movement (WASD) -----------------
    ghost.moveTimer += delta;
        if (ghost.moveTimer >= 1.0f / ghost.speed) {
            ghost.moveTimer = 0.0f;

            int moveX = 0, moveY = 0;
            if (IsKeyDown(KEY_D)) moveX = 1;
            if (IsKeyDown(KEY_A)) moveX = -1;
            if (IsKeyDown(KEY_W)) moveY = -1;
            if (IsKeyDown(KEY_S)) moveY = 1;

            int targetX = ghost.x + moveX;
            int targetY = ghost.y + moveY;

            if (IsWalkable(targetX, ghost.y)) ghost.x = targetX;
            if (IsWalkable(ghost.x, targetY)) ghost.y = targetY;
       
        

            
        }
        CheckDotCollision(&pacman);
        CheckGreenFeedCollision(&pacman);
        //CheckBlueFeedCollision(&ghost, &pacman);
        //UpdateBlueFeedTimer(delta);


        if (CheckGhostCollision(&pacman, &ghost)) {
            PlaySound(death);
            lives--;
            if (lives <= 0) {
                SaveHighScore();
                PlaySound(gameOverSound);
                // Game Over Screen
                while (!WindowShouldClose()) {
                    BeginDrawing();
                    ClearBackground(BLACK);
                    DrawText("GAME OVER", SCREEN_SIZE/2 - MeasureText("GAME OVER", 60)/2, SCREEN_SIZE/2 - 30, 60, RED);
                    DrawText("Press R to Restart", SCREEN_SIZE/2 - MeasureText("Press R to Restart", 30)/2, SCREEN_SIZE/2 + 40, 30, WHITE);
                    EndDrawing();

                    if (IsKeyPressed(KEY_R)) {
                        lives = 3;
                        score = 0;
                        LoadHighScore();
                        ResetGame();
                        break;
                    }
                }
            } else {
                ResetGame();
            }
        }

        bool allDotsCollected = true;
        for (int i = 0; i < dotCount; i++) {
            if (dots[i].active) {
                allDotsCollected = false;
                break;
            }
        }
        if (allDotsCollected) {
            SaveHighScore();
            BeginDrawing();
            ClearBackground(BLACK);
            DrawText("YOU WIN", SCREEN_SIZE/2 - MeasureText("YOU WIN", 60)/2, SCREEN_SIZE/2 - 30, 60, GREEN);
            DrawText("Press R to Restart", SCREEN_SIZE/2 - MeasureText("Press R to Restart", 30)/2, SCREEN_SIZE/2 + 40, 30, WHITE);
            EndDrawing();
            if (IsKeyPressed(KEY_R)) {
                        lives = 3;
                        score = 0;
                        LoadHighScore();
                        ResetGame();
                        break;
                    }
            
        }
 
        BeginDrawing();
        ClearBackground(BLACK);
        char scoreText[64];
        sprintf(scoreText, "Score: %d", score);
        DrawText(scoreText, 20, 50, 20, GREEN);

        char highScoreText[64];
        sprintf(highScoreText, "High Score: %d", highScore);
        DrawText(highScoreText, 20, 80, 20, GREEN);
        for (int i = 0; i < lives; i++) {
            DrawTexture(heartTexture, SCREEN_SIZE - (i+1)*(CELL_SIZE+5) - 20, 50, WHITE);
        }
        int minutes = (int)remainingTime / 60;
        int seconds = (int)remainingTime % 60;
        char timeText[20];
        sprintf(timeText, "%02d:%02d", minutes, seconds);

        DrawText(timeText, SCREEN_SIZE - 160, 9, 50, WHITE); // text - x_axis - y_axis - Font Scale - Colour



        DrawMap();
        DrawDots();
        DrawGreenFeed(greenFeed);
        //DrawBlueFeed(bluefeed);
        

        Texture2D currentPacmanTexture = (currentFrame == 0) ? pacmanTexture1 : pacmanTexture2;
        DrawTexture(currentPacmanTexture, pacman.x * CELL_SIZE, pacman.y * CELL_SIZE, WHITE);

        Texture2D currentGhostTexture = (ghostCurrentFrame == 0) ? ghostTexture1 : ghostTexture2;
        DrawTexture(currentGhostTexture, ghost.x * CELL_SIZE, ghost.y * CELL_SIZE, WHITE);





        DrawText("Couch co-op PacMan", 20, 10, 20, WHITE);

        EndDrawing();
    }

    UnloadTexture(pacmanTexture1);
    UnloadTexture(pacmanTexture2);
    UnloadTexture(ghostTexture1);
    UnloadTexture(ghostTexture2);
    UnloadTexture(heartTexture);
    UnloadTexture(greenFeed);
    UnloadTexture(bluefeed);
    
    
    UnloadSound(gameOverSound);
    UnloadSound(coinSound);
    UnloadSound(death);
    CloseAudioDevice();



    CloseWindow();
    return 0;
}