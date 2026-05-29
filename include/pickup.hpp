#pragma once

#include "game.hpp"
#include "player.hpp"

class Pickup : public GameObject {
protected:
    Player *player;
public:
    Pickup(float startX, float startY, Player *player);
};

class RawMeat : public Pickup {
private:
    float healingValue;
    float sellingValue;
    float multiplier;
public:
    RawMeat(float startX, float startY, Player *player, float multiplier);
    
    void draw(sf::RenderWindow &window) override;
    void update(float dt, sf::RenderWindow &window) override;
};