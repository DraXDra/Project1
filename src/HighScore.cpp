#include "HighScore.h"
#include <fstream>
#include <iostream>

HighScore::HighScore() : highScoreClassic(0), highScoreSurvivalRush(0) {}

void HighScore::loadHighScores() {
    std::ifstream inFileClassic("highscore_classic.txt");
    if (!inFileClassic) {
        highScoreClassic = 0;
    } else {
        inFileClassic >> highScoreClassic;
        inFileClassic.close();
    }

    std::ifstream inFileSurvival("highscore_survivalrush.txt");
    if (!inFileSurvival) {
        highScoreSurvivalRush = 0;
    } else {
        inFileSurvival >> highScoreSurvivalRush;
        inFileSurvival.close();
    }
}

void HighScore::saveHighScores() {
    std::ofstream outFileClassic("highscore_classic.txt");
    if (!outFileClassic) {
        std::cerr << "Failed to save high score (Classic)!" << std::endl;
        return;
    }
    outFileClassic << highScoreClassic;
    outFileClassic.close();

    std::ofstream outFileSurvival("highscore_survivalrush.txt");
    if (!outFileSurvival) {
        std::cerr << "Failed to save high score (Survival Rush)!" << std::endl;
        return;
    }
    outFileSurvival << highScoreSurvivalRush;
    outFileSurvival.close();
}

int HighScore::getHighScoreClassic() const {
    return highScoreClassic;
}

int HighScore::getHighScoreSurvivalRush() const {
    return highScoreSurvivalRush;
}

void HighScore::updateHighScoreClassic(int score) {
    if (score > highScoreClassic) {
        highScoreClassic = score;
        saveHighScores();
    }
}

void HighScore::updateHighScoreSurvivalRush(int score) {
    if (score > highScoreSurvivalRush) {
        highScoreSurvivalRush = score;
        saveHighScores();
    }
}
