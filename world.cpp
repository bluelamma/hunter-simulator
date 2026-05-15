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
    entranceHitbox.setFillColor(sf::Color(255, 0, 0, 0)); // set last one to 100 for debugging
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
// --- UpgradeStation ---
// ----------------------
UpgradeStation::UpgradeStation(float startX, float startY, float width, float height, Player *player)
    : GameObject(startX, startY), player(player), isPlayerNear(false), upgradeCooldown(0.0f), promptText(font) {
    
    hitbox.setSize(sf::Vector2f({width, height}));
    hitbox.setPosition(sf::Vector2f({startX, startY}));
    hitbox.setFillColor(sf::Color(0, 0, 255, 0)); // set last one to 100 for debugging
    
    if (!font.openFromFile("fonts/pixelFont.ttf")) {
        std::cerr << "Failed to load font for UpgradeStation\n";
    }
    
    promptText.setFont(font);
    promptText.setCharacterSize(14);
    promptText.setFillColor(sf::Color::White);
    promptText.setOutlineColor(sf::Color::Black);
    promptText.setOutlineThickness(1.0f);
}

void UpgradeStation::update(float dt, sf::RenderWindow &window) {
    if (upgradeCooldown > 0.0f) {
        upgradeCooldown -= dt;
    }

    sf::Vector2f playerPos = player->getPosition();
    float playerCenterX = playerPos.x + 31.0f;
    float playerCenterY = playerPos.y + 32.0f;

    isPlayerNear = hitbox.getGlobalBounds().contains({playerCenterX, playerCenterY});

    if (isPlayerNear) {
        std::stringstream ss;
        ss << "'1' +damage ($" << std::fixed << std::setprecision(2) << player->getNextDamageCost() << ")\n"
           << "'2' -reloadSpeed ($" << player->getNextReloadCost() << ")\n"
           << "'3' +bulletSpeed ($" << player->getNextVelocityCost() << ")";
        promptText.setString(ss.str());
        
        promptText.setPosition(sf::Vector2f({hitbox.getPosition().x, hitbox.getPosition().y - 60.0f}));

        if (upgradeCooldown <= 0.0f) {
            // Damage
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Num1)) {
                if (player->spendCash(player->getNextDamageCost())) {
                    player->upgradeDamage(10.0f); 
                    upgradeCooldown = 0.5f;
                }
            } 
            // ReloadSpeed
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Num2)) {
                if (player->spendCash(player->getNextReloadCost())) {
                    player->upgradeReloadSpeed(0.05f); // 0.05 seconds faster
                    upgradeCooldown = 0.5f;
                }
            } 
            // BulletVelocity
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Num3)) {
                if (player->spendCash(player->getNextVelocityCost())) {
                    player->upgradeBulletVelocity(100.0f); // 100 speed units faster
                    upgradeCooldown = 0.5f;
                }
            }
        }
    }
}

void UpgradeStation::draw(sf::RenderWindow &window) {
    window.draw(hitbox);
    
    if (isPlayerNear) {
        window.draw(promptText);
    }
}

// ----------------------
// ------- Stall --------
// ----------------------
Stall::Stall(float startX, float startY, float width, float height, const std::string &texturePath, Player *player)
    : GameObject(startX, startY), player(player), isPlayerNear(false), buyCooldown(0.0f), promptText(font) {
    
    hitbox.setSize(sf::Vector2f({width, height}));
    hitbox.setPosition(sf::Vector2f({startX, startY}));
    hitbox.setFillColor(sf::Color(0, 0, 255, 0)); // set last one to 100 for debugging

    sprite.setPosition(sf::Vector2f({startX, startY}));
    
    if (!font.openFromFile("fonts/pixelFont.ttf")) {
        std::cerr << "Failed to load font for the Stall\n";
    } 
    
    promptText.setFont(font);
    promptText.setCharacterSize(14);
    promptText.setFillColor(sf::Color::White);
    promptText.setOutlineColor(sf::Color::Black);
    promptText.setOutlineThickness(1.0f);

    // Texture
    if (!texture.loadFromFile(texturePath)) {
        std::cerr << "Couldn't load stall's texture: " << texturePath << "\n";
    } else {
        sprite.setTexture(texture, true); 
    }
}

