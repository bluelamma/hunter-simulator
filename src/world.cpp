#include "../include/world.hpp"
#include "../include/world_objects.hpp"

// Can be changed in settings
float MapLoader::entityMultiplier = 1.0f;
float MapLoader::difficultyMultiplier = 1.0f;

// ----------------------
// ------ MapLoader -----
// ----------------------
// Overworld
void MapLoader::loadOverworld(std::vector<std::unique_ptr<GameObject>> &gameObjects, Player *player, int tileSize, sf::Vector2f spawnPoint) {

    // Map
    auto map = std::make_unique<TileMap>("../assets/maps/tiles_overworld.png", "../assets/maps/overworld.csv", 300, 150, tileSize, std::vector<int>{3}, 3, std::vector<int>{2, 4, 5});
    TileMap* tileMap = map.get(); // Save a pointer so we can add collision boxes
    GameObject::world = map.get(); 
    gameObjects.emplace_back(std::move(map));

    // Objects
    // Cave
    auto cave = std::make_unique<SwitchLocation>(
        tileSize * 25, tileSize * 130, // Position of the object
        "../assets/objects/cave.png", 
        player, 
        LocationID::Cave, 
        sf::Vector2f({tileSize * 26.0f, tileSize * 78.0f}), // Player's spawnpoint after interacting
        sf::Vector2f({220.0f, 130.0f}), // Dimensions of the entrance 
        sf::Vector2f({168.0f, 128.0f}) // Offset of the entrance 
    );
    sf::FloatRect caveBounds = cave->getGlobalBounds();
    // Safe area around the home
    tileMap->addSpawnArea(sf::FloatRect(sf::Vector2f(caveBounds.position.x - tileSize * 10, caveBounds.position.y - tileSize * 2), sf::Vector2f(tileSize * 25, tileSize * 20)));
    // Cave Hitboxes
    tileMap->addSolidBox(sf::FloatRect(caveBounds.position, sf::Vector2f(168.0f, caveBounds.size.y))); // Left of entrance
    tileMap->addSolidBox(sf::FloatRect(sf::Vector2f(caveBounds.position.x + 168.0f + 220.0f, caveBounds.position.y), sf::Vector2f(caveBounds.size.x - (168.0f + 220.0f), caveBounds.size.y))); // Right of entrance
    tileMap->addSolidBox(sf::FloatRect(caveBounds.position, sf::Vector2f(caveBounds.size.x, 128.0f))); // Top part
    gameObjects.emplace_back(std::move(cave));

    // Home
    auto home = std::make_unique<SwitchLocation>(        
        tileSize * 180 - 2.0f, tileSize * 109 + 24.0f, // Object's position
        "../assets/objects/home.png", 
        player, 
        LocationID::Home, 
        sf::Vector2f({tileSize * 4.0f, tileSize * 8.0f}), // Player's spawnpoint after interacting
        sf::Vector2f({70.0f, 80.0f}), // Dimensions of the entrance 
        sf::Vector2f({120.0f, 150.0f}) // Offset of the entrance
    );
    home->setSpriteScale({6.0f, 6.0f});
    sf::FloatRect homeBounds = home->getGlobalBounds();
    // Safe area around the home
    tileMap->addSpawnArea(sf::FloatRect(sf::Vector2f(homeBounds.position.x - tileSize * 5, homeBounds.position.y - tileSize * 5), sf::Vector2f(tileSize * 20, tileSize * 20)));
    
    // Hitbox of the home
    // home: 54x40 * 6 => 324x240 // door: 10x12 * 6 => 60x72
    tileMap->addSolidBox(sf::FloatRect(sf::Vector2f(homeBounds.position.x + 18.0f, homeBounds.position.y), sf::Vector2f(114.0f, homeBounds.size.y))); // Left of door
    tileMap->addSolidBox(sf::FloatRect(sf::Vector2f(homeBounds.position.x + 162.0f + 30.0f, homeBounds.position.y), sf::Vector2f(homeBounds.size.x - (162.0f + 48.0f), homeBounds.size.y))); // Right of door
    tileMap->addSolidBox(sf::FloatRect(sf::Vector2f(homeBounds.position.x + 80.0f, homeBounds.position.y), sf::Vector2f(homeBounds.size.x - 100.0f, 200.0f))); // Top part
    tileMap->addSolidBox(sf::FloatRect(homeBounds.position, sf::Vector2f(homeBounds.size.x, 54.0f))); // A bit wider on the top for the roof
    gameObjects.emplace_back(std::move(home));

    // Stall
    auto stall = std::make_unique<Stall>(        
        tileSize * 189 - 10.0f, tileSize * 110, // Object's position
        tileSize * 3 + 15.0f, tileSize * 3, // Size
        "../assets/objects/stall.png",
        player
    );
    stall->setSpriteScale({4.0f, 4.0f});
    sf::FloatRect stallBounds = stall->getGlobalBounds();
    // Hitbox of the stall
    tileMap->addSolidBox(sf::FloatRect(sf::Vector2f(stallBounds.position.x, stallBounds.position.y), sf::Vector2f(stallBounds.size.x, stallBounds.size.y)));
    gameObjects.emplace_back(std::move(stall));

    // --- Entities ---

    // Hares
    // sf::FloatRect({left_x, top_y}, {width, height})
    // Whole right side of the map
    sf::FloatRect hareSpawnpoint1({tileSize * 170.0f, tileSize * 30.0f}, {tileSize * 120.0f, tileSize * 150.0f});
    spawnHare(gameObjects, player, hareSpawnpoint1, (rand() % 31 + 30) * entityMultiplier);

    // Top, Left part of the map
    sf::FloatRect hareSpawnpoint2({tileSize * 20.0f, tileSize * 20.0f}, {tileSize * 180.0f, tileSize * 40.0f});
    spawnHare(gameObjects, player, hareSpawnpoint2, (rand() % 11 + 10) * entityMultiplier);

    // Close to the spawn area
    sf::FloatRect hareSpawnpoint3({tileSize * 170.0f, tileSize * 80.0f}, {tileSize * 120.0f, tileSize * 80.0f});
    spawnHare(gameObjects, player, hareSpawnpoint3, (rand() % 11 + 10) * entityMultiplier);

    // Boars
    // Right part of the map
    sf::FloatRect boarSpawnpoint1({tileSize * 180.0f, tileSize * 10.0f}, {tileSize * 120.0f, tileSize * 150.0f});
    spawnBoar(gameObjects, player, boarSpawnpoint1, (rand() % 26 + 30) * entityMultiplier, 0);

    // Right part of the map // bottom
    sf::FloatRect boarSpawnpoint2({tileSize * 170.0f, tileSize * 140.0f}, {tileSize * 100.0f, tileSize * 15.0f});
    spawnBoar(gameObjects, player, boarSpawnpoint2, (rand() % 3 + 1) * entityMultiplier, 0);

    // Black // right part // center
    sf::FloatRect boarSpawnpoint5({tileSize * 200.0f, tileSize * 40.0f}, {tileSize * 60.0f, tileSize * 40.0f});
    spawnBoar(gameObjects, player, boarSpawnpoint5, (rand() % 3 + 2) * entityMultiplier, 1);

    // Left part of the map
    sf::FloatRect boarSpawnpoint3({tileSize * 20.0f, tileSize * 20.0f}, {tileSize * 160.0f, tileSize * 100.0f});
    spawnBoar(gameObjects, player, boarSpawnpoint3, (rand() % 36 + 50) * entityMultiplier, 0);

    // Black // Left part of the map
    sf::FloatRect boarSpawnpoint4({tileSize * 10.0f, tileSize * 20.0f}, {tileSize * 150.0f, tileSize * 100.0f});
    spawnBoar(gameObjects, player, boarSpawnpoint4, (rand() % 11 + 20) * entityMultiplier, 1);

    // Bears
    // Left part of the map
    sf::FloatRect bearSpawnpoint1({tileSize * 20.0f, tileSize * 20.0f}, {tileSize * 160.0f, tileSize * 100.0f});
    spawnBear(gameObjects, player, bearSpawnpoint1, 4 * entityMultiplier);

    // Right part of the map
    sf::FloatRect bearSpawnpoint2({tileSize * 180.0f, tileSize * 20.0f}, {tileSize * 120.0f, tileSize * 100.0f});
    spawnBear(gameObjects, player, bearSpawnpoint2, 2 * entityMultiplier);

    // --- Scenery ---

    // Assets
    std::vector<std::string> grassVariants = {
        "../assets/grass/grass0.png", "../assets/grass/grass1.png", "../assets/grass/grass2.png",
        "../assets/grass/grass3.png", "../assets/grass/grass4.png"
    };
    std::vector<std::string> bushVariants = {
        "../assets/grass/bush0.png", "../assets/grass/bush1.png"
    };
    sf::FloatRect wholeMap({0.0f, 0.0f}, {tileSize * 300.0f, tileSize * 150.0f});
    
    // grass patches
    spawnScenery(gameObjects, wholeMap, 300, grassVariants);
    // bushes
    spawnScenery(gameObjects, wholeMap, 50, bushVariants);
}

