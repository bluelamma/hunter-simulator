#include "world.hpp"

// ----------------------
// --- SwitchLocation ---
// ----------------------
SwitchLocation::SwitchLocation(float startX, float startY, const std::string& texturePath, Player *player, 
    LocationID destination, sf::Vector2f spawn, sf::Vector2f entrance_dimensions, sf::Vector2f hitboxOffset) 
    : GameObject(startX, startY), playerTarget(player), playerEntered(false), destinationLevel(destination), spawnPoint(spawn) {
    
    // Texture
    if (!texture.loadFromFile(texturePath)) {
        std::cerr << "Couldn't load texture: " << texturePath << "\n";
    } else {
        sprite.setTexture(texture, true); 
    }

    // Hitbox
    entranceHitbox.setSize(entrance_dimensions);
    entranceHitbox.setPosition(sf::Vector2f(startX + hitboxOffset.x, startY + hitboxOffset.y));
    entranceHitbox.setFillColor(sf::Color(255, 0, 0, 100)); 
}

void SwitchLocation::draw(sf::RenderWindow &window) {
    window.draw(sprite);
    window.draw(entranceHitbox); // uncomment for easier checking if the thing aligns properly
}

void SwitchLocation::update(float dt, sf::RenderWindow &window) {
    sf::Vector2f locPos = sprite.getPosition();
    sf::Vector2f playerPos = playerTarget->getPosition();

    // Player is scaled x2
    float playerCenterX = playerPos.x + 31.0f;
    float playerCenterY = playerPos.y + 32.0f;

    // Checks if player is inside the hitbox
    bool isInside = entranceHitbox.getGlobalBounds().contains({playerCenterX, playerCenterY});

    // If inside the hitbox and interacting
    if (isInside && sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::E)) {
        playerEntered = true;
    }
}

bool SwitchLocation::hasPlayerEntered() const {
    return playerEntered;
}

LocationID SwitchLocation::getDestination() const {
    return destinationLevel;
}

sf::Vector2f SwitchLocation::getSpawnPoint() const {
    return spawnPoint;
}


// ----------------------
// ------ MapLoader -----
// ----------------------

// Overworld
void MapLoader::loadOverworld(std::vector<std::unique_ptr<GameObject>> &gameObjects, Player *player, int tileSize, sf::Vector2f spawnPoint) {

    // Map
    auto map = std::make_unique<TileMap>("maps/tiles_overworld.png", "maps/overworld.csv", 300, 150, tileSize, std::vector<int>{3});
    GameObject::world = map.get(); 
    gameObjects.emplace_back(std::move(map));

    // Objects
    gameObjects.emplace_back(std::make_unique<SwitchLocation>(
        tileSize * 253, tileSize * 125, // Spawn point of the object
        "objects/cave.png", 
        player, 
        LocationID::Cave, 
        sf::Vector2f({tileSize * 8.0f, tileSize * 38.0f}), // Spawn point of the player after interaction
        sf::Vector2f({220.0f, 130.0f}), // Dimensions of the entrance 
        sf::Vector2f({168.0f, 128.0f}) // Offset of the entrance 
    ));

    auto home = std::make_unique<SwitchLocation>(        
        tileSize * 258, tileSize * 125, // Spawn point of the object
        "objects/home.png", 
        player, 
        LocationID::Home, 
        sf::Vector2f({tileSize * 4.0f, tileSize * 8.0f}), // Spawn point of the player after interaction
        sf::Vector2f({70.0f, 80.0f}), // Dimensions of the entrance 
        sf::Vector2f({158.0f, 310.0f}) // Offset of the entrance );
    );
    home->setSpriteScale({6.0f, 6.0f});
    gameObjects.emplace_back(std::move(home));

    // Entities
    // sf::FloatRect({left_x, top_y}, {width, height})
    int numberOfEntities1 = rand() % 21 + 10; // 10 - 30;
    sf::FloatRect hareSpawnpoint1({tileSize * 200.0f, tileSize * 20.0f}, {tileSize * 100.0f, tileSize * 75.0f});
    spawnHare(gameObjects, player, hareSpawnpoint1, numberOfEntities1);

    int numberOfEntities2 = rand() % 11 + 20; // 20 - 30;
    sf::FloatRect hareSpawnpoint2({tileSize * 15.0f, tileSize * 20.0f}, {tileSize * 150.0f, tileSize * 50.0f});
    spawnHare(gameObjects, player, hareSpawnpoint2, numberOfEntities2);
}


// Cave
void MapLoader::loadCave(std::vector<std::unique_ptr<GameObject>> &gameObjects, Player *player, int tileSize, sf::Vector2f spawnPoint) {

    // Loads the 20x40 Cave map
    auto map = std::make_unique<TileMap>("maps/tiles_cave.png", "maps/cave.csv", 20, 40, tileSize, std::vector<int>{1});
    GameObject::world = map.get(); 
    gameObjects.emplace_back(std::move(map));

    // Cave exit (to go back to overworld)
    gameObjects.emplace_back(std::make_unique<SwitchLocation>(
        tileSize * 8, tileSize * 39, // Spawn point of the object
        "objects/cave_exit.png",
        player, 
        LocationID::Overworld, 
        sf::Vector2f({tileSize * 253.0f, tileSize * 126.0f}), // Spawn point of the player after interaction
        sf::Vector2f({tileSize * 3.0f + 20.0f, tileSize * 1.0f}), // Dimensions of the entrance 
        sf::Vector2f({-20.0f, -10.0f}) // Offset of the entrance 
    ));
}