void Stall::update(float dt, sf::RenderWindow &window) {
    sf::Vector2f playerPos = player->getPosition();
    float playerCenterX = playerPos.x + 31.0f;
    float playerCenterY = playerPos.y + 32.0f;

    isPlayerNear = hitbox.getGlobalBounds().contains({playerCenterX, playerCenterY});

    if (isPlayerNear) {
        promptText.setString("Press 'E' to interact with Stall");
        promptText.setPosition(sf::Vector2f({hitbox.getPosition().x, hitbox.getPosition().y - 30.0f}));
    }
}

void Stall::draw(sf::RenderWindow &window) {
    window.draw(hitbox);
    window.draw(sprite);
    
    if (isPlayerNear) {
        window.draw(promptText);
    }
}


// ----------------------
// ------ MapLoader -----
// ----------------------
// Overworld
void MapLoader::loadOverworld(std::vector<std::unique_ptr<GameObject>> &gameObjects, Player *player, int tileSize, sf::Vector2f spawnPoint) {

    // Map
    auto map = std::make_unique<TileMap>("maps/tiles_overworld.png", "maps/overworld.csv", 300, 150, tileSize, std::vector<int>{3});
    TileMap* tileMap = map.get(); // Save a pointer so we can add collision boxes
    GameObject::world = map.get(); 
    gameObjects.emplace_back(std::move(map));

    // Objects
    // Cave
    auto cave = std::make_unique<SwitchLocation>(
        tileSize * 25, tileSize * 130, // Position of the object
        "objects/cave.png", 
        player, 
        LocationID::Cave, 
        sf::Vector2f({tileSize * 8.0f, tileSize * 38.0f}), 
        sf::Vector2f({220.0f, 130.0f}), // Dimensions of the entrance 
        sf::Vector2f({168.0f, 128.0f}) // Offset of the entrance 
    );
    sf::FloatRect caveBounds = cave->getGlobalBounds();
    // Cave Hitboxe
    tileMap->addSolidBox(sf::FloatRect(caveBounds.position, sf::Vector2f(168.0f, caveBounds.size.y))); // Left of entrance
    tileMap->addSolidBox(sf::FloatRect(sf::Vector2f(caveBounds.position.x + 168.0f + 220.0f, caveBounds.position.y), sf::Vector2f(caveBounds.size.x - (168.0f + 220.0f), caveBounds.size.y))); // Right of entrance
    tileMap->addSolidBox(sf::FloatRect(caveBounds.position, sf::Vector2f(caveBounds.size.x, 128.0f))); // Top part
    gameObjects.emplace_back(std::move(cave));

    // Home
    auto home = std::make_unique<SwitchLocation>(        
        tileSize * 180, tileSize * 110, // Object's position
        "objects/home.png", 
        player, 
        LocationID::Home, 
        sf::Vector2f({tileSize * 4.0f, tileSize * 8.0f}), 
        sf::Vector2f({70.0f, 80.0f}), // Dimensions of the entrance 
        sf::Vector2f({120.0f, 150.0f}) // Offset of the entrance
    );
    home->setSpriteScale({6.0f, 6.0f});
    sf::FloatRect homeBounds = home->getGlobalBounds();
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
        "objects/stall.png",
        player
    );
    stall->setSpriteScale({4.0f, 4.0f});
    sf::FloatRect stallBounds = stall->getGlobalBounds();
    // Hitbox of the stall
    tileMap->addSolidBox(sf::FloatRect(sf::Vector2f(stallBounds.position.x, stallBounds.position.y), sf::Vector2f(stallBounds.size.x, stallBounds.size.y)));
    gameObjects.emplace_back(std::move(stall));

    // Entities
    // sf::FloatRect({left_x, top_y}, {width, height})
    // Around the Player's home
    int numberOfEntities1 = rand() % 21 + 20; // 20 - 40;
    sf::FloatRect hareSpawnpoint1({tileSize * 170.0f, tileSize * 30.0f}, {tileSize * 120.0f, tileSize * 150.0f});
    spawnHare(gameObjects, player, hareSpawnpoint1, numberOfEntities1);

    // Top, Left part of the map
    int numberOfEntities2 = rand() % 11 + 10; // 10 - 20;
    sf::FloatRect hareSpawnpoint2({tileSize * 20.0f, tileSize * 20.0f}, {tileSize * 180.0f, tileSize * 40.0f});
    spawnHare(gameObjects, player, hareSpawnpoint2, numberOfEntities2);

    // Top, right part of the map
    int numberOfEntities3 = rand() % 5 + 5; // 5 - 10;
    sf::FloatRect boarSpawnpoint1({tileSize * 170.0f, tileSize * 30.0f}, {tileSize * 120.0f, tileSize * 120.0f});
    spawnBoar(gameObjects, player, boarSpawnpoint1, numberOfEntities3);

    // Bottom, right part of the map
    int numberOfEntities4 = rand() % 2 + 1; // 1 - 3;
    sf::FloatRect boarSpawnpoint2({tileSize * 170.0f, tileSize * 140.0f}, {tileSize * 120.0f, tileSize * 10.0f});
    spawnBoar(gameObjects, player, boarSpawnpoint2, numberOfEntities4);

    // Left part of the map
    int numberOfEntities5 = rand() % 20 + 40; // 40 - 60;
    sf::FloatRect boarSpawnpoint3({tileSize * 20.0f, tileSize * 20.0f}, {tileSize * 180.0f, tileSize * 150.0f});
    spawnBoar(gameObjects, player, boarSpawnpoint3, numberOfEntities5);
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
        sf::Vector2f({tileSize * 28.0f, tileSize * 134.0f}), // Spawn point of the player after interaction
        sf::Vector2f({tileSize * 3.0f + 20.0f, tileSize * 1.0f}), // Dimensions of the entrance 
        sf::Vector2f({-20.0f, -10.0f}) // Offset of the entrance 
    ));
}