// Cave
bool MapLoader::bearBossDefeated = false;
Bear* MapLoader::activeBearBoss = nullptr;

void MapLoader::loadCave(std::vector<std::unique_ptr<GameObject>> &gameObjects, Player *player, int tileSize, sf::Vector2f spawnPoint) {

    // Loads the 50x80 Cave map
    auto map = std::make_unique<TileMap>("../assets/maps/tiles_cave.png", "../assets/maps/cave.csv", 50, 80, tileSize, std::vector<int>{1}, 1, std::vector<int>{99});
    GameObject::world = map.get(); 
    gameObjects.emplace_back(std::move(map));

    // Cave exit (to go back to overworld)
    gameObjects.emplace_back(std::make_unique<SwitchLocation>(
        tileSize * 20, tileSize * 79, // Spawn point of the object
        "objects/cave_exit.png",
        player, 
        LocationID::Overworld, 
        sf::Vector2f({tileSize * 28.0f, tileSize * 134.0f}), // Spawn point of the player after interaction
        sf::Vector2f({tileSize * 9.0f + 20.0f, tileSize * 1.0f}), // Dimensions of the entrance 
        sf::Vector2f({-20.0f, -10.0f}) // Offset of the entrance 
    ));

    // Spawns the bear only if it wasn't defeated yet
    if (!bearBossDefeated) {
        auto bearBoss = std::make_unique<Bear>(tileSize * 20.0f, tileSize * 20.0f, player, true);
        
        // Pointer
        activeBearBoss = bearBoss.get(); 
        
        gameObjects.emplace_back(std::move(bearBoss));
    } else {
        // Pointer is null if the bear isn't spawned
        activeBearBoss = nullptr;
    }
}

    // Checks if the bear died during the player's visit
    void MapLoader::saveCaveState() {
        if (activeBearBoss != nullptr) {
            if (activeBearBoss->checkIfDead()) {
                bearBossDefeated = true; // Mark as dead
            }
        }
        // Prevents unnecessary pointers when the map gets unloaded 
        activeBearBoss = nullptr;
    }


