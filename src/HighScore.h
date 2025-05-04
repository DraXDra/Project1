#ifndef HIGH_SCORE_H
#define HIGH_SCORE_H

class HighScore {
public:
    HighScore();
    void loadHighScores();
    void saveHighScores();
    int getHighScoreClassic() const;
    int getHighScoreSurvivalRush() const;
    void updateHighScoreClassic(int score);
    void updateHighScoreSurvivalRush(int score);

private:
    int highScoreClassic;
    int highScoreSurvivalRush;
};

#endif
