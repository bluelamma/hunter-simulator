#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <fstream>
#include <sstream>

#include "game.hpp"
#include "entities.hpp"
#include "player.hpp"
#include "world_objects.hpp"

// Makes the map appear in the game // used by Game::loadLocation
class MapLoader {
private:
    static bool bearBossDefeated;    // Remembers if the boss is dead
    static Bear* activeBearBoss;     // Temporarily watches the bear while in the cave
public:
    static void loadOverworld(std::vector<std::unique_ptr<GameObject>> &gameObjects, Player *player, int tileSize, sf::Vector2f spawnPoint);
    static void loadCave(std::vector<std::unique_ptr<GameObject>> &gameObjects, Player *player, int tileSize, sf::Vector2f spawnPoint);
    static void loadHome(std::vector<std::unique_ptr<GameObject>> &gameObjects, Player *player, int tileSize, sf::Vector2f spawnPoint);

    static void saveCaveState();

    static void spawnHare(std::vector<std::unique_ptr<GameObject>> &gameObjects, Player *player, sf::FloatRect spawnArea, int entityCount);
    static void spawnBoar(std::vector<std::unique_ptr<GameObject>> &gameObjects, Player *player, sf::FloatRect spawnArea, int entityCount);
    static void spawnBear(std::vector<std::unique_ptr<GameObject>> &gameObjects, Player *player, sf::FloatRect spawnArea, int entityCount);
};


// Loads the map from a file
class TileMap : public GameObject {
private:
    std::vector<int> mapData;
    std::vector<int> solidTiles; // Collisions with stuff
    std::vector<sf::FloatRect> solidBoxes; // Collisions with objects
    std::vector<sf::FloatRect> spawnAreas;
    int mapWidth; // in tiles
    int mapHeight; // in tiles
    int tileSize;
    int tilesetColumns; // Number of columns in the png
    int waterTile;

    sf::VertexArray tile_vertices;

public:
    TileMap(const std::string &textureFile, const std::string &csvFile, int width, int height, int tSize, std::vector<int> solids, int waterTile);

    void addSolidBox(const sf::FloatRect& box);
    void addSpawnArea(const sf::FloatRect& box);

    void draw(sf::RenderWindow &window) override;
    bool isSolid(float pixelX, float pixelY, bool isProjectile) const; // for Checking if the player can walk on a specitic tile
    bool isSpawn(float pixelX, float pixelY) const; // Everything can walk over the areas but creatures can't spawn
};