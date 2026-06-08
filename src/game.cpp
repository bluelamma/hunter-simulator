#include "../include/entities.hpp"
#include "../include/world.hpp"
#include "../include/player.hpp"
#include "../include/textDisplay.hpp"
#include "../include/pickup.hpp"

TileMap* GameObject::world = nullptr;

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

sf::FloatRect GameObject::getGlobalBounds() const {
    return sprite.getGlobalBounds();
}

// ----------------------
// -------- Game --------
// ----------------------
sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
unsigned int screenWidth = desktop.size.x;
unsigned int screenHeight = desktop.size.y;

Game::Game() 
    : window(sf::VideoMode({screenWidth, screenHeight}),  "Hunter Simulator", sf::Style::None, sf::State::Fullscreen) {}

Game::~Game() = default;

void Game::init() {
sf::Image icon;
    // Makes random random
    srand(static_cast<unsigned int>(time(nullptr)));

    isPaused = false;
    isSettingsMenu = false;
    
    // Start game at home
    player = std::make_unique<Player>(0.0f, 0.0f);
    textDisplay = std::make_unique<TextDisplay>(player.get());
    loadLocation(LocationID::Home, sf::Vector2f(tileSize * 4.0f, tileSize * 2.0f));
}

void Game::loadLocation(LocationID dest, sf::Vector2f spawnPoint) {
    MapLoader::saveCaveState(); // Remembers situation in the cave
    GameObjects.clear(); // Deletes the old map and entities
    currentLocation = dest;

    if (player) {
        player->setPosition(spawnPoint); 
    }

    if (dest == LocationID::Overworld) {
        MapLoader::loadOverworld(GameObjects, player.get(), tileSize, spawnPoint);
        camera.setSize({1280, 720});
    } else if (dest == LocationID::Cave) {
        MapLoader::loadCave(GameObjects, player.get(), tileSize, spawnPoint);
        camera.setSize({1280, 720});
    } else if (dest == LocationID::Home) {
        MapLoader::loadHome(GameObjects, player.get(), tileSize, spawnPoint);
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

        if (player != nullptr) {
            player->update(dt, window);
        }

        if (switchLevel) {
            loadLocation(nextDest, nextSpawn);
        }

        // Camera logic
        if (player != nullptr) {
            float playerX = player->getPosition().x;
            float playerY = player->getPosition().y;

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
                mapWidthPixels = 50.0f * tileSize;
                mapHeightPixels = 80.0f * tileSize;
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

        // Meat will be stored here
        std::vector<std::unique_ptr<GameObject>> newDrops;

        // Projectile collision logic
        if (player != nullptr) {
            for (auto& object : GameObjects) {
                Creature* creature = dynamic_cast<Creature*>(object.get());
                
                if (creature && creature->active) {
                    for (auto& proj : player->getProjectiles()) {
                        
                        if (proj->active && creature->getBounds().findIntersection(proj->getBounds())) {
                            
                            proj->active = false; // The bullet is destroyed on impact
                            creature->takeDamage(player->getDamage()); // Reduces HP
                            
                        }
                    }
                }
            }
        }

        for (auto& object : GameObjects) {
            Creature* creature = dynamic_cast<Creature*>(object.get());
            
            if (creature && creature->checkIfDead() && !creature->active) {
                creature->grantRewards(newDrops); // Xp/cash/meat
            }
        }

        for (auto& drop : newDrops) {
            GameObjects.push_back(std::move(drop));
        }

        // Remove inactive objects
        GameObjects.erase(
            std::remove_if(GameObjects.begin(), GameObjects.end(),
                [](const std::unique_ptr<GameObject>& obj) { return !obj->active; }),
            GameObjects.end()
        );
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
                // Unpause or quit settings
                if (keyPressed->scancode == sf::Keyboard::Scancode::Escape && !player->checkIfDead()) {
                    if (isSettingsMenu) {
                        isSettingsMenu = false; // Go back to pause if in settings
                    } else {
                        isPaused = !isPaused; // Toggle pause normally
                    }
                }

                // Opens settings
                if (isPaused && !player->checkIfDead()) {
                    if (keyPressed->scancode == sf::Keyboard::Scancode::S) {
                        isSettingsMenu = !isSettingsMenu;
                        selectedSetting = 0; // Sets the top option when opening
                    }
                    
                    if (isSettingsMenu) {
                        // Navigate Up/Down
                        if (keyPressed->scancode == sf::Keyboard::Scancode::Up || keyPressed->scancode == sf::Keyboard::Scancode::Down) {
                            selectedSetting = (selectedSetting == 0) ? 1 : 0; // Toggle between 0 and 1
                        }

                        // Adjust Left/Right
                        if (keyPressed->scancode == sf::Keyboard::Scancode::Left) {
                            if (selectedSetting == 0) {
                                if (MapLoader::entityMultiplier > 0.25f) MapLoader::entityMultiplier -= 0.05f; // Decrease entity count
                            } else {
                                if (MapLoader::difficultyMultiplier > 0.25f) MapLoader::difficultyMultiplier -= 0.05f; // Decrease difficulty
                            }
                        } else if (keyPressed->scancode == sf::Keyboard::Scancode::Right) {
                            if (selectedSetting == 0) {
                                 if (MapLoader::entityMultiplier < 1.95f) MapLoader::entityMultiplier += 0.05f; // Increase entity count
                            } else {
                                 if (MapLoader::difficultyMultiplier < 1.95f) MapLoader::difficultyMultiplier += 0.05f; // Increase difficulty
                            }
                        }
                    }
                }
                
                // Quit if 'Q' is pressed while paused or dead
                if ((isPaused || player->checkIfDead()) && keyPressed->scancode == sf::Keyboard::Scancode::Q) {
                    window.close();
                }

                // Restart if the Player is dead and 'R' is pressed
                if (keyPressed->scancode == sf::Keyboard::Scancode::R) {
                    if (player != nullptr && player->checkIfDead() && !isPaused) {
                        init();
                    }
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

    if (player != nullptr) {
        player->draw(window);
    }

    // HUD
    window.setView(window.getDefaultView()); // Detaches drawing from the camera

    if (player != nullptr) {
        textDisplay->drawHud(window);
    }

    window.setView(camera); 
    // Pause and settings menu
    if (isPaused) {
        sf::Vector2f cameraCenter = camera.getCenter();
        sf::Vector2f cameraSize = camera.getSize();

        if (isSettingsMenu) {
            textDisplay->drawSettings(window, cameraCenter, cameraSize, MapLoader::entityMultiplier, MapLoader::difficultyMultiplier, selectedSetting);
        } else {
            textDisplay->drawPause(window, cameraCenter, cameraSize);
        }
    }
    // Restart menu
    if(player->checkIfDead()) {
        sf::Vector2f cameraCenter = camera.getCenter();
        sf::Vector2f cameraSize = camera.getSize();

        textDisplay->drawRestart(window, cameraCenter, cameraSize);
    }

    window.display();
}