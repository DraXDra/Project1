#ifndef CONSTANTS_H
#define CONSTANTS_H

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const int PLAYER_WIDTH = 50;
const int PLAYER_HEIGHT = 50;
const int OBJECT_SIZE = 30;
const float PLAYER_SPEED = 5.0f;
const float RAIN_PLAYER_SPEED = PLAYER_SPEED * 0.8f;
const float INITIAL_OBJECT_SPEED = 3.0f;
const float SPEED_INCREMENT = 0.5f;
const int SPEED_INCREASE_INTERVAL = 10000;
const int SPAWN_INTERVAL = 500;
const int SPAWN_INTERVAL_SURVIVAL = 200;
const int SPAWN_INTERVAL_DECREASE_RATE = 50;
const int SPAWN_INTERVAL_MIN = 100;
const int FLASH_COOLDOWN = 15000;
const int FLASH_DISTANCE = 200;
const int READY_DISPLAY_TIME = 15000;
const int SURVIVAL_RUSH_DURATION = 60000;
const int CLASSIC_WEATHER_INTERVAL = 30000;
const int CLASSIC_WEATHER_DURATION = 10000;
const int SURVIVAL_WEATHER_INTERVAL = 10000;
const int SURVIVAL_WEATHER_DURATION = 5000;

#endif
