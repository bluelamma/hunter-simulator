#pragma once

#include "game.hpp"

class Animation {
private:
    int frameWidth;
    int frameHeight;
    float holdTime;
    float timer;
    int currentFrame;
public: 
    Animation(int frameWidth, int frameHeight, float holdTime);

    void update(int row, int startFrame, int endFrame, float dt, sf::Sprite &sprite);
    void setFrameDuration(float newHoldTime);
};