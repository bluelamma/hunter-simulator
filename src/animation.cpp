#include "../include/animation.hpp"

// -----------------------
// ------ Animation ------
// -----------------------
Animation::Animation(int frameWidth, int frameHeight, float holdTime) 
    : frameWidth(frameWidth), frameHeight(frameHeight), holdTime(holdTime), timer(0.0f), currentFrame(0) {}

void Animation::update(int row, int startFrame, int endFrame, float dt, sf::Sprite &sprite) {
    if (currentFrame < startFrame || currentFrame > endFrame) {
        currentFrame = startFrame;
        timer = 0.0f;
    }

    timer += dt;

    if (timer >= holdTime) {
        timer -= holdTime;
        currentFrame++;

        if (currentFrame > endFrame) {
            currentFrame = startFrame;
        }
    }

    int left = frameWidth * currentFrame;
    int top = frameHeight * row;

    sprite.setTextureRect(sf::IntRect({left, top}, {frameWidth, frameHeight}));
}

void Animation::setFrameDuration(float newHoldTime) {
    holdTime = newHoldTime;
}