// Home
void MapLoader::loadHome(std::vector<std::unique_ptr<GameObject>> &gameObjects, Player *player, int tileSize, sf::Vector2f spawnPoint) {

    // Loads the 9x10 Home map
    auto map = std::make_unique<TileMap>("../assets/maps/tiles_home.png", "../assets/maps/home.csv", 9, 10, tileSize, std::vector<int>{1, 2, 3, 4}, 99, std::vector<int>{99});
    GameObject::world = map.get(); 
    gameObjects.emplace_back(std::move(map));

    gameObjects.emplace_back(std::make_unique<UpgradeStation>(
        tileSize * 3.0f, tileSize * 1.0f,  // Start X, Y
        tileSize * 3.0f, tileSize * 1.5f,  // Width, Height
        player
    ));

    gameObjects.emplace_back(std::make_unique<SwitchLocation>(
        tileSize * 4, tileSize * 9, // Spawn point of the object
        "../assets/objects/home_exit.png",
        player, 
        LocationID::Overworld, 
        sf::Vector2f({tileSize * 182.0f, tileSize * 114.0f}), // Spawn point of the player after interaction
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
        int maxAttempts = 30; 

        // 0.5 width and full height of the sprite
        float offsetX = 24.0f; 
        float offsetY = 36.0f; 

        for (int attempt = 0; attempt < maxAttempts; ++attempt) {
            // Random numbers between 0 and 1
            float randomX = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
            float randomY = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);

            spawnX = spawnArea.position.x + (randomX * spawnArea.size.x);
            spawnY = spawnArea.position.y + (randomY * spawnArea.size.y);

            float feetX = spawnX + offsetX;
            float feetY = spawnY + offsetY;

            // Checks position at the feet
            if (GameObject::world != nullptr && !GameObject::world->isSolid(feetX, feetY, false, true)) {
                validSpotFound = true;
                break; 
            }
        }

        if (validSpotFound) {
            gameObjects.emplace_back(std::make_unique<Hare>(spawnX, spawnY, player));
        } else {
            std::cerr << "Couldn't find a valid spawn point for Hare in the designated area\n";
        }
    }
}

