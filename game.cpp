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

void GameObject::setSpriteScale(sf::Vector2f scale) {
    sprite.setScale(scale);
}

// ----------------------
// -------- Game --------
// ----------------------
sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
unsigned int screenWidth = desktop.size.x;
unsigned int screenHeight = desktop.size.y;

Game::Game() : window(sf::VideoMode({screenWidth, screenHeight}),  "Hunter Simulator", sf::Style::None, sf::State::Fullscreen), pauseText(font) {}

void Game::init() {
    srand(static_cast<unsigned int>(time(nullptr)));

    isPaused = false;

    // --- PAUSE MENU SETUP ---
    if (!font.openFromFile("fonts/pixelFont.ttf")) { 
        std::cerr << "Failed to load font!\n";
    }
    
    pauseText.setFont(font);
    pauseText.setString("PAUSED\n\nPress ESC to Resume\nPress Q to Quit");
    pauseText.setCharacterSize(48);
    pauseText.setFillColor(sf::Color::White);

    // Centers the text alignment
    sf::FloatRect textBounds = pauseText.getLocalBounds();
    pauseText.setOrigin({textBounds.size.x / 2.0f, textBounds.size.y / 2.0f});

    // Dark semi-transparent overlay
    pauseOverlay.setFillColor(sf::Color(0, 0, 0, 150));
    // ------------------------

    // Start game at the home
    loadLocation(LocationID::Home, sf::Vector2f(tileSize * 4.0f, tileSize * 2.0f));
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
    } else if (dest == LocationID::Home) {
        MapLoader::loadHome(GameObjects, trackedPlayer, tileSize, spawnPoint);
        camera.setSize({680, 360});
    }
}

void Game::update(float dt) {
    if (!isPaused) {
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

        float viewWidth = camera.getSize().x;
        float viewHeight = camera.getSize().y;
        float halfWidth = camera.getSize().x / 2.0f;
        float halfHeight = camera.getSize().y / 2.0f;

        float mapWidthPixels = 0.0f;
        float mapHeightPixels = 0.0f;

        if (currentLocation == LocationID::Overworld) {
            mapWidthPixels = 300.0f * tileSize;
            mapHeightPixels = 150.0f * tileSize;
        } else if (currentLocation == LocationID::Cave) {
            mapWidthPixels = 20.0f * tileSize;
            mapHeightPixels = 40.0f * tileSize;
        } else if (currentLocation == LocationID::Home) {
            mapWidthPixels = 9.0f * tileSize;   // Home is 9 tiles wide
            mapHeightPixels = 10.0f * tileSize; // Home is 10 tiles high
        }

        // std::max ensures the camera center doesn't break if the map is smaller than the window
        float maxX = std::max(halfWidth, mapWidthPixels - halfWidth);
        float maxY = std::max(halfHeight, mapHeightPixels - halfHeight);

        float cameraX = playerX;
        float cameraY = playerY;

        // Center horizontally if the map is smaller than the screen, otherwise clamp normally
        if (mapWidthPixels < viewWidth) {
            cameraX = mapWidthPixels / 2.0f;
        } else {
            cameraX = std::clamp(playerX, halfWidth, mapWidthPixels - halfWidth);
        }

        // Center vertically if the map is smaller than the screen, otherwise clamp normally
        if (mapHeightPixels < viewHeight) {
            cameraY = mapHeightPixels / 2.0f;
        } else {
            cameraY = std::clamp(playerY, halfHeight, mapHeightPixels - halfHeight);
        }

        camera.setCenter({cameraX, cameraY});
        }
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
            
            if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
                // Toggle pause on Escape
                if (keyPressed->scancode == sf::Keyboard::Scancode::Escape) {
                    isPaused = !isPaused;
                }
                
                // Quit if 'Q' is pressed while paused
                if (isPaused && keyPressed->scancode == sf::Keyboard::Scancode::Q) {
                    window.close();
                }
            }
        }
        update(dt.asSeconds());
        render();
    }
}

void Game::render() {
    if (currentLocation == LocationID::Overworld) {
        window.clear(sf::Color({80, 244, 51})); // Grass Green
    } else {
        window.clear(sf::Color::Black); // Black void for indoors
    }

    window.setView(camera);

    for(const auto &object : GameObjects) {
        object->draw(window);
    }

    // Draw the pause menu
    if (isPaused) {
        // Set the overlay and text to where the camera is looking
        sf::Vector2f cameraCenter = camera.getCenter();
        sf::Vector2f cameraSize = camera.getSize();

        pauseOverlay.setSize(cameraSize);
        pauseOverlay.setPosition(cameraCenter - (cameraSize / 2.0f));
        pauseText.setPosition(cameraCenter);

        window.draw(pauseOverlay);
        window.draw(pauseText);
    }

    window.display();
}