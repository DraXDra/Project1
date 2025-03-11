// SDL Endless Runner Game - Improved Version
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <iostream>
#include <vector>
#include <random>
#include <algorithm>
#include <string>
#include <sstream>

// Game constants
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const int GROUND_HEIGHT = 500;
const float GRAVITY = 0.6f;
const float JUMP_FORCE = -16.0f;
const float SCROLL_SPEED = 6.0f;

// Player constants
const int PLAYER_WIDTH = 60;
const int PLAYER_HEIGHT = 90;
const int PLAYER_START_X = 100;

// Obstacle constants
const int MIN_OBSTACLE_DISTANCE = 400;
const int MAX_OBSTACLE_DISTANCE = 800;
const int MIN_OBSTACLE_WIDTH = 40;
const int MAX_OBSTACLE_WIDTH = 80;
const int OBSTACLE_HEIGHT = 60;

// Animation constants
const int PLAYER_FRAME_COUNT = 2;
const int ANIMATION_SPEED = 100; // milliseconds per frame

enum GameState {
    MENU,
    PLAYING,
    GAME_OVER
};

// Game objects
struct Player {
    float x, y;
    float velocityY;
    bool isJumping;
    SDL_Rect rect;
    int currentFrame;
    Uint32 lastFrameTime;

    Player() : x(PLAYER_START_X), y(GROUND_HEIGHT - PLAYER_HEIGHT),
               velocityY(0), isJumping(false), currentFrame(0), lastFrameTime(0) {
        rect = {(int)x, (int)y, PLAYER_WIDTH, PLAYER_HEIGHT};
    }

    void update(Uint32 currentTime) {
        // Apply gravity
        if (isJumping) {
            velocityY += GRAVITY;
            y += velocityY;

            // Check ground
            if (y > GROUND_HEIGHT - PLAYER_HEIGHT) {
                y = GROUND_HEIGHT - PLAYER_HEIGHT;
                isJumping = false;
                velocityY = 0;
            }
        }

        // Update animation
        if (currentTime - lastFrameTime > ANIMATION_SPEED) {
            currentFrame = (currentFrame + 1) % PLAYER_FRAME_COUNT;
            lastFrameTime = currentTime;
        }

        // Update rect position
        rect.x = (int)x;
        rect.y = (int)y;
    }

    void jump() {
        if (!isJumping) {
            velocityY = JUMP_FORCE;
            isJumping = true;
        }
    }
};

struct Obstacle {
    float x, y;
    int width, height;
    SDL_Rect rect;

    Obstacle(float startX, int w, int h) : x(startX), y(GROUND_HEIGHT - h), width(w), height(h) {
        rect = {(int)x, (int)y, width, height};
    }

    void update() {
        x -= SCROLL_SPEED;
        rect.x = (int)x;
    }

    bool isOffScreen() const {
        return x + width < 0;
    }
};

struct Cloud {
    float x, y;
    int width, height;
    float speed;

    Cloud(float startX, float startY, int w, int h, float s)
        : x(startX), y(startY), width(w), height(h), speed(s) {}

    void update() {
        x -= speed;
    }

    bool isOffScreen() const {
        return x + width < 0;
    }

    SDL_Rect getRect() const {
        return {(int)x, (int)y, width, height};
    }
};

class Game {
private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* playerTexture;
    SDL_Texture* obstacleTexture;
    SDL_Texture* backgroundTexture;
    SDL_Texture* cloudTexture;
    SDL_Texture* menuTexture;
    SDL_Texture* gameOverTexture;
    TTF_Font* font;

    Player player;
    std::vector<Obstacle> obstacles;
    std::vector<Cloud> clouds;

    GameState gameState;
    bool running;
    int score;
    int highScore;

    Uint32 lastObstacleTime;
    float gameSpeed;
    std::mt19937 rng;

public:
    Game() : window(nullptr), renderer(nullptr), playerTexture(nullptr),
             obstacleTexture(nullptr), backgroundTexture(nullptr), cloudTexture(nullptr),
             menuTexture(nullptr), gameOverTexture(nullptr), font(nullptr),
             gameState(MENU), running(false), score(0), highScore(0),
             lastObstacleTime(0), gameSpeed(1.0f) {
        // Initialize random number generator
        std::random_device rd;
        rng = std::mt19937(rd());
    }

    ~Game() {
        cleanup();
    }

    bool init() {
        // Initialize SDL
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
            std::cerr << "SDL could not initialize! SDL Error: " << SDL_GetError() << std::endl;
            return false;
        }