// Home
void MapLoader::loadHome(std::vector<std::unique_ptr<GameObject>> &gameObjects, Player *player, int tileSize, sf::Vector2f spawnPoint) {

    // Loads the 9x10 Home map
    auto map = std::make_unique<TileMap>("maps/tiles_home.png", "maps/home.csv", 9, 10, tileSize, std::vector<int>{1, 2, 3, 4});
    GameObject::world = map.get(); 
    gameObjects.emplace_back(std::move(map));

    gameObjects.emplace_back(std::make_unique<UpgradeStation>(
        tileSize * 3.0f, tileSize * 1.0f,  // Start X, Y
        tileSize * 3.0f, tileSize * 1.5f,  // Width, Height
        player
    ));

    gameObjects.emplace_back(std::make_unique<SwitchLocation>(
        tileSize * 4, tileSize * 9, // Spawn point of the object
        "objects/home_exit.png",
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

void MapLoader::spawnBoar(std::vector<std::unique_ptr<GameObject>> &gameObjects, Player *player, sf::FloatRect spawnArea, int entityCount) {
    for (int i = 0; i < entityCount; ++i) {
        float spawnX = 0.0f;
        float spawnY = 0.0f;
        bool validSpotFound = false;
        int maxAttempts = 30; 

        float offsetX = 16.0f; // ~~ half the boar's width
        float offsetY = 32.0f; // ~~ the boar's full height

        for (int attempt = 0; attempt < maxAttempts; ++attempt) {
            float randomX = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
            float randomY = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);

            spawnX = spawnArea.position.x + (randomX * spawnArea.size.x);
            spawnY = spawnArea.position.y + (randomY * spawnArea.size.y);

            // Calculates where the feet will actually land
            float feetX = spawnX + offsetX;
            float feetY = spawnY + offsetY;

            // Check if the spot is valid based on where the feet are
            if (GameObject::world != nullptr && !GameObject::world->isSolid(feetX, feetY)) {
                validSpotFound = true;
                break; 
            }
        }

        if (validSpotFound) {
            gameObjects.emplace_back(std::make_unique<Boar>(spawnX, spawnY, player));
        } else {
            std::cerr << "Couldn't find a valid spawn point for Boar in the designated area\n";
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

void TileMap::addSolidBox(const sf::FloatRect& box) {
    solidBoxes.push_back(box);
}

bool TileMap::isSolid(float pixelX, float pixelY) const {
    // Checks custom solid boundary boxes for collision 
    for (const auto& box : solidBoxes) {
        if (box.contains(sf::Vector2f(pixelX, pixelY))) {
            return true;
        }
    }

    // Then checks the tiles
    int tileX = static_cast<int>(pixelX / tileSize);
    int tileY = static_cast<int>(pixelY / tileSize);

    if (tileX < 0 || tileX >= mapWidth || tileY < 0 || tileY >= mapHeight) {
        return true; 
    }

    int tileID = mapData[tileX + tileY * mapWidth];
    return std::find(solidTiles.begin(), solidTiles.end(), tileID) != solidTiles.end();
}