// Home
void MapLoader::loadHome(std::vector<std::unique_ptr<GameObject>> &gameObjects, Player *player, int tileSize, sf::Vector2f spawnPoint) {

    // Loads the 9x10 Home map
    auto map = std::make_unique<TileMap>("maps/tiles_home.png", "maps/home.csv", 9, 10, tileSize, std::vector<int>{1});
    GameObject::world = map.get(); 
    gameObjects.emplace_back(std::move(map));

    gameObjects.emplace_back(std::make_unique<SwitchLocation>(
        tileSize * 4, tileSize * 9, // Spawn point of the object
        "objects/home_exit.png",
        player, 
        LocationID::Overworld, 
        sf::Vector2f({tileSize * 253.0f, tileSize * 126.0f}), // Spawn point of the player after interaction
        sf::Vector2f({tileSize * 1.0f, tileSize * 1.0f}), // Dimensions of the entrance 
        sf::Vector2f({0.0f, 0.0f}) // Offset of the entrance 
    ));
}


// Spawn things
void MapLoader::spawnHare(std::vector<std::unique_ptr<GameObject>> &gameObjects, Player *player, sf::FloatRect spawnArea, int entityCount) {
    for (int i = 0; i < entityCount; ++i) {
        float spawnX = 0.0f;
        float spawnY = 0.0f;
        bool validSpotFound = false;
        int maxAttempts = 30; // Prevents infinite loops if the area is completely solid

        for (int attempt = 0; attempt < maxAttempts; ++attempt) {
            // Generates random numbers within the rectangle
            float randomX = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
            float randomY = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);

            spawnX = spawnArea.position.x + (randomX * spawnArea.size.x);
            spawnY = spawnArea.position.y + (randomY * spawnArea.size.y);

            // Ensures GameObject::world exists and the tile isn't solid
            if (GameObject::world != nullptr && !GameObject::world->isSolid(spawnX, spawnY)) {
                validSpotFound = true;
                break; // If a good spot is found, exits the attempt loop
            }
        }

        if (validSpotFound) {
            gameObjects.emplace_back(std::make_unique<Hare>(spawnX, spawnY, player));
        } else {
            std::cerr << "Couldn't find a valid spawn point for Hare in the designated area\n";
        }
    }
}




// ----------------------
// ------- TileMap ------
// ----------------------
TileMap::TileMap(const std::string &textureFile, const std::string &csvFile, int width, int height, int tSize, std::vector<int> solids) 
    : GameObject(0, 0), mapWidth(width), mapHeight(height), tileSize(tSize), solidTiles(solids) {
    // Loads the texture
    if (!texture.loadFromFile(textureFile)) {
        std::cerr << "Couldn't load the tileset\n";
    }
    texture.setSmooth(false);

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
        std::cerr << "Expected " << expectedSize << " tiles, but loaded " << mapData.size() << "!\n";
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

            // Defines the 6 corners of the two triangles on the screen
            // Triangle 1 (Top-Left, Top-Right, Bottom-Left)
            triangles[0].position = sf::Vector2f(i * tileSize, j * tileSize);
            triangles[1].position = sf::Vector2f((i + 1) * tileSize, j * tileSize);
            triangles[2].position = sf::Vector2f(i * tileSize, (j + 1) * tileSize);
            
            // Triangle 2 (Bottom-Left, Top-Right, Bottom-Right)
            triangles[3].position = sf::Vector2f(i * tileSize, (j + 1) * tileSize);
            triangles[4].position = sf::Vector2f((i + 1) * tileSize, j * tileSize);
            triangles[5].position = sf::Vector2f((i + 1) * tileSize, (j + 1) * tileSize);

            // Defines the 6 corners of the two triangles on the texture
            // Inset fixed blue streaks appearing between tiles, it prevents the gpu from taking pixels of neighbouring tile
            float inset = 0.1f; 

            // Triangle 1 (Top-Left, Top-Right, Bottom-Left)
            triangles[0].texCoords = sf::Vector2f(tu * tileSize + inset, tv * tileSize + inset);
            triangles[1].texCoords = sf::Vector2f((tu + 1) * tileSize - inset, tv * tileSize + inset);
            triangles[2].texCoords = sf::Vector2f(tu * tileSize + inset, (tv + 1) * tileSize - inset);

            // Triangle 2 (Bottom-Left, Top-Right, Bottom-Right)
            triangles[3].texCoords = sf::Vector2f(tu * tileSize + inset, (tv + 1) * tileSize - inset);
            triangles[4].texCoords = sf::Vector2f((tu + 1) * tileSize - inset, tv * tileSize + inset);
            triangles[5].texCoords = sf::Vector2f((tu + 1) * tileSize - inset, (tv + 1) * tileSize - inset);
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
    int tileX = static_cast<int>(pixelX / tileSize);
    int tileY = static_cast<int>(pixelY / tileSize);

    // Prevent walking out of bounds
    if (tileX < 0 || tileX >= mapWidth || tileY < 0 || tileY >= mapHeight) {
        return true; 
    }

    int tileID = mapData[tileX + tileY * mapWidth];

    // Checks if the tileID exists in solidTiles list
    return std::find(solidTiles.begin(), solidTiles.end(), tileID) != solidTiles.end();
}