#include "WeatherSystem.h"
#include "Constants.h"
#include <cstdlib>

WeatherSystem::WeatherSystem()
    : currentWeather(WeatherEffect::NONE), weatherStartTime(0), weatherDuration(0), lastWeatherChange(0) {}

void WeatherSystem::updateWeather(Uint32 currentTime, bool isClassicMode) {
    Uint32 elapsedTime = currentTime - lastWeatherChange;
    int weatherInterval = isClassicMode ? CLASSIC_WEATHER_INTERVAL : SURVIVAL_WEATHER_INTERVAL;

    if (elapsedTime >= weatherInterval) {
        if (currentWeather != WeatherEffect::NONE && (currentTime - weatherStartTime) >= weatherDuration) {
            currentWeather = WeatherEffect::NONE;
            rainDrops.clear();
            lastWeatherChange = currentTime;
            return;
        }

        if (currentWeather == WeatherEffect::NONE) {
            int weatherType = rand() % 2;
            currentWeather = (weatherType == 0) ? WeatherEffect::RAIN : WeatherEffect::FOG;
            weatherStartTime = currentTime;
            weatherDuration = isClassicMode ? CLASSIC_WEATHER_DURATION : SURVIVAL_WEATHER_DURATION;
            lastWeatherChange = currentTime;
        }
    }

    if (currentWeather == WeatherEffect::RAIN && rand() % 10 == 0) {
        rainDrops.push_back(spawnRainDrop());
    }

    for (auto it = rainDrops.begin(); it != rainDrops.end();) {
        it->y += it->speed;
        if (it->y > WINDOW_HEIGHT) {
            it = rainDrops.erase(it);
        } else {
            ++it;
        }
    }
}

void WeatherSystem::renderWeather(SDL_Renderer* renderer) {
    if (currentWeather == WeatherEffect::FOG) {
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 100);
        SDL_Rect fogRect = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
        SDL_RenderFillRect(renderer, &fogRect);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
    } else if (currentWeather == WeatherEffect::RAIN) {
        SDL_SetRenderDrawColor(renderer, 0, 0, 255, 150);
        for (const auto& drop : rainDrops) {
            SDL_RenderDrawLine(renderer, drop.x, drop.y, drop.x, drop.y + drop.length);
        }
    }
}

WeatherEffect WeatherSystem::getCurrentWeather() const {
    return currentWeather;
}
