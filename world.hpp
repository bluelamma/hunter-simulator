#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <fstream>
#include <sstream>

#include "game.hpp"
#include "entities.hpp"


// Objects letting player to switch location like the cave
class SwitchLocation : public GameObject {
private:
    Player* playerTarget;
    bool playerEntered;
    LocationID destinationLevel;
    sf::Vector2f spawnPoint; // of the player
    sf::RectangleShape entranceHitbox;
    sf::Vector2f hitboxOffset;

public:
    // Takes the texture path, destination, spawn coordinates
    SwitchLocation(float startX, float startY, const std::string &texturePath, Player *player, 
        LocationID destination, sf::Vector2f spawn, sf::Vector2f entrance_dimensions, sf::Vector2f hitboxOffset);
    
    void draw(sf::RenderWindow &window) override;
    void update(float dt, sf::RenderWindow &window) override;
    
    bool hasPlayerEntered() const;
    LocationID getDestination() const;
    sf::Vector2f getSpawnPoint() const;
};


// Makes the map appear in the game // used by Game::loadLocation
class MapLoader {
public:
    static void loadOverworld(std::vector<std::unique_ptr<GameObject>>& gameObjects, Player*& trackedPlayer, int tileSize, sf::Vector2f spawnPoint);
    static void loadCave(std::vector<std::unique_ptr<GameObject>>& gameObjects, Player*& trackedPlayer, int tileSize, sf::Vector2f spawnPoint);
    static void loadHome(std::vector<std::unique_ptr<GameObject>>& gameObjects, Player*& trackedPlayer, int tileSize, sf::Vector2f spawnPoint);
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