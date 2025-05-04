#include "Game.h"
#include "Constants.h"
#include "Utils.h"
#include <iostream>
#include <cmath>

Game::Game()
    : window(nullptr), renderer(nullptr), playerTexture(nullptr), obstacleTexture(nullptr),
      backgroundTexture(nullptr), logoTexture(nullptr), bgMusic(nullptr), hitSound(nullptr), font(nullptr),
      playerX(WINDOW_WIDTH / 2.0f - PLAYER_WIDTH / 2.0f),
      playerY(WINDOW_HEIGHT / 2.0f - PLAYER_HEIGHT / 2.0f),
      targetX(playerX), targetY(playerY),
      running(true), lastSpawnTime(0), gameStartTime(0), lastFlashTime(0),
      score(0), currentObjectSpeed(INITIAL_OBJECT_SPEED),
      currentSpawnInterval(SPAWN_INTERVAL),
      state(GameState::MENU), menuSelection(0) {
    srand(time(nullptr));
}

Game::~Game() {
    if (playerTexture) SDL_DestroyTexture(playerTexture);
    if (obstacleTexture) SDL_DestroyTexture(obstacleTexture);
    if (backgroundTexture) SDL_DestroyTexture(backgroundTexture);
    if (logoTexture) SDL_DestroyTexture(logoTexture);
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

bool Game::initSDL() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        std::cerr << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
        return false;
    }
    if (IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG) {
        std::cerr << "Failed to initialize SDL_image: " << SDL_GetError() << std::endl;
        return false;
    }
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        std::cerr << "Failed to initialize SDL_mixer: " << Mix_GetError() << std::endl;
        return false;
    }
    if (TTF_Init() < 0) {
        std::cerr << "Failed to initialize SDL_ttf: " << TTF_GetError() << std::endl;
        return false;
    }
    window = SDL_CreateWindow("Dodge", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "Failed to create window: " << SDL_GetError() << std::endl;
        return false;
    }
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "Failed to create renderer: " << SDL_GetError() << std::endl;
        return false;
    }
    return true;
}

bool Game::loadAssets() {
    backgroundTexture = IMG_LoadTexture(renderer, "assets/background.png");
    if (!backgroundTexture) {
        std::cerr << "Failed to load background texture: " << SDL_GetError() << std::endl;
        return false;
    }
    playerTexture = IMG_LoadTexture(renderer, "assets/player.png");
    if (!playerTexture) {
        std::cerr << "Failed to load player texture: " << SDL_GetError() << std::endl;
        return false;
    }
    obstacleTexture = IMG_LoadTexture(renderer, "assets/obstacle.png");
    if (!obstacleTexture) {
        std::cerr << "Failed to load obstacle texture: " << SDL_GetError() << std::endl;
        return false;
    }
    logoTexture = IMG_LoadTexture(renderer, "assets/logo.png");
    if (!logoTexture) {
        std::cerr << "Failed to load logo texture: " << SDL_GetError() << std::endl;
        return false;
    }
    bgMusic = Mix_LoadMUS("assets/background_music.mp3");
    if (!bgMusic) {
        std::cerr << "Failed to load background music: " << Mix_GetError() << std::endl;
        return false;
    }
    hitSound = Mix_LoadWAV("assets/hit.mp3");
    if (!hitSound) {
        std::cerr << "Failed to load sound effect: " << Mix_GetError() << std::endl;
        return false;
    }
    font = TTF_OpenFont("assets/arial.ttf", 24);
    if (!font) {
        std::cerr << "Failed to load font: " << TTF_GetError() << std::endl;
        return false;
    }
    return true;
}

bool Game::init() {
    return initSDL() && loadAssets();
}

bool Game::checkCollision(float x1, float y1, int w1, int h1, float x2, float y2, int w2, int h2) {
    return x1 < x2 + w2 && x1 + w1 > x2 && y1 < y2 + h2 && y1 + h1 > y2;
}

float Game::distance(float x1, float y1, float x2, float y2) {
    return sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
}

