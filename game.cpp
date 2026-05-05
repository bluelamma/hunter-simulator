#include "entities.hpp"
#include "world.hpp"

TileMap* GameObject::world = nullptr;

// -----------------------
// ------ Animation ------
// -----------------------
Animation::Animation(int frameWidth, int frameHeight, float holdTime) 
    : frameWidth(frameWidth), frameHeight(frameHeight), holdTime(holdTime), timer(0.0f), currentFrame(0) {}

void Animation::update(int row, int startFrame, int endFrame, float dt, sf::Sprite &sprite) {
    if (currentFrame < startFrame || currentFrame > endFrame) {
        currentFrame = startFrame;
        timer = 0.0f;
    }

    timer += dt;

    if (timer >= holdTime) {
        timer -= holdTime;
        currentFrame++;

        if (currentFrame > endFrame) {
            currentFrame = startFrame;
        }
    }

    int left = frameWidth * currentFrame;
    int top = frameHeight * row;

    // Sprite was moving one pixel to the right for whatever reason, that corrects it
    if(startFrame == 3 && row == 1) {
        left += 1;
    }

    sprite.setTextureRect(sf::IntRect({left, top}, {frameWidth, frameHeight}));
}

// -----------------------
// ----- Game object -----
// -----------------------
GameObject::GameObject(float startX, float startY) 
    : x(startX), y(startY), width(0.0f), height(0.0f), active(true), sprite(texture) {
        sprite.setPosition({x, y});
    }

// ----------------------
// -------- Game --------
// ----------------------
sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
unsigned int screenWidth = desktop.size.x;
unsigned int screenHeight = desktop.size.y;

Game::Game() : window(sf::VideoMode({screenWidth, screenHeight}),  "Hunter Simulator") {}

void Game::init() {
    srand(static_cast<unsigned int>(time(nullptr)));
    camera.setSize({1280, 720});

    // Start game in the Overworld
    loadLocation(LocationID::Overworld, sf::Vector2f(tileSize * 250.0f, tileSize * 125.0f));
}

void Game::loadLocation(LocationID dest, sf::Vector2f spawnPoint) {
    GameObjects.clear(); // Deletes the old player, map, and entities
    currentLocation = dest;

    if (dest == LocationID::Overworld) {
        MapLoader::loadOverworld(GameObjects, trackedPlayer, tileSize, spawnPoint);
        camera.setSize({1280, 720});
    } else if (dest == LocationID::Cave) {
        MapLoader::loadCave(GameObjects, trackedPlayer, tileSize, spawnPoint);
        camera.setSize({680, 360});
    }
}

void Game::update(float dt) {
    bool switchLevel = false;
    LocationID nextDest;
    sf::Vector2f nextSpawn;

    for (const auto &object : GameObjects) {
        object->update(dt, window);
        
        // Identifies if the object is a SwitchLocation and if the player triggered it
        SwitchLocation* switchObj = dynamic_cast<SwitchLocation*>(object.get());
        if (switchObj && switchObj->hasPlayerEntered()) {
            switchLevel = true;
            nextDest = switchObj->getDestination();
            nextSpawn = switchObj->getSpawnPoint();
        }
    }

    if (switchLevel) {
        loadLocation(nextDest, nextSpawn);
    }

    // Camera logic
    if (trackedPlayer != nullptr) {
        float playerX = trackedPlayer->getPosition().x;
        float playerY = trackedPlayer->getPosition().y;

        float halfWidth = camera.getSize().x / 2.0f;
        float halfHeight = camera.getSize().y / 2.0f;

        // Set boundaries based on current map size
        // true = overworld values // false = cave values (for now)
        float mapWidthPixels = (currentLocation == LocationID::Overworld ? 300.0f : 20.0f) * tileSize;
        float mapHeightPixels = (currentLocation == LocationID::Overworld ? 150.0f : 40.0f) * tileSize;

        // std::max ensures the camera center doesn't break if the map is smaller than the window
        float maxX = std::max(halfWidth, mapWidthPixels - halfWidth);
        float maxY = std::max(halfHeight, mapHeightPixels - halfHeight);

        float cameraX = std::clamp(playerX, halfWidth, maxX);
        float cameraY = std::clamp(playerY, halfHeight, maxY);

        camera.setCenter({cameraX, cameraY});
    }
}

void Game::run() {
    this->init();

    sf::Clock clock;

    // Main game loop
    while (window.isOpen()) {

        // Restarts the clock to get delta time
        sf::Time dt = clock.restart();

        while (auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>())
                window.close();
        }
        update(dt.asSeconds());
        render();
    }
}

void Game::render() {
    window.clear(sf::Color({80, 244, 51}));

    window.setView(camera);

    for(const auto &object : GameObjects) {
        object->draw(window);
    }

    window.display();
}