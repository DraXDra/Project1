#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <fstream>
#include <string>

using namespace std;

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const int PLAYER_WIDTH = 50;
const int PLAYER_HEIGHT = 50;
const int OBJECT_SIZE = 30;
const float PLAYER_SPEED = 5.0f;
const float INITIAL_OBJECT_SPEED = 3.0f;
const float SPEED_INCREMENT = 0.5f;
const int SPEED_INCREASE_INTERVAL = 10000; // 10 giây
const int SPAWN_INTERVAL = 500; // ms
const int FLASH_COOLDOWN = 15000; // 15 giây (mili giây)
const int FLASH_DISTANCE = 200; // Khoảng cách dịch chuyển

struct GameObject {
    float x, y;
    float dx, dy;
    SDL_Texture* texture;
};

class Game {
private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* playerTexture;
    SDL_Texture* obstacleTexture;
    SDL_Texture* backgroundTexture;
    SDL_Texture* logoTexture; // Texture cho logo
    Mix_Music* bgMusic;
    Mix_Chunk* hitSound;
    TTF_Font* font;
    SDL_Color textColor = {255, 255, 255, 255};
    SDL_Color highlightColor = {255, 255, 0, 255};
    float playerX, playerY;
    float targetX, targetY;
    vector<GameObject> objects;
    bool running;
    Uint32 lastSpawnTime;
    Uint32 gameStartTime;
    Uint32 lastFlashTime; // Thời gian sử dụng chiêu thức flash lần cuối
    int score;
    int highScore;
    float currentObjectSpeed;
    enum class GameState { MENU, PLAYING, GAME_OVER };
    GameState state;
    int menuSelection;

