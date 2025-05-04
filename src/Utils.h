#ifndef UTILS_H
#define UTILS_H

#include <SDL.h>
#include <SDL_ttf.h>
#include <string>

void renderText(SDL_Renderer* renderer, TTF_Font* font, const std::string& text, int y, SDL_Color color);
void renderTextCentered(SDL_Renderer* renderer, TTF_Font* font, const std::string& text, int x, int y, SDL_Color color);

#endif
