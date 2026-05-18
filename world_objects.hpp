#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <fstream>
#include <sstream>

#include "game.hpp"
#include "entities.hpp"
#include "player.hpp"

// Objects letting player to switch location like the cave or home
class SwitchLocation : public GameObject {
private:
    Player* playerTarget;
    bool playerEntered;
    LocationID destinationLevel;
    sf::Vector2f spawnPoint; // of the player
    sf::RectangleShape entranceHitbox;
    sf::Vector2f hitboxOffset;

public:
    SwitchLocation(float startX, float startY, const std::string &texturePath, Player *player, 
        LocationID destination, sf::Vector2f spawn, sf::Vector2f entrance_dimensions, sf::Vector2f hitboxOffset);
    
    void draw(sf::RenderWindow &window) override;
    void update(float dt, sf::RenderWindow &window) override;
    
    bool hasPlayerEntered() const;
    LocationID getDestination() const;
    sf::Vector2f getSpawnPoint() const;
};

// Present in home location
class UpgradeStation : public GameObject {
private:
    Player* player;
    sf::RectangleShape hitbox;
    bool isPlayerNear;
    float upgradeCooldown;
    
    sf::Font font;
    sf::Text promptText;

public:
    UpgradeStation(float startX, float startY, float width, float height, Player *player);
    void draw(sf::RenderWindow &window) override;
    void update(float dt, sf::RenderWindow &window) override;
};

class Stall : public GameObject {
private:
    Player* player;
    sf::RectangleShape hitbox;
    bool isPlayerNear;
    float buyingCooldown;
    
    sf::Font font;
    sf::Text promptText;

public:
    Stall(float startX, float startY, float width, float height, const std::string &texturePath, Player *player);
    void draw(sf::RenderWindow &window) override;
    void update(float dt, sf::RenderWindow &window) override;
};