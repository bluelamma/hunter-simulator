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
    Player* playerTarget;
    sf::RectangleShape hitbox;
    bool isPlayerNear;
    float upgradeCooldown;
    
    float costDamage;
    float costReload;
    float costVelocity;
    
    sf::Font font;
    sf::Text promptText;

public:
    UpgradeStation(float startX, float startY, float width, float height, Player *player);
    void draw(sf::RenderWindow &window) override;
    void update(float dt, sf::RenderWindow &window) override;
};


// Makes the map appear in the game // used by Game::loadLocation
class MapLoader {
public:
    static void loadOverworld(std::vector<std::unique_ptr<GameObject>> &gameObjects, Player *player, int tileSize, sf::Vector2f spawnPoint);
    static void loadCave(std::vector<std::unique_ptr<GameObject>> &gameObjects, Player *player, int tileSize, sf::Vector2f spawnPoint);
    static void loadHome(std::vector<std::unique_ptr<GameObject>> &gameObjects, Player *player, int tileSize, sf::Vector2f spawnPoint);

    static void spawnHare(std::vector<std::unique_ptr<GameObject>> &gameObjects, Player *player, sf::FloatRect spawnArea, int entityCount);
};


// Loads the map from a file
class TileMap : public GameObject {
private:
    std::vector<int> mapData;
    std::vector<int> solidTiles;
    int mapWidth; // in tiles
    int mapHeight; // in tiles
    int tileSize;
    int tilesetColumns; // Number of columns in the png

    sf::VertexArray tile_vertices;

public:
    TileMap(const std::string &textureFile, const std::string &csvFile, int width, int height, int tSize, std::vector<int> solids);
    void draw(sf::RenderWindow &window) override;
    bool isSolid(float pixelX, float pixelY) const; // for Checking if the player can walk on a specitic tile
};