#ifndef WEATHER_SYSTEM_H
#define WEATHER_SYSTEM_H

#include "GameObject.h"
#include <vector>
#include <SDL.h>

enum class WeatherEffect { NONE, RAIN, FOG };

class WeatherSystem {
public:
    WeatherSystem();
    void updateWeather(Uint32 currentTime, bool isClassicMode);
    void renderWeather(SDL_Renderer* renderer);
    WeatherEffect getCurrentWeather() const;

private:
    WeatherEffect currentWeather;
    Uint32 weatherStartTime;
    Uint32 weatherDuration;
    Uint32 lastWeatherChange;
    std::vector<RainDrop> rainDrops;
};

#endif