void Game::movePlayerToTarget() {
    if (distance(playerX, playerY, targetX, targetY) > 5.0f) {
        float dx = targetX - playerX;
        float dy = targetY - playerY;
        float dist = distance(playerX, playerY, targetX, targetY);
        float effectiveSpeed = (weatherSystem.getCurrentWeather() == WeatherEffect::RAIN) ? RAIN_PLAYER_SPEED : PLAYER_SPEED;
        float moveX = (dx / dist) * effectiveSpeed;
        float moveY = (dy / dist) * effectiveSpeed;

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

void Game::saveGameState() {
    std::ofstream outFile("savegame.txt");
    if (!outFile) {
        std::cerr << "Failed to save game state!" << std::endl;
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

bool Game::loadGameState() {
    std::ifstream inFile("savegame.txt");
    if (!inFile) {
        std::cerr << "Save game file not found!" << std::endl;
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

void Game::updateScore() {
    Uint32 currentTime = SDL_GetTicks();
    score = ((currentTime - gameStartTime) / 1000) * 10; // 1 second = 10 points
}

void Game::updateObjectSpeed() {
    if (state != GameState::PLAYING_SURVIVAL) {
        Uint32 currentTime = SDL_GetTicks();
        Uint32 elapsedTime = currentTime - gameStartTime;
        currentObjectSpeed = INITIAL_OBJECT_SPEED + (elapsedTime / SPEED_INCREASE_INTERVAL) * SPEED_INCREMENT;
    }
}

void Game::renderMenu() {
    SDL_Rect bgRect = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
    SDL_RenderCopy(renderer, backgroundTexture, nullptr, &bgRect);

    renderText(renderer, font, "Dodge Game", 100, textColor);
    if (menuSelection == 0) {
        renderText(renderer, font, "High Score (Classic): " + std::to_string(highScore.getHighScoreClassic()), 150, textColor);
    } else if (menuSelection == 1) {
        renderText(renderer, font, "High Score (Survival Rush): " + std::to_string(highScore.getHighScoreSurvivalRush()), 150, textColor);
    } else {
        renderText(renderer, font, "High Score (Classic): " + std::to_string(highScore.getHighScoreClassic()), 150, textColor);
    }
    renderText(renderer, font, "Play (Classic)", 250, menuSelection == 0 ? highlightColor : textColor);
    renderText(renderer, font, "Survival Rush", 300, menuSelection == 1 ? highlightColor : textColor);
    renderText(renderer, font, "Load Game", 350, menuSelection == 2 ? highlightColor : textColor);
    renderText(renderer, font, "Exit", 400, menuSelection == 3 ? highlightColor : textColor);

    SDL_RenderPresent(renderer);
}

void Game::run() {
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            } else if (state == GameState::MENU) {
                if (event.type == SDL_KEYDOWN) {
                    if (event.key.keysym.sym == SDLK_UP) {
                        menuSelection = (menuSelection - 1 + 4) % 4;
                    } else if (event.key.keysym.sym == SDLK_DOWN) {
                        menuSelection = (menuSelection + 1) % 4;
                    } else if (event.key.keysym.sym == SDLK_RETURN) {
                        if (menuSelection == 0) {
                            state = GameState::PLAYING;
                            gameStartTime = SDL_GetTicks();
                            score = 0;
                            objects.clear();
                            currentObjectSpeed = INITIAL_OBJECT_SPEED;
                            currentSpawnInterval = SPAWN_INTERVAL;
                            lastFlashTime = gameStartTime - FLASH_COOLDOWN - 1;
                            Mix_PlayMusic(bgMusic, -1);
                        } else if (menuSelection == 1) {
                            state = GameState::PLAYING_SURVIVAL;
                            gameStartTime = SDL_GetTicks();
                            score = 0;
                            objects.clear();
                            currentObjectSpeed = INITIAL_OBJECT_SPEED;
                            currentSpawnInterval = SPAWN_INTERVAL_SURVIVAL;
                            lastFlashTime = gameStartTime - FLASH_COOLDOWN - 1;
                            Mix_PlayMusic(bgMusic, -1);
                        } else if (menuSelection == 2) {
                            if (loadGameState()) {
                                state = GameState::PLAYING;
                                Mix_PlayMusic(bgMusic, -1);
                            }
                        } else if (menuSelection == 3) {
                            running = false;
                        }
                    }
                }
            } else if (state == GameState::PLAYING || state == GameState::PLAYING_SURVIVAL) {
                if (event.type == SDL_MOUSEMOTION) {
                    targetX = event.motion.x - PLAYER_WIDTH / 2.0f;
                    targetY = event.motion.y - PLAYER_HEIGHT / 2.0f;
                } else if (event.type == SDL_KEYDOWN) {
                    if (event.key.keysym.sym == SDLK_f) {
                        Uint32 currentTime = SDL_GetTicks();
                        if (currentTime - lastFlashTime >= FLASH_COOLDOWN) {
                            float dx = targetX - playerX;
                            float dy = targetY - playerY;
                            float dist = distance(playerX, playerY, targetX, targetY);
                            if (dist > 0) {
                                float moveX = (dx / dist) * FLASH_DISTANCE;
                                float moveY = (dy / dist) * FLASH_DISTANCE;
                                playerX += moveX;
                                playerY += moveY;

                                if (playerX < 0) playerX = 0;
                                if (playerX > WINDOW_WIDTH - PLAYER_WIDTH) playerX = WINDOW_WIDTH - PLAYER_WIDTH;
                                if (playerY < 0) playerY = 0;
                                if (playerY > WINDOW_HEIGHT - PLAYER_HEIGHT) playerY = WINDOW_HEIGHT - PLAYER_HEIGHT;

                                lastFlashTime = currentTime;
                            }
                        }
                    } else if (event.key.keysym.sym == SDLK_s) {
                        if (state == GameState::PLAYING) {
                            saveGameState();
                            Mix_HaltMusic();
                            state = GameState::MENU;
                        }
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
        } else if (state == GameState::PLAYING || state == GameState::PLAYING_SURVIVAL) {
            movePlayerToTarget();
            weatherSystem.updateWeather(SDL_GetTicks(), state == GameState::PLAYING);

            Uint32 currentTime = SDL_GetTicks();
            if (state == GameState::PLAYING) {
                Uint32 elapsedTime = currentTime - gameStartTime;
                if (elapsedTime / 30000 > 0) {
                    int decreaseCount = elapsedTime / 30000;
                    currentSpawnInterval = std::max(SPAWN_INTERVAL - (decreaseCount * SPAWN_INTERVAL_DECREASE_RATE), SPAWN_INTERVAL_MIN);
                }
            }

            if (state == GameState::PLAYING_SURVIVAL) {
                Uint32 elapsedTime = currentTime - gameStartTime;
                if (elapsedTime >= SURVIVAL_RUSH_DURATION) {
                    Mix_HaltMusic();
                    updateScore();
                    std::cout << "Survival Rush ended. Final score: " << score << std::endl;
                    highScore.updateHighScoreSurvivalRush(score);
                    state = GameState::GAME_OVER;
                    continue;
                }
            }

            if (currentTime - lastSpawnTime > currentSpawnInterval) {
                updateObjectSpeed();
                objects.push_back(spawnObject(obstacleTexture, currentObjectSpeed));
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
                    updateScore();
                    std::cout << "Collision detected. Final score: " << score << std::endl;
                    if (state == GameState::PLAYING) {
                        highScore.updateHighScoreClassic(score);
                    } else {
                        highScore.updateHighScoreSurvivalRush(score);
                    }
                    state = GameState::GAME_OVER;
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

            weatherSystem.renderWeather(renderer);

            renderText(renderer, font, "Score: " + std::to_string(score), 10, textColor);
            if (state == GameState::PLAYING) {
                renderText(renderer, font, "Press S to save game", 40, textColor);
            } else if (state == GameState::PLAYING_SURVIVAL) {
                int timeLeft = (SURVIVAL_RUSH_DURATION - (currentTime - gameStartTime)) / 1000;
                renderText(renderer, font, "Time Left: " + std::to_string(timeLeft) + "s", 40, textColor);
            }

            std::string weatherText = "Weather: ";
            if (weatherSystem.getCurrentWeather() == WeatherEffect::RAIN) {
                weatherText += "Rain";
            } else if (weatherSystem.getCurrentWeather() == WeatherEffect::FOG) {
                weatherText += "Fog";
            } else {
                weatherText += "Clear";
            }
            renderText(renderer, font, weatherText, 70, textColor);

            SDL_Rect logoRect = {(WINDOW_WIDTH / 2) - 25, WINDOW_HEIGHT - 80, 50, 50};
            SDL_RenderCopy(renderer, logoTexture, nullptr, &logoRect);

            Uint32 timeSinceLastFlash = currentTime - lastFlashTime;
            if (timeSinceLastFlash < FLASH_COOLDOWN) {
                int cooldownTimeRemaining = (FLASH_COOLDOWN - timeSinceLastFlash) / 1000;
                std::string cooldownText = std::to_string(cooldownTimeRemaining) + "s";
                renderTextCentered(renderer, font, cooldownText, WINDOW_WIDTH / 2, WINDOW_HEIGHT - 110, textColor);
            } else if (timeSinceLastFlash < FLASH_COOLDOWN + READY_DISPLAY_TIME) {
                renderTextCentered(renderer, font, "Ready", WINDOW_WIDTH / 2, WINDOW_HEIGHT - 110, textColor);
            }

            SDL_RenderPresent(renderer);
        } else if (state == GameState::GAME_OVER) {
            SDL_Rect bgRect = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
            SDL_RenderCopy(renderer, backgroundTexture, nullptr, &bgRect);

            renderText(renderer, font, "Game Over!", (WINDOW_HEIGHT / 2) - 50, textColor);
            renderText(renderer, font, "Score: " + std::to_string(score), WINDOW_HEIGHT / 2, textColor);
            renderText(renderer, font, "Press Enter to return to menu", (WINDOW_HEIGHT / 2) + 50, textColor);

            SDL_RenderPresent(renderer);
        }

        SDL_Delay(16);
    }
}
