#include "world.hpp"

TileMap::TileMap(const std::string& textureFile, const std::string& csvFile, int width, int height, int tSize) 
    : GameObject(0, 0), mapWidth(width), mapHeight(height), tileSize(tSize) {
    // Loads the texture
    if (!texture.loadFromFile(textureFile)) {
        std::cerr << "Couldn't load the tileset\n";
    }

    tilesetColumns = texture.getSize().x / tileSize;

    // Parses the CSV file into mapData
    std::ifstream file(csvFile);
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            std::stringstream ss(line);
            std::string cell;
            while (std::getline(ss, cell, ',')) {
                if (!cell.empty()) {
                    mapData.push_back(std::stoi(cell));
                }
            }
        }
        file.close();
    } else {
        std::cerr << "ERROR: Could not open file: " << csvFile << "\n";
    }

    int expectedSize = mapWidth * mapHeight;
    if (mapData.size() != expectedSize) {
        std::cerr << "WARNING: Expected " << expectedSize << " tiles, but loaded " << mapData.size() << "!\n";
    }

    // Populates the Vertex Array
    // 1 Square Tile = 2 Triangles = 6 Vertices
    tile_vertices.setPrimitiveType(sf::PrimitiveType::Triangles);
    tile_vertices.resize(mapWidth * mapHeight * 6);

    for (int i = 0; i < mapWidth; i++) {
        for (int j = 0; j < mapHeight; j++) {
            
            int tileNumber = mapData[i + j * mapWidth];

            if (tileNumber < 0) continue;

            int tu = tileNumber % tilesetColumns; // column
            int tv = tileNumber / tilesetColumns; // row

            // A pointer to the 6 vertices needed for this specific tile
            sf::Vertex* triangles = &tile_vertices[(i + j * mapWidth) * 6];

            // Defines the 6 corners of the two triangles on the SCREEN
            // Triangle 1 (Top-Left, Top-Right, Bottom-Left)
            triangles[0].position = sf::Vector2f(i * tileSize, j * tileSize);
            triangles[1].position = sf::Vector2f((i + 1) * tileSize, j * tileSize);
            triangles[2].position = sf::Vector2f(i * tileSize, (j + 1) * tileSize);
            
            // Triangle 2 (Bottom-Left, Top-Right, Bottom-Right)
            triangles[3].position = sf::Vector2f(i * tileSize, (j + 1) * tileSize);
            triangles[4].position = sf::Vector2f((i + 1) * tileSize, j * tileSize);
            triangles[5].position = sf::Vector2f((i + 1) * tileSize, (j + 1) * tileSize);

            // Defines the 6 corners of the two triangles on the texture
            // Triangle 1
            triangles[0].texCoords = sf::Vector2f(tu * tileSize, tv * tileSize);
            triangles[1].texCoords = sf::Vector2f((tu + 1) * tileSize, tv * tileSize);
            triangles[2].texCoords = sf::Vector2f(tu * tileSize, (tv + 1) * tileSize);
            
            // Triangle 2
            triangles[3].texCoords = sf::Vector2f(tu * tileSize, (tv + 1) * tileSize);
            triangles[4].texCoords = sf::Vector2f((tu + 1) * tileSize, tv * tileSize);
            triangles[5].texCoords = sf::Vector2f((tu + 1) * tileSize, (tv + 1) * tileSize);
        }
    }
}

void TileMap::draw(sf::RenderWindow &window) {
    // Lets SFML know what texture to wrap around defined triangles
    sf::RenderStates states;

    states.texture = &texture; 

    // Draws all the vertices
    window.draw(tile_vertices, states);
}

bool TileMap::isSolid(float pixelX, float pixelY) const {
    // Pixel coordinates -> Tile Grid coordinates
    int tileX = static_cast<int>(pixelX / tileSize);
    int tileY = static_cast<int>(pixelY / tileSize);

    // Finds out what tile ID is at this location
    int tileID = mapData[tileX + tileY * mapWidth];

    if (tileID == 3) { 
        return true; // Block movement
    }

    return false; // Movement allowed
}