        // Initialize SDL_image
        int imgFlags = IMG_INIT_PNG;
        if (!(IMG_Init(imgFlags) & imgFlags)) {
            std::cerr << "SDL_image could not initialize! SDL_image Error: " << IMG_GetError() << std::endl;
            return false;
        }

        // Initialize SDL_ttf
        if (TTF_Init() == -1) {
            std::cerr << "SDL_ttf could not initialize! SDL_ttf Error: " << TTF_GetError() << std::endl;
            return false;
        }

        // Create window
        window = SDL_CreateWindow("Endless Runner", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                  SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
        if (window == nullptr) {
            std::cerr << "Window could not be created! SDL Error: " << SDL_GetError() << std::endl;
            return false;
        }

        // Create renderer
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
        if (renderer == nullptr) {
            std::cerr << "Renderer could not be created! SDL Error: " << SDL_GetError() << std::endl;
            return false;
        }

        // Set renderer color
        SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);

        // Load font
        font = TTF_OpenFont("assets/font.ttf", 28);
        if (font == nullptr) {
            std::cerr << "Failed to load font! SDL_ttf Error: " << TTF_GetError() << std::endl;
            // Continue anyway, we'll render without text
        }

        // Load textures
        if (!loadTextures()) {
            std::cerr << "Failed to load textures!" << std::endl;
            return false;
        }

