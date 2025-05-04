#include "GameObject.h"
#include "Constants.h"
#include <cstdlib>

GameObject spawnObject(SDL_Texture* obstacleTexture, float currentObjectSpeed) {
    GameObject obj;
    int side = rand() % 4;
    switch (side) {
        case 0: // Từ trên
            obj.x = rand() % (WINDOW_WIDTH - OBJECT_SIZE);
            obj.y = -OBJECT_SIZE;
            obj.dx = 0;
            obj.dy = currentObjectSpeed;
            break;
        case 1: // Từ dưới
            obj.x = rand() % (WINDOW_WIDTH - OBJECT_SIZE);
            obj.y = WINDOW_HEIGHT;
            obj.dx = 0;
            obj.dy = -currentObjectSpeed;
            break;
        case 2: // Từ trái
            obj.x = -OBJECT_SIZE;
            obj.y = rand() % (WINDOW_HEIGHT - OBJECT_SIZE);
            obj.dx = currentObjectSpeed;
            obj.dy = 0;
            break;
        case 3: // Từ phải
            obj.x = WINDOW_WIDTH;
            obj.y = rand() % (WINDOW_HEIGHT - OBJECT_SIZE);
            obj.dx = -currentObjectSpeed;
            obj.dy = 0;
            break;
    }
    obj.texture = obstacleTexture;
    return obj;
}

RainDrop spawnRainDrop() {
    RainDrop drop;
    drop.x = rand() % WINDOW_WIDTH;
    drop.y = -10;
    drop.speed = 5.0f + static_cast<float>(rand() % 5);
    drop.length = 10 + rand() % 10;
    return drop;
}

#endif
