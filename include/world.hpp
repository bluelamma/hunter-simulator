#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <fstream>
#include <sstream>
#include <unordered_map>

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
    // Adjustable in settings
    static float entityMultiplier;
    static float difficultyMultiplier;

    // Maps
    static void loadOverworld(std::vector<std::unique_ptr<GameObject>> &gameObjects, Player *player, int tileSize, sf::Vector2f spawnPoint);
    static void loadCave(std::vector<std::unique_ptr<GameObject>> &gameObjects, Player *player, int tileSize, sf::Vector2f spawnPoint);
    static void loadHome(std::vector<std::unique_ptr<GameObject>> &gameObjects, Player *player, int tileSize, sf::Vector2f spawnPoint);

    static void saveCaveState();

    // Spawn things
    static void spawnHare(std::vector<std::unique_ptr<GameObject>> &gameObjects, Player *player, sf::FloatRect spawnArea, int entityCount);
    static void spawnBoar(std::vector<std::unique_ptr<GameObject>> &gameObjects, Player *player, sf::FloatRect spawnArea, int entityCount, int variant);
    static void spawnBear(std::vector<std::unique_ptr<GameObject>> &gameObjects, Player *player, sf::FloatRect spawnArea, int entityCount);

    static void spawnScenery(std::vector<std::unique_ptr<GameObject>> &gameObjects, sf::FloatRect spawnArea, int entityCount, const std::vector<std::string> &texturePaths);
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
    std::vector<int> noSceneryTiles;

    sf::VertexArray tile_vertices;

public:
    TileMap(const std::string &textureFile, const std::string &csvFile, int width, int height, int tSize, std::vector<int> solids, int waterTile, std::vector<int> noSceneryTiles);

    void addSolidBox(const sf::FloatRect& box);
    void addSpawnArea(const sf::FloatRect& box);

    void draw(sf::RenderWindow &window) override;
    bool isSolid(float pixelX, float pixelY, bool isProjectile, bool isScenery) const; // for Checking if the player can walk on a specitic tile
    bool isSpawn(float pixelX, float pixelY) const; // Everything can walk over the areas but creatures can't spawn
};

// Bushes, grass
class Scenery : public GameObject {
private:
    // Stores textures
    static std::unordered_map<std::string, sf::Texture> textureCache;

public:
    Scenery(float startX, float startY, const std::string &texturePath);
    
    void draw(sf::RenderWindow &window) override;
    void update(float dt, sf::RenderWindow &window) override;
};