    bool initSDL() {
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
            cerr << "Failed to initialize SDL: " << SDL_GetError() << endl;
            return false;
        }
        if (IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG) {
            cerr << "Failed to initialize SDL_image: " << SDL_GetError() << endl;
            return false;
        }
        if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
            cerr << "Failed to initialize SDL_mixer: " << Mix_GetError() << endl;
            return false;
        }
        if (TTF_Init() < 0) {
            cerr << "Failed to initialize SDL_ttf: " << TTF_GetError() << endl;
            return false;
        }
        window = SDL_CreateWindow("Dodge", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                  WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
        if (!window) {
            cerr << "Failed to create window: " << SDL_GetError() << endl;
            return false;
        }
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        if (!renderer) {
            cerr << "Failed to create renderer: " << SDL_GetError() << endl;
            return false;
        }
        return true;
    }

    bool loadAssets() {
        backgroundTexture = IMG_LoadTexture(renderer, "assets/background.png");
        if (!backgroundTexture) {
            cerr << "Failed to load background texture: " << SDL_GetError() << endl;
            return false;
        }
        playerTexture = IMG_LoadTexture(renderer, "assets/player.png");
        if (!playerTexture) {
            cerr << "Failed to load player texture: " << SDL_GetError() << endl;
            return false;
        }
        obstacleTexture = IMG_LoadTexture(renderer, "assets/obstacle.png");
        if (!obstacleTexture) {
            cerr << "Failed to load obstacle texture: " << SDL_GetError() << endl;
            return false;
        }
        logoTexture = IMG_LoadTexture(renderer, "assets/logo.png"); // Tải logo
        if (!logoTexture) {
            cerr << "Failed to load logo texture: " << SDL_GetError() << endl;
            return false;
        }
        bgMusic = Mix_LoadMUS("assets/background_music.mp3");
        if (!bgMusic) {
            cerr << "Failed to load background music: " << Mix_GetError() << endl;
            return false;
        }
        hitSound = Mix_LoadWAV("assets/hit.mp3");
        if (!hitSound) {
            cerr << "Failed to load sound effect: " << Mix_GetError() << endl;
            return false;
        }
        font = TTF_OpenFont("assets/arial.ttf", 24);
        if (!font) {
            cerr << "Failed to load font: " << TTF_GetError() << endl;
            return false;
        }
        return true;
    }

    void spawnObject() {
        GameObject obj;
        int side = rand() % 4;
        switch (side) {
            case 0: // From top
                obj.x = rand() % (WINDOW_WIDTH - OBJECT_SIZE);
                obj.y = -OBJECT_SIZE;
                obj.dx = 0;
                obj.dy = currentObjectSpeed;
                break;
            case 1: // From bottom
                obj.x = rand() % (WINDOW_WIDTH - OBJECT_SIZE);
                obj.y = WINDOW_HEIGHT;
                obj.dx = 0;
                obj.dy = -currentObjectSpeed;
                break;
            case 2: // From left
                obj.x = -OBJECT_SIZE;
                obj.y = rand() % (WINDOW_HEIGHT - OBJECT_SIZE);
                obj.dx = currentObjectSpeed;
                obj.dy = 0;
                break;
            case 3: // From right
                obj.x = WINDOW_WIDTH;
                obj.y = rand() % (WINDOW_HEIGHT - OBJECT_SIZE);
                obj.dx = -currentObjectSpeed;
                obj.dy = 0;
                break;
        }
        obj.texture = obstacleTexture;
        objects.push_back(obj);
    }

    bool checkCollision(float x1, float y1, int w1, int h1, float x2, float y2, int w2, int h2) {
        return x1 < x2 + w2 && x1 + w1 > x2 && y1 < y2 + h2 && y1 + h1 > y2;
    }

    float distance(float x1, float y1, float x2, float y2) {
        return sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
    }

    void movePlayerToTarget() {
        if (distance(playerX, playerY, targetX, targetY) > 5.0f) {
            float dx = targetX - playerX;
            float dy = targetY - playerY;
            float dist = distance(playerX, playerY, targetX, targetY);
            float moveX = (dx / dist) * PLAYER_SPEED;
            float moveY = (dy / dist) * PLAYER_SPEED;

            if (abs(moveX) > abs(dx)) moveX = dx;
            if (abs(moveY) > abs(dy)) moveY = dy;

            playerX += moveX;
            playerY += moveY;

            if (playerX < 0) playerX = 0;
            if (playerX > WINDOW_WIDTH - PLAYER_WIDTH) playerX = WINDOW_WIDTH - PLAYER_WIDTH;
            if (playerY < 0) playerY = 0;
            if (playerY > WINDOW_HEIGHT - PLAYER_HEIGHT) playerY = WINDOW_HEIGHT - PLAYER_HEIGHT;
        }
    }

    void saveGameState() {
        ofstream outFile("savegame.txt");
        if (!outFile) {
            cerr << "Failed to save game state!" << endl;
            return;
        }
        outFile << playerX << " " << playerY << "\n";
        outFile << (SDL_GetTicks() - gameStartTime) << "\n";
        outFile << score << "\n";
        outFile << objects.size() << "\n";
        for (const auto& obj : objects) {
            outFile << obj.x << " " << obj.y << " " << obj.dx << " " << obj.dy << "\n";
        }
        outFile.close();
    }

    bool loadGameState() {
        ifstream inFile("savegame.txt");
        if (!inFile) {
            cerr << "Save game file not found!" << endl;
            return false;
        }
        float savedTime;
        inFile >> playerX >> playerY;
        inFile >> savedTime;
        inFile >> score;
        gameStartTime = SDL_GetTicks() - savedTime;
        size_t numObjects;
        inFile >> numObjects;
        objects.clear();
        for (size_t i = 0; i < numObjects; ++i) {
            GameObject obj;
            inFile >> obj.x >> obj.y >> obj.dx >> obj.dy;
            obj.texture = obstacleTexture;
            objects.push_back(obj);
        }
        inFile.close();
        targetX = playerX;
        targetY = playerY;
        lastSpawnTime = SDL_GetTicks();
        return true;
    }

    void saveHighScore() {
        ofstream outFile("highscore.txt");
        if (!outFile) {
            cerr << "Failed to save high score!" << endl;
            return;
        }
        outFile << highScore;
        outFile.close();
    }

    void loadHighScore() {
        ifstream inFile("highscore.txt");
        if (!inFile) {
            highScore = 0;
            return;
        }
        inFile >> highScore;
        inFile.close();
    }

    void updateScore() {
        Uint32 currentTime = SDL_GetTicks();
        score = ((currentTime - gameStartTime) / 1000) * 10; // 1 second = 10 points
    }

    void updateObjectSpeed() {
        Uint32 currentTime = SDL_GetTicks();
        Uint32 elapsedTime = currentTime - gameStartTime;
        currentObjectSpeed = INITIAL_OBJECT_SPEED + (elapsedTime / SPEED_INCREASE_INTERVAL) * SPEED_INCREMENT;
    }

    void renderText(const string& text, int y, SDL_Color color) {
        SDL_Surface* surface = TTF_RenderText_Solid(font, text.c_str(), color);
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        int textWidth, textHeight;
        TTF_SizeText(font, text.c_str(), &textWidth, &textHeight);
        SDL_Rect dst = {(WINDOW_WIDTH - textWidth) / 2, y, textWidth, textHeight};
        SDL_RenderCopy(renderer, texture, nullptr, &dst);
        SDL_FreeSurface(surface);
        SDL_DestroyTexture(texture);
    }

    void renderMenu() {
        SDL_Rect bgRect = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
        SDL_RenderCopy(renderer, backgroundTexture, nullptr, &bgRect);

        renderText("Dodge Game", 100, textColor);
        renderText("High Score: " + to_string(highScore), 150, textColor);
        renderText("Play", 250, menuSelection == 0 ? highlightColor : textColor);
        renderText("Load Game", 300, menuSelection == 1 ? highlightColor : textColor);
        renderText("Exit", 350, menuSelection == 2 ? highlightColor : textColor);

        SDL_RenderPresent(renderer);
    }

