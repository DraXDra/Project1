#ifndef GAME_H
#define GAME_H

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#include <vector>
#include "GameObject.h"
#include "WeatherSystem.h"
#include "HighScore.h"

class Game {
public:
    Game();
    ~Game();
    bool init();
    void run();

private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* playerTexture;
    SDL_Texture* obstacleTexture;
    SDL_Texture* backgroundTexture;
    SDL_Texture* logoTexture;
    Mix_Music* bgMusic;
    Mix_Chunk* hitSound;
    TTF_Font* font;
    SDL_Color textColor = {255, 255, 255, 255};
    SDL_Color highlightColor = {255, 255, 0, 255};
    float playerX, playerY;
    float targetX, targetY;
    std::vector<GameObject> objects;
    bool running;
    Uint32 lastSpawnTime;
    Uint32 gameStartTime;
    Uint32 lastFlashTime;
    int score;
    float currentObjectSpeed;
    int currentSpawnInterval;
    enum class GameState { MENU, PLAYING, PLAYING_SURVIVAL, GAME_OVER };
    GameState state;
    int menuSelection;
    WeatherSystem weatherSystem;
    HighScore highScore;

    bool initSDL();
    bool loadAssets();
    bool checkCollision(float x1, float y1, int w1, int h1, float x2, float y2, int w2, int h2);
    float distance(float x1, float y1, float x2, float y2);
    void movePlayerToTarget();
    void saveGameState();
    bool loadGameState();
    void updateScore();
    void updateObjectSpeed();
    void renderMenu();
};

#endif
