#ifndef GAME_OBJECT_H
#define GAME_OBJECT_H

#include <SDL.h>

struct GameObject {
    float x, y;
    float dx, dy;
    SDL_Texture* texture;
};

struct RainDrop {
    float x, y;
    float speed;
    int length;
};

#endif