void MapLoader::spawnBoar(std::vector<std::unique_ptr<GameObject>> &gameObjects, Player *player, sf::FloatRect spawnArea, int entityCount, int variant) {
    for (int i = 0; i < entityCount; ++i) {
        float spawnX = 0.0f;
        float spawnY = 0.0f;
        bool validSpotFound = false;
        int maxAttempts = 30; 

        float offsetX = 23.0f; // ~~ half the boar's width
        float offsetY = 28.0f; // ~~ boar's full height 

        for (int attempt = 0; attempt < maxAttempts; ++attempt) {
            float randomX = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
            float randomY = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);

            spawnX = spawnArea.position.x + (randomX * spawnArea.size.x);
            spawnY = spawnArea.position.y + (randomY * spawnArea.size.y);

            // Calculates where the feet will land
            float feetX = spawnX + offsetX;
            float feetY = spawnY + offsetY;

            // Check if the spot is valid based on where the feet are
            if (GameObject::world != nullptr && !GameObject::world->isSolid(feetX, feetY, false, true) && !GameObject::world->isSpawn(feetX, feetY)) {
                validSpotFound = true;
                break; 
            }
        }

        if (validSpotFound) {
            gameObjects.emplace_back(std::make_unique<Boar>(spawnX, spawnY, player, variant));
        } else {
            std::cerr << "Couldn't find a valid spawn point for Boar in the designated area\n";
        }
    }
}

void MapLoader::spawnBear(std::vector<std::unique_ptr<GameObject>> &gameObjects, Player *player, sf::FloatRect spawnArea, int entityCount) {
    for (int i = 0; i < entityCount; ++i) {
        float spawnX = 0.0f;
        float spawnY = 0.0f;
        bool validSpotFound = false;
        int maxAttempts = 30; 

        float offsetX = 31.5f; // ~~ half the bear's width
        float offsetY = 40.0f; // bear's full height

        for (int attempt = 0; attempt < maxAttempts; ++attempt) {
            float randomX = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
            float randomY = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);

            spawnX = spawnArea.position.x + (randomX * spawnArea.size.x);
            spawnY = spawnArea.position.y + (randomY * spawnArea.size.y);

            // Calculates where the feet will land
            float feetX = spawnX + offsetX;
            float feetY = spawnY + offsetY;

            // Check if the spot is valid based on where the feet are
            if (GameObject::world != nullptr && !GameObject::world->isSolid(feetX, feetY, false, true) && !GameObject::world->isSpawn(feetX, feetY)) {
                validSpotFound = true;
                break; 
            }
        }

        if (validSpotFound) {
            gameObjects.emplace_back(std::make_unique<Bear>(spawnX, spawnY, player, false));
        } else {
            std::cerr << "Couldn't find a valid spawn point for Bear in the designated area\n";
        }
    }
}

