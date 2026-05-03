#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <fstream>
#include <sstream>

#include "game.hpp"

class TileMap : public GameObject {
private:
    std::vector<int> mapData;
    int mapWidth; // in tiles
    int mapHeight; // in tiles
    int tileSize;
    int tilesetColumns; // Number of columns in the png

    sf::VertexArray tile_vertices;

public:
    TileMap(const std::string &textureFile, const std::string &csvFile, int width, int height, int tSize);
    void draw(sf::RenderWindow &window) override;
    bool isSolid(float pixelX, float pixelY) const; // for Checking if the player can walk on a specitic tile
};