public:
    Game() : window(nullptr), renderer(nullptr), playerTexture(nullptr), obstacleTexture(nullptr),
             backgroundTexture(nullptr), logoTexture(nullptr), bgMusic(nullptr), hitSound(nullptr), font(nullptr),
             playerX(WINDOW_WIDTH / 2.0f - PLAYER_WIDTH / 2.0f),
             playerY(WINDOW_HEIGHT / 2.0f - PLAYER_HEIGHT / 2.0f),
             targetX(playerX), targetY(playerY),
             running(true), lastSpawnTime(0), gameStartTime(0), lastFlashTime(0),
             score(0), highScore(0), currentObjectSpeed(INITIAL_OBJECT_SPEED),
             state(GameState::MENU), menuSelection(0) {
        srand(time(nullptr));
        loadHighScore();
    }

    ~Game() {
        saveHighScore();
        if (playerTexture) SDL_DestroyTexture(playerTexture);
        if (obstacleTexture) SDL_DestroyTexture(obstacleTexture);
        if (backgroundTexture) SDL_DestroyTexture(backgroundTexture);
        if (logoTexture) SDL_DestroyTexture(logoTexture); // Giải phóng logo
        if (bgMusic) Mix_FreeMusic(bgMusic);
        if (hitSound) Mix_FreeChunk(hitSound);
        if (font) TTF_CloseFont(font);
        if (renderer) SDL_DestroyRenderer(renderer);
        if (window) SDL_DestroyWindow(window);
        TTF_Quit();
        Mix_Quit();
        IMG_Quit();
        SDL_Quit();
    }

    bool init() {
        return initSDL() && loadAssets();
    }

    void run() {
        while (running) {
            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_QUIT) {
                    running = false;
                } else if (state == GameState::MENU) {
                    if (event.type == SDL_KEYDOWN) {
                        if (event.key.keysym.sym == SDLK_UP) {
                            menuSelection = (menuSelection - 1 + 3) % 3;
                        } else if (event.key.keysym.sym == SDLK_DOWN) {
                            menuSelection = (menuSelection + 1) % 3;
                        } else if (event.key.keysym.sym == SDLK_RETURN) {
                            if (menuSelection == 0) { // Play
                                state = GameState::PLAYING;
                                gameStartTime = SDL_GetTicks();
                                score = 0;
                                objects.clear();
                                currentObjectSpeed = INITIAL_OBJECT_SPEED;
                                Mix_PlayMusic(bgMusic, -1);
                            } else if (menuSelection == 1) { // Load state
                                if (loadGameState()) {
                                    state = GameState::PLAYING;
                                    Mix_PlayMusic(bgMusic, -1);
                                }
                            } else if (menuSelection == 2) { // Exit
                                running = false;
                            }
                        }
                    }
                } else if (state == GameState::PLAYING) {
                    if (event.type == SDL_MOUSEMOTION) {
                        targetX = event.motion.x - PLAYER_WIDTH / 2.0f;
                        targetY = event.motion.y - PLAYER_HEIGHT / 2.0f;
                    } else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_s) {
                        Uint32 currentTime = SDL_GetTicks();
                        if (currentTime - lastFlashTime >= FLASH_COOLDOWN) {
                            // Tính hướng dịch chuyển
                            float dx = targetX - playerX;
                            float dy = targetY - playerY;
                            float dist = distance(playerX, playerY, targetX, targetY);
                            if (dist > 0) {
                                float moveX = (dx / dist) * FLASH_DISTANCE;
                                float moveY = (dy / dist) * FLASH_DISTANCE;
                                playerX += moveX;
                                playerY += moveY;

                                // Giới hạn trong màn hình
                                if (playerX < 0) playerX = 0;
                                if (playerX > WINDOW_WIDTH - PLAYER_WIDTH) playerX = WINDOW_WIDTH - PLAYER_WIDTH;
                                if (playerY < 0) playerY = 0;
                                if (playerY > WINDOW_HEIGHT - PLAYER_HEIGHT) playerY = WINDOW_HEIGHT - PLAYER_HEIGHT;

                                lastFlashTime = currentTime;
                            }
                        } else {
                            saveGameState();
                            Mix_HaltMusic();
                            state = GameState::MENU;
                        }
                    }
                } else if (state == GameState::GAME_OVER) {
                    if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_RETURN) {
                        state = GameState::MENU;
                    }
                }
            }

            if (state == GameState::MENU) {
                renderMenu();
            } else if (state == GameState::PLAYING) {
                movePlayerToTarget();

                Uint32 currentTime = SDL_GetTicks();
                if (currentTime - lastSpawnTime > SPAWN_INTERVAL) {
                    updateObjectSpeed();
                    spawnObject();
                    lastSpawnTime = currentTime;
                }

                for (auto it = objects.begin(); it != objects.end();) {
                    it->x += it->dx;
                    it->y += it->dy;

                    if (it->x < -OBJECT_SIZE || it->x > WINDOW_WIDTH ||
                        it->y < -OBJECT_SIZE || it->y > WINDOW_HEIGHT) {
                        it = objects.erase(it);
                        continue;
                    }

                    if (checkCollision(playerX, playerY, PLAYER_WIDTH, PLAYER_HEIGHT,
                                       it->x, it->y, OBJECT_SIZE, OBJECT_SIZE)) {
                        Mix_PlayChannel(-1, hitSound, 0);
                        Mix_HaltMusic();
                        state = GameState::GAME_OVER;
                        if (score > highScore) {
                            highScore = score;
                            saveHighScore();
                        }
                    }
                    ++it;
                }

                updateScore();

                SDL_Rect bgRect = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
                SDL_RenderCopy(renderer, backgroundTexture, nullptr, &bgRect);

                SDL_Rect playerRect = {(int)playerX, (int)playerY, PLAYER_WIDTH, PLAYER_HEIGHT};
                SDL_RenderCopy(renderer, playerTexture, nullptr, &playerRect);

                for (const auto& obj : objects) {
                    SDL_Rect objRect = {(int)obj.x, (int)obj.y, OBJECT_SIZE, OBJECT_SIZE};
                    SDL_RenderCopy(renderer, obj.texture, nullptr, &objRect);
                }

                renderText("Score: " + to_string(score), 10, textColor);
                renderText("Press S to flash (15s cooldown)", 40, textColor);

                // Vẽ logo chỉ trong gameplay
                SDL_Rect logoRect = {(WINDOW_WIDTH / 2) - 50, WINDOW_HEIGHT - 100, 100, 50};
                SDL_RenderCopy(renderer, logoTexture, nullptr, &logoRect);

                SDL_RenderPresent(renderer);
            } else if (state == GameState::GAME_OVER) {
                SDL_Rect bgRect = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
                SDL_RenderCopy(renderer, backgroundTexture, nullptr, &bgRect);

                renderText("Game Over!", (WINDOW_HEIGHT / 2) - 50, textColor);
                renderText("Score: " + to_string(score), WINDOW_HEIGHT / 2, textColor);
                renderText("Press Enter to return to menu", (WINDOW_HEIGHT / 2) + 50, textColor);

                SDL_RenderPresent(renderer);
            }

            SDL_Delay(16); // ~60 FPS
        }
    }
};

int main(int argc, char* argv[]) {
    Game game;
    if (!game.init()) {
        cerr << "Initialization failed!" << endl;
        return 1;
    }
    game.run();
    return 0;
}
