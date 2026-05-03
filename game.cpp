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
    // Allows random behavior of entities
    srand(static_cast<unsigned int>(time(nullptr)));
    GameObjects.clear();

    // Creates the map
    auto map = std::make_unique<TileMap>("map/tiles.png", "map/overworld.csv", 300, 150, 64);
    GameObject::world = map.get(); // makes it visible by all gameObjects 
    GameObjects.emplace_back(std::move(map));

    // Creates the player
    auto player = std::make_unique<Player>((tileSize * 250), (tileSize * 125));
    trackedPlayer = player.get();

    GameObjects.emplace_back(std::move(player));
    GameObjects.emplace_back(std::make_unique<Hare>(((tileSize * 255)), ((tileSize * 125)), trackedPlayer));

    camera.setSize({640, 360});
}

void Game::run() {
    this->init();

    sf::Clock clock;

    while (window.isOpen()) {

        sf::Time dt = clock.restart();

        while (auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>())
                window.close();
        }
        update(dt.asSeconds());
        render();
    }
}

void Game::update(float dt) {
    for (const auto &object : GameObjects) {
        object->update(dt, window);
    }

    // Camera logic
    if (trackedPlayer != nullptr) {
        
        float playerX = trackedPlayer->getPosition().x;
        float playerY = trackedPlayer->getPosition().y;

        // Calculates the boundaries
        float halfWidth = camera.getSize().x / 2.0f;
        float halfHeight = camera.getSize().y / 2.0f;

        // The map is 300 tiles x 64 pixels (19,200) wide
        // and 150 tiles x 64 pixels (9,600) high
        float mapWidthPixels = 300.0f * 64.0f;
        float mapHeightPixels = 150.0f * 64.0f;

        // Camera's clamped position
        // std::clamp(value_to_check, absolute_minimum, absolute_maximum)
        float cameraX = std::clamp(playerX, halfWidth, mapWidthPixels - halfWidth);
        float cameraY = std::clamp(playerY, halfHeight, mapHeightPixels - halfHeight);

        camera.setCenter({cameraX, cameraY}); 
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