#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_image.h>
#include <iostream>
#include <vector>
#include <random>
#include <algorithm>

// Constants
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const int GROUND_HEIGHT = 500;
const int PLAYER_WIDTH = 50;
const int PLAYER_HEIGHT = 80;
const int OBSTACLE_WIDTH = 40;
const int OBSTACLE_HEIGHT = 80;
const int JUMP_VELOCITY = -18;
const float GRAVITY = 0.8f;
const int FPS = 60;
const int FRAME_DELAY = 1000 / FPS;

// Game states
enum GameState {
    RUNNING,
    GAME_OVER,
    PAUSED
};

// Player class
class Player {
public:
    float x, y, velocity_y;
    bool is_jumping;
    SDL_Rect hitbox;

    Player() : x(100), y(GROUND_HEIGHT - PLAYER_HEIGHT), velocity_y(0), is_jumping(false) {
        hitbox = { (int)x, (int)y, PLAYER_WIDTH, PLAYER_HEIGHT };
    }

    void update() {
        velocity_y += GRAVITY;
        y += velocity_y;
        if (y >= GROUND_HEIGHT - PLAYER_HEIGHT) {
            y = GROUND_HEIGHT - PLAYER_HEIGHT;
            velocity_y = 0;
            is_jumping = false;
        }
        hitbox.x = (int)x;
        hitbox.y = (int)y;
    }

    void jump() {
        if (!is_jumping) {
            velocity_y = JUMP_VELOCITY;
            is_jumping = true;
        }
    }
};

// Obstacle class
class Obstacle {
public:
    float x, y, speed;
    SDL_Rect hitbox;

    Obstacle(float start_x, float obstacle_speed) : x(start_x), y(GROUND_HEIGHT - OBSTACLE_HEIGHT), speed(obstacle_speed) {
        hitbox = { (int)x, (int)y, OBSTACLE_WIDTH, OBSTACLE_HEIGHT };
    }

    void update() {
        x -= speed;
        hitbox.x = (int)x;
    }

    bool isOffScreen() { return x + OBSTACLE_WIDTH < 0; }
};

// Game class
class EndlessRunnerGame {
private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    Player player;
    std::vector<Obstacle> obstacles;
    GameState game_state;
    int score;
    float obstacle_speed;
    float spawn_timer;
    std::mt19937 rng;

public:
    EndlessRunnerGame() : window(nullptr), renderer(nullptr), game_state(RUNNING), score(0), obstacle_speed(5.0f), spawn_timer(2.0f) {
        std::random_device rd;
        rng = std::mt19937(rd());
    }

    ~EndlessRunnerGame() { cleanup(); }

    bool initialize() {
        if (SDL_Init(SDL_INIT_VIDEO) < 0) return false;
        window = SDL_CreateWindow("Endless Runner", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
        if (!window) return false;
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        return renderer != nullptr;
    }

    void run() {
        bool quit = false;
        SDL_Event e;
        Uint32 frame_start;
        int frame_time;

        while (!quit) {
            frame_start = SDL_GetTicks();
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_QUIT) quit = true;
                else if (e.type == SDL_KEYDOWN) handleKeyDown(e.key.keysym.sym);
            }
            if (game_state == RUNNING) update();
            render();
            frame_time = SDL_GetTicks() - frame_start;
            if (FRAME_DELAY > frame_time) SDL_Delay(FRAME_DELAY - frame_time);
        }
    }

private:
    void handleKeyDown(SDL_Keycode key) {
        if (key == SDLK_SPACE && game_state == RUNNING) player.jump();
        else if (key == SDLK_p) game_state = (game_state == RUNNING) ? PAUSED : RUNNING;
        else if (key == SDLK_r) restartGame();
    }

    void update() {
        player.update();
        for (auto it = obstacles.begin(); it != obstacles.end();) {
            it->update();
            if (SDL_HasIntersection(&player.hitbox, &it->hitbox)) game_state = GAME_OVER;
            if (it->isOffScreen()) it = obstacles.erase(it);
            else ++it;
        }
        spawn_timer -= 1.0f / FPS;
        if (spawn_timer <= 0) {
            obstacles.emplace_back(SCREEN_WIDTH, obstacle_speed);
            spawn_timer = 2.0f;
        }
    }

    void render() {
        SDL_SetRenderDrawColor(renderer, 135, 206, 235, 255);
        SDL_RenderClear(renderer);
        SDL_SetRenderDrawColor(renderer, 120, 80, 40, 255);
        SDL_Rect ground = { 0, GROUND_HEIGHT, SCREEN_WIDTH, SCREEN_HEIGHT - GROUND_HEIGHT };
        SDL_RenderFillRect(renderer, &ground);
        SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
        SDL_RenderFillRect(renderer, &player.hitbox);
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        for (auto& obs : obstacles) SDL_RenderFillRect(renderer, &obs.hitbox);
        SDL_RenderPresent(renderer);
    }

    void restartGame() {
        game_state = RUNNING;
        player = Player();
        obstacles.clear();
        score = 0;
        obstacle_speed = 5.0f;
        spawn_timer = 2.0f;
    }

    void cleanup() {
        if (renderer) SDL_DestroyRenderer(renderer);
        if (window) SDL_DestroyWindow(window);
        SDL_Quit();
    }
};

int main() {
    EndlessRunnerGame game;
    if (!game.initialize()) return -1;
    game.run();
    return 0;
}