void MapLoader::spawnScenery(std::vector<std::unique_ptr<GameObject>> &gameObjects, sf::FloatRect spawnArea, int entityCount, const std::vector<std::string> &texturePaths) {
    if (texturePaths.empty()) return;

    for (int i = 0; i < entityCount; ++i) {
        float spawnX = 0.0f;
        float spawnY = 0.0f;
        bool validSpotFound = false;
        int maxAttempts = 30; 

        for (int attempt = 0; attempt < maxAttempts; ++attempt) {
            // random number between 0 and 1
            float randomX = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
            float randomY = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);

            spawnX = spawnArea.position.x + (randomX * spawnArea.size.x) - 32.0f;
            spawnY = spawnArea.position.y + (randomY * spawnArea.size.y) - 32.0f;

            float size = 16.0f * 2.5f;

            // Checks the bottom of the sprite
            bool bottomLeftBad = GameObject::world->isSolid(spawnX, spawnY + size, false, true);
            bool bottomRightBad = GameObject::world->isSolid(spawnX + size, spawnY + size, false, true);

            if (GameObject::world != nullptr && !bottomLeftBad && !bottomRightBad) {
                validSpotFound = true;
                break;
            }
        }

        if (validSpotFound) {
            // Picks a random texture
            std::string randomTexture = texturePaths[rand() % texturePaths.size()];
            auto scenery = std::make_unique<Scenery>(spawnX, spawnY, randomTexture);
            scenery->setSpriteScale(sf::Vector2f{2.5f, 2.5f});
            gameObjects.emplace_back(std::move(scenery));
        } else {
            std::cerr << "Couldn't find a valid spawn point for Scenery\n";
        }
    }
}




// ----------------------
// ------- TileMap ------
// ----------------------
TileMap::TileMap(const std::string &textureFile, const std::string &csvFile, int width, int height, int tSize, std::vector<int> solids, int waterTile, std::vector<int> noSceneryTiles) 
    : GameObject(0, 0), mapWidth(width), mapHeight(height), tileSize(tSize), solidTiles(solids), waterTile(waterTile), noSceneryTiles(noSceneryTiles) {
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
        std::cerr << "Couldn't open file: " << csvFile << "\n";
    }

    int expectedSize = mapWidth * mapHeight;
    if (mapData.size() != expectedSize) {
        std::cerr << "Expected " << expectedSize << " tiles, but loaded " << mapData.size() << "\n";
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
            // Inset fixed blue streaks appearing between tiles
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

void TileMap::addSolidBox(const sf::FloatRect& box) {
    solidBoxes.push_back(box);
}

void TileMap::addSpawnArea(const sf::FloatRect& area) {
    spawnAreas.push_back(area);
}

bool TileMap::isSolid(float pixelX, float pixelY, bool isProjectile, bool isScenery) const {
    // Checks custom boundary boxes for collision 
    for (const auto& box : solidBoxes) {
        if (box.contains(sf::Vector2f(pixelX, pixelY))) {
            return true;
        }
    }

    // Then checks the tiles
    int tileX = static_cast<int>(pixelX / tileSize);
    int tileY = static_cast<int>(pixelY / tileSize);

    // Prevents walking out of bounds
    if (tileX < 0 || tileX >= mapWidth || tileY < 0 || tileY >= mapHeight) {
        return true; 
    }

    int tileID = mapData[tileX + tileY * mapWidth];

    if(isProjectile && tileID == waterTile) {
        return false;
    }

    if(isScenery) {
        for (auto blockedTile : noSceneryTiles) {
            if (tileID == blockedTile) {
                return true;
            }
        }
    }

    return std::find(solidTiles.begin(), solidTiles.end(), tileID) != solidTiles.end();
}

bool TileMap::isSpawn(float pixelX, float pixelY) const {
    // Checks custom solid boundary boxes for collision 
    for (const auto& area : spawnAreas) {
        if (area.contains(sf::Vector2f(pixelX, pixelY))) {
            return true;
        } 
    }

    return false;
}

// ----------------------
// ------- Scenery ------
// ----------------------
std::unordered_map<std::string, sf::Texture> Scenery::textureCache;

Scenery::Scenery(float startX, float startY, const std::string& texturePath) 
    : GameObject(startX, startY) {
    
    // Checks if the texture isn't in the cache yet
    if (textureCache.find(texturePath) == textureCache.end()) {
        sf::Texture newTexture;
        if (!newTexture.loadFromFile(texturePath)) {
            std::cerr << "Couldn't load scenery texture: " << texturePath << "\n";
        }
        textureCache[texturePath] = newTexture; // Stores it for next time
    }
    
    sprite.setTexture(textureCache[texturePath], true); 
}

void Scenery::draw(sf::RenderWindow &window) {
    window.draw(sprite);
}

void Scenery::update(float dt, sf::RenderWindow &window) {}