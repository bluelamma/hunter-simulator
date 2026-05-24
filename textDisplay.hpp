#pragma once

#include "game.hpp"
#include "entities.hpp"
#include "player.hpp"
#include "world_objects.hpp"


class TextDisplay {
private:
    Player *player;

    sf::Font font;
    sf::Text pauseText;
    sf::Text restartText;
    sf::Text cashText;
    sf::RectangleShape pauseOverlay;
    sf::RectangleShape restartOverlay;
public:
    TextDisplay(Player *player);

    void drawHud(sf::RenderWindow &window);
    void drawPause(sf::RenderWindow &window, sf::Vector2f cameraCenter, sf::Vector2f cameraSize);
    void drawRestart(sf::RenderWindow &window, sf::Vector2f cameraCenter, sf::Vector2f cameraSize);
};