        return true;
    }

    bool loadTextures() {
        // Create colored textures for our game objects

        // Player texture with animation frames (blue rectangle divided in two)
        SDL_Surface* playerSurface = SDL_CreateRGBSurface(0, PLAYER_WIDTH * PLAYER_FRAME_COUNT, PLAYER_HEIGHT, 32, 0, 0, 0, 0);
        if (playerSurface == nullptr) {
            std::cerr << "Unable to create player surface! SDL Error: " << SDL_GetError() << std::endl;
            return false;
        }

        // First frame - standing
        SDL_Rect frameRect = {0, 0, PLAYER_WIDTH, PLAYER_HEIGHT};
        SDL_FillRect(playerSurface, &frameRect, SDL_MapRGB(playerSurface->format, 50, 100, 255));

        // Second frame - running
        frameRect = {PLAYER_WIDTH, 0, PLAYER_WIDTH, PLAYER_HEIGHT};
        SDL_FillRect(playerSurface, &frameRect, SDL_MapRGB(playerSurface->format, 30, 80, 220));

        // Create texture from surface
        playerTexture = SDL_CreateTextureFromSurface(renderer, playerSurface);
        SDL_FreeSurface(playerSurface);

        // Obstacle texture (red rectangle with details)
        SDL_Surface* obstacleSurface = SDL_CreateRGBSurface(0, MAX_OBSTACLE_WIDTH, OBSTACLE_HEIGHT, 32, 0, 0, 0, 0);
        if (obstacleSurface == nullptr) {
            std::cerr << "Unable to create obstacle surface! SDL Error: " << SDL_GetError() << std::endl;
            return false;
        }

        // Fill with base color
        SDL_FillRect(obstacleSurface, NULL, SDL_MapRGB(obstacleSurface->format, 220, 50, 50));

        // Add some details
        SDL_Rect detailRect = {5, 5, MAX_OBSTACLE_WIDTH - 10, 10};
        SDL_FillRect(obstacleSurface, &detailRect, SDL_MapRGB(obstacleSurface->format, 180, 30, 30));

        obstacleTexture = SDL_CreateTextureFromSurface(renderer, obstacleSurface);
        SDL_FreeSurface(obstacleSurface);

        // Background texture with parallax layers
        SDL_Surface* bgSurface = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32, 0, 0, 0, 0);
        if (bgSurface == nullptr) {
            std::cerr << "Unable to create background surface! SDL Error: " << SDL_GetError() << std::endl;
            return false;
        }

        // Sky gradient (light blue to darker blue)
        for (int y = 0; y < GROUND_HEIGHT; y++) {
            SDL_Rect lineRect = {0, y, SCREEN_WIDTH, 1};
            int blue = 235 - (y * 30 / GROUND_HEIGHT);
            SDL_FillRect(bgSurface, &lineRect, SDL_MapRGB(bgSurface->format, 135, 206, blue));
        }

        // Ground (brown with details)
        SDL_Rect groundRect = {0, GROUND_HEIGHT, SCREEN_WIDTH, SCREEN_HEIGHT - GROUND_HEIGHT};
        SDL_FillRect(bgSurface, &groundRect, SDL_MapRGB(bgSurface->format, 139, 90, 43));

        // Ground detail line
        SDL_Rect groundLineRect = {0, GROUND_HEIGHT, SCREEN_WIDTH, 3};
        SDL_FillRect(bgSurface, &groundLineRect, SDL_MapRGB(bgSurface->format, 101, 67, 33));

        backgroundTexture = SDL_CreateTextureFromSurface(renderer, bgSurface);
        SDL_FreeSurface(bgSurface);

        // Cloud texture (white fluffy shape)
        SDL_Surface* cloudSurface = SDL_CreateRGBSurface(0, 100, 60, 32, 0, 0, 0, 0);
        if (cloudSurface == nullptr) {
            std::cerr << "Unable to create cloud surface! SDL Error: " << SDL_GetError() << std::endl;
            return false;
        }

        // Make it transparent initially
        SDL_FillRect(cloudSurface, NULL, SDL_MapRGBA(cloudSurface->format, 0, 0, 0, 0));

        // Draw cloud shape
        filledCircleRGBA(cloudSurface, 30, 30, 25, 255, 255, 255, 220);
        filledCircleRGBA(cloudSurface, 50, 35, 30, 255, 255, 255, 220);
        filledCircleRGBA(cloudSurface, 70, 30, 25, 255, 255, 255, 220);

        cloudTexture = SDL_CreateTextureFromSurface(renderer, cloudSurface);
        SDL_SetTextureBlendMode(cloudTexture, SDL_BLENDMODE_BLEND);
        SDL_FreeSurface(cloudSurface);

        // Menu texture (simple text)
        SDL_Surface* menuSurface = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32, 0, 0, 0, 0);
        if (menuSurface == nullptr) {
            std::cerr << "Unable to create menu surface! SDL Error: " << SDL_GetError() << std::endl;
            return false;
        }

        // Semi-transparent background
        SDL_FillRect(menuSurface, NULL, SDL_MapRGBA(menuSurface->format, 0, 0, 0, 180));

        menuTexture = SDL_CreateTextureFromSurface(renderer, menuSurface);
        SDL_SetTextureBlendMode(menuTexture, SDL_BLENDMODE_BLEND);
        SDL_FreeSurface(menuSurface);

        // Game over texture (similar to menu)
        SDL_Surface* gameOverSurface = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32, 0, 0, 0, 0);
        if (gameOverSurface == nullptr) {
            std::cerr << "Unable to create game over surface! SDL Error: " << SDL_GetError() << std::endl;
            return false;
        }

        // Semi-transparent background
        SDL_FillRect(gameOverSurface, NULL, SDL_MapRGBA(gameOverSurface->format, 0, 0, 0, 180));

        gameOverTexture = SDL_CreateTextureFromSurface(renderer, gameOverSurface);
        SDL_SetTextureBlendMode(gameOverTexture, SDL_BLENDMODE_BLEND);
        SDL_FreeSurface(gameOverSurface);

        return (playerTexture != nullptr && obstacleTexture != nullptr &&
                backgroundTexture != nullptr && cloudTexture != nullptr &&
                menuTexture != nullptr && gameOverTexture != nullptr);
    }

    void initGame() {
        player = Player();
        obstacles.clear();
        clouds.clear();
        score = 0;
        gameSpeed = 1.0f;
        lastObstacleTime = SDL_GetTicks();

        // Generate initial clouds
        generateClouds();
    }

    void run() {
        running = true;
        Uint32 lastTicks = SDL_GetTicks();

        while (running) {
            Uint32 currentTicks = SDL_GetTicks();
            float deltaTime = (currentTicks - lastTicks) / 1000.0f;
            lastTicks = currentTicks;

            handleEvents();

            switch (gameState) {
                case MENU:
                    renderMenu();
                    break;
                case PLAYING:
                    update(currentTicks);
                    render();
                    break;
                case GAME_OVER:
                    renderGameOver();
                    break;
            }

            // Cap to ~60 FPS
            Uint32 frameTicks = SDL_GetTicks() - currentTicks;
            if (frameTicks < 16) {
                SDL_Delay(16 - frameTicks);
            }
        }
    }

    void handleEvents() {
        SDL_Event e;
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                running = false;
            } else if (e.type == SDL_KEYDOWN) {
                handleKeyDown(e.key.keysym.sym);
            }
        }
    }

    void handleKeyDown(SDL_Keycode key) {
        switch (gameState) {
            case MENU:
                if (key == SDLK_RETURN || key == SDLK_SPACE) {
                    gameState = PLAYING;
                    initGame();
                } else if (key == SDLK_ESCAPE) {
                    running = false;
                }
                break;

            case PLAYING:
                if (key == SDLK_SPACE || key == SDLK_UP) {
                    player.jump();
                } else if (key == SDLK_ESCAPE) {
                    gameState = MENU;
                }
                break;

            case GAME_OVER:
                if (key == SDLK_RETURN || key == SDLK_SPACE) {
                    gameState = PLAYING;
                    initGame();
                } else if (key == SDLK_ESCAPE) {
                    gameState = MENU;
                }
                break;
        }
    }

    void generateClouds() {
        std::uniform_real_distribution<float> yDist(50.0f, 200.0f);
        std::uniform_int_distribution<int> sizeDist(60, 100);
        std::uniform_real_distribution<float> speedDist(0.5f, 2.0f);

        for (int i = 0; i < 5; i++) {
            float xPos = SCREEN_WIDTH * i / 5;
            float yPos = yDist(rng);
            int width = sizeDist(rng);
            int height = width * 0.6;
            float speed = speedDist(rng);

            clouds.push_back(Cloud(xPos, yPos, width, height, speed));
        }
    }

    void update(Uint32 currentTime) {
        // Update player
        player.update(currentTime);

        // Update obstacles
        updateObstacles(currentTime);

        // Update clouds
        updateClouds();

        // Increase game speed over time
        gameSpeed += 0.0001f;
    }

    void updateObstacles(Uint32 currentTime) {
        // Update existing obstacles
        for (auto it = obstacles.begin(); it != obstacles.end();) {
            it->update();
            if (it->isOffScreen()) {
                it = obstacles.erase(it);
                score++;
            } else {
                ++it;
            }
        }

        // Generate new obstacles
        if (obstacles.empty() || obstacles.back().x < SCREEN_WIDTH - MIN_OBSTACLE_DISTANCE * gameSpeed) {
            std::uniform_int_distribution<int> widthDist(MIN_OBSTACLE_WIDTH, MAX_OBSTACLE_WIDTH);
            std::uniform_int_distribution<int> heightDist(OBSTACLE_HEIGHT, OBSTACLE_HEIGHT + 20);

            int width = widthDist(rng);
            int height = heightDist(rng);

            obstacles.push_back(Obstacle(SCREEN_WIDTH, width, height));
        }

        // Check collisions
        for (const auto& obstacle : obstacles) {
            if (checkCollision(player.rect, obstacle.rect)) {
                if (score > highScore) {
                    highScore = score;
                }
                gameState = GAME_OVER;
                break;
            }
        }
    }

    void updateClouds() {
        // Update existing clouds
        for (auto it = clouds.begin(); it != clouds.end();) {
            it->update();
            if (it->isOffScreen()) {
                it = clouds.erase(it);
            } else {
                ++it;
            }
        }

        // Generate new clouds if needed
        if (clouds.size() < 5) {
            std::uniform_real_distribution<float> yDist(50.0f, 200.0f);
            std::uniform_int_distribution<int> sizeDist(60, 100);
            std::uniform_real_distribution<float> speedDist(0.5f, 2.0f);

            float yPos = yDist(rng);
            int width = sizeDist(rng);
            int height = width * 0.6;
            float speed = speedDist(rng);

            clouds.push_back(Cloud(SCREEN_WIDTH, yPos, width, height, speed));
        }
    }

    bool checkCollision(const SDL_Rect& a, const SDL_Rect& b) {
        // Shrink the collision box a bit for more forgiving gameplay
        SDL_Rect adjustedA = {a.x + 10, a.y + 10, a.w - 20, a.h - 20};
        return SDL_HasIntersection(&adjustedA, &b);
    }

    void render() {
        // Clear screen
        SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
        SDL_RenderClear(renderer);

        // Render background
        SDL_RenderCopy(renderer, backgroundTexture, NULL, NULL);

        // Render clouds
        for (const auto& cloud : clouds) {
            SDL_Rect cloudRect = cloud.getRect();
            SDL_RenderCopy(renderer, cloudTexture, NULL, &cloudRect);
        }

        // Render obstacles
        for (const auto& obstacle : obstacles) {
            SDL_RenderCopy(renderer, obstacleTexture, NULL, &obstacle.rect);
        }

        // Render player with animation
        SDL_Rect srcRect = {player.currentFrame * PLAYER_WIDTH, 0, PLAYER_WIDTH, PLAYER_HEIGHT};
        SDL_RenderCopy(renderer, playerTexture, &srcRect, &player.rect);

        // Render score
        renderText("Score: " + std::to_string(score), 10, 10);

        // Update screen
        SDL_RenderPresent(renderer);
    }

    void renderMenu() {
        // First render game scene in background
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, backgroundTexture, NULL, NULL);

        // Render overlay
        SDL_RenderCopy(renderer, menuTexture, NULL, NULL);

        // Render title and instructions
        renderText("ENDLESS RUNNER", SCREEN_WIDTH / 2 - 150, SCREEN_HEIGHT / 3);
        renderText("Press SPACE or ENTER to start", SCREEN_WIDTH / 2 - 200, SCREEN_HEIGHT / 2);
        renderText("Press ESC to quit", SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 + 50);

        if (highScore > 0) {
            renderText("High Score: " + std::to_string(highScore), SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 + 100);
        }

        SDL_RenderPresent(renderer);
    }

    void renderGameOver() {
        // First render game scene in background
        render();

        // Render overlay
        SDL_RenderCopy(renderer, gameOverTexture, NULL, NULL);

        // Render game over text
        renderText("GAME OVER", SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 3);
        renderText("Score: " + std::to_string(score), SCREEN_WIDTH / 2 - 70, SCREEN_HEIGHT / 2 - 20);
        renderText("High Score: " + std::to_string(highScore), SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 + 20);
        renderText("Press SPACE or ENTER to play again", SCREEN_WIDTH / 2 - 220, SCREEN_HEIGHT / 2 + 80);
        renderText("Press ESC for menu", SCREEN_WIDTH / 2 - 120, SCREEN_HEIGHT / 2 + 120);

        SDL_RenderPresent(renderer);
    }

    void renderText(const std::string& text, int x, int y) {
        if (font == nullptr) {
            // If font loading failed, draw a simple rectangle instead
            SDL_Rect textRect = {x, y, (int)text.length() * 14, 30};
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderFillRect(renderer, &textRect);
            return;
        }

        SDL_Color textColor = {255, 255, 255, 255};
        SDL_Surface* textSurface = TTF_RenderText_Solid(font, text.c_str(), textColor);
        if (textSurface == nullptr) {
            std::cerr << "Unable to render text surface! SDL_ttf Error: " << TTF_GetError() << std::endl;
            return;
        }

        SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
        if (textTexture == nullptr) {
            std::cerr << "Unable to create texture from rendered text! SDL Error: " << SDL_GetError() << std::endl;
            SDL_FreeSurface(textSurface);
            return;
        }

        SDL_Rect renderRect = {x, y, textSurface->w, textSurface->h};
        SDL_RenderCopy(renderer, textTexture, NULL, &renderRect);

        SDL_FreeSurface(textSurface);
        SDL_DestroyTexture(textTexture);
    }

    // Helper function for drawing circles since SDL2 doesn't have built-in circle drawing
    void filledCircleRGBA(SDL_Surface* surface, int cx, int cy, int radius, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
        for (int y = -radius; y <= radius; y++) {
            for (int x = -radius; x <= radius; x++) {
                if (x*x + y*y <= radius*radius) {
                    int drawX = cx + x;
                    int drawY = cy + y;
                    if (drawX >= 0 && drawX < surface->w && drawY >= 0 && drawY < surface->h) {
                        Uint32 pixel = SDL_MapRGBA(surface->format, r, g, b, a);
                        Uint32* target_pixel = (Uint32*)((Uint8*)surface->pixels + drawY * surface->pitch + drawX * 4);
                        *target_pixel = pixel;
                    }
                }
            }
        }
    }

    void cleanup() {
        if (font != nullptr) {
            TTF_CloseFont(font);
            font = nullptr;
        }

        if (playerTexture != nullptr) {
            SDL_DestroyTexture(playerTexture);
            playerTexture = nullptr;
        }

        if (obstacleTexture != nullptr) {
            SDL_DestroyTexture(obstacleTexture);
            obstacleTexture = nullptr;
        }

        if (backgroundTexture != nullptr) {
            SDL_DestroyTexture(backgroundTexture);
            backgroundTexture = nullptr;
        }

        if (cloudTexture != nullptr) {
            SDL_DestroyTexture(cloudTexture);
            cloudTexture = nullptr;
        }

        if (menuTexture != nullptr) {
            SDL_DestroyTexture(menuTexture);
            menuTexture = nullptr;
        }

        if (gameOverTexture != nullptr) {
            SDL_DestroyTexture(gameOverTexture);
            gameOverTexture = nullptr;
        }

        if (renderer != nullptr) {
            SDL_DestroyRenderer(renderer);
            renderer = nullptr;
        }

        if (window != nullptr) {
            SDL_DestroyWindow(window);
            window = nullptr;
        }

        TTF_Quit();
        IMG_Quit();
        SDL_Quit();
    }
};

int main(int argc, char* argv[]) {
    Game game;

    if (!game.init()) {
        std::cerr << "Failed to initialize game!" << std::endl;
        return -1;
    }

    game.run();

    return 0;
}
