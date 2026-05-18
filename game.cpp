#include "entities.hpp"
#include "world.hpp"
#include "player.hpp"

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

    // Player's sprite was moving one pixel to the right, that corrects it
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
    : window(sf::VideoMode({screenWidth, screenHeight}),  "Hunter Simulator", sf::Style::None, sf::State::Fullscreen), pauseText(font), cashText(font), restartText(font) {}

Game::~Game() = default;

void Game::init() {
    srand(static_cast<unsigned int>(time(nullptr)));

    isPaused = false;

    // --- HUD ---
    // -- Pausing -- 
    if (!font.openFromFile("fonts/pixelFont.ttf")) { 
        std::cerr << "Failed to load the font\n";
    }
    
    pauseText.setFont(font);
    pauseText.setString("PAUSED\n\nPress ESC to Resume\nPress Q to Quit");
    pauseText.setCharacterSize(48);
    pauseText.setFillColor(sf::Color::White);

    // Centers the text alignment
    sf::FloatRect pauseTextBounds = pauseText.getLocalBounds();
    pauseText.setOrigin({pauseTextBounds.size.x / 2.0f, pauseTextBounds.size.y / 2.0f});

    // Dark overlay
    pauseOverlay.setFillColor(sf::Color(0, 0, 0, 150));


    // -- Restart --
    restartText.setFont(font);
    restartText.setString("\n\nPress R to Restart\nPress Q to Quit");
    restartText.setCharacterSize(28);
    restartText.setFillColor(sf::Color(255, 255, 255));

    // Centers the text alignment
    sf::FloatRect restartTextBounds = restartText.getLocalBounds();
    restartText.setOrigin({restartTextBounds.size.x / 2.0f, restartTextBounds.size.y / 2.0f - 20.0f});

    // Transparent
    restartOverlay.setFillColor(sf::Color(255, 0, 0, 80));


    // -- Cash display --
    cashText.setFont(font);
    cashText.setCharacterSize(36); // Adjust size as needed
    cashText.setFillColor(sf::Color::Yellow); // Using yellow to represent cash
    cashText.setOutlineColor(sf::Color::Black);
    cashText.setOutlineThickness(2.0f);
    // ------------------------

    // Start game at the home
    player = std::make_unique<Player>(0.0f, 0.0f);
    loadLocation(LocationID::Home, sf::Vector2f(tileSize * 4.0f, tileSize * 2.0f));
}

void Game::loadLocation(LocationID dest, sf::Vector2f spawnPoint) {
    MapLoader::saveCaveState(); // Remembers situation in the cave
    GameObjects.clear(); // Deletes the old player, map, and entities
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

        // Projectile collision logic
        if (player != nullptr) {
            for (auto& object : GameObjects) {
                // Checks if the object is a hare
                Hare* hare = dynamic_cast<Hare*>(object.get());
                Boar* boar = dynamic_cast<Boar*>(object.get());
                Bear* bear = dynamic_cast<Bear*>(object.get());
                
                if (hare && hare->active) {
                    for (auto& proj : player->getProjectiles()) {
                        
                        if (proj->active && hare->getBounds().findIntersection(proj->getBounds())) {
                            
                            proj->active = false; // The bullet is destroyed on impact
                            
                            hare->takeDamage(player->getDamage()); // will reduce hp of the hare
                            
                            if (hare->checkIfDead() && hare->getDeathTimer() == 0) { 
                                player->addExperience(50 * hare->getDiff()); 
                                player->addCash(2.0f * hare->getDiff() * 3.0f );
                            }
                        }
                    }
                }

                if (boar && boar->active) {
                    for (auto& proj : player->getProjectiles()) {
                        
                        if (proj->active && boar->getBounds().findIntersection(proj->getBounds())) {
                            
                            proj->active = false; // The bullet is destroyed on impact
                            
                            boar->takeDamage(player->getDamage()); // will reduce hp of the boar
                            
                            if (boar->checkIfDead() && boar->getDeathTimer() == 0) { 
                                player->addExperience(100 * boar->getDiff()); 
                                player->addCash(5.0f * boar->getDiff() * 5.0f);
                            }
                        }
                    }
                }

                if (bear && bear->active) {
                    for (auto& proj : player->getProjectiles()) {
                        
                        if (proj->active && bear->getBounds().findIntersection(proj->getBounds())) {
                            
                            proj->active = false; // The bullet is destroyed on impact
                            
                            bear->takeDamage(player->getDamage()); // will reduce hp of the bear

                            if (bear->checkIfDead() && bear->getDeathTimer() == 0) { 
                                
                                // Checks if it's the boss or a normal bear for reward
                                if (bear->checkIfBoss()) {
                                    player->addExperience(3000 * bear->getDiff()); 
                                    player->addCash(500.0f * bear->getDiff() * 5.0f);
                                } else {
                                    player->addExperience(500 * bear->getDiff()); 
                                    player->addCash(50.0f * bear->getDiff() * 5.0f);
                                }
                            }
                        }
                    }
                }
            }
        }

        // Remove dead entities
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
                // Toggle pause on Escape
                if (keyPressed->scancode == sf::Keyboard::Scancode::Escape && !player->checkIfDead()) {
                    isPaused = !isPaused;
                }
                
                // Quit if 'Q' is pressed while paused or while dead
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
        // Left side (hp, xp)
        // Prevents negative percentages if HP drops below 0
        float hpPercent = player->getHp() > 0 ? static_cast<float>(player->getHp()) / player->getMaxHp() : 0.0f;
        float xpPercent = static_cast<float>(player->getExperience()) / player->getExperienceThreshold();

        // XP bar
        sf::RectangleShape xpBg(sf::Vector2f({200.0f, 8.0f}));
        xpBg.setPosition(sf::Vector2f({20.0f, 22.0f}));
        xpBg.setFillColor(sf::Color(0, 0, 50, 150)); // Dark blue background

        sf::RectangleShape xpBar(sf::Vector2f({200.0f * xpPercent, 8.0f}));
        xpBar.setPosition(sf::Vector2f({20.0f, 22.0f}));
        xpBar.setFillColor(sf::Color(0, 150, 255, 255)); // Bright blue fill

        // HP bar
        sf::RectangleShape hpBg(sf::Vector2f({200.0f, 20.0f}));
        hpBg.setPosition(sf::Vector2f({20.0f, 30.0f})); 
        hpBg.setFillColor(sf::Color(50, 0, 0, 150)); // Dark red background

        sf::RectangleShape hpBar(sf::Vector2f({200.0f * hpPercent, 20.0f}));
        hpBar.setPosition(sf::Vector2f({20.0f, 30.0f}));
        hpBar.setFillColor(sf::Color(255, 0, 0, 255)); // Bright red fill

        // Right side (cash)
        std::stringstream stream;
        stream << "Cash: $" << std::fixed << std::setprecision(2) << player->getCash();
        cashText.setString(stream.str());

        sf::FloatRect textBounds = cashText.getLocalBounds();
        float screenWidth = window.getSize().x;
        cashText.setPosition(sf::Vector2f(screenWidth - textBounds.size.x - 20.0f, 20.0f));

        // Draws the hud
        window.draw(xpBg);
        window.draw(xpBar);
        window.draw(hpBg);
        window.draw(hpBar); 
        window.draw(cashText);
    }

    // Pause menu
    window.setView(camera); 

    if (isPaused) {
        sf::Vector2f cameraCenter = camera.getCenter();
        sf::Vector2f cameraSize = camera.getSize();

        pauseOverlay.setSize(cameraSize);
        pauseOverlay.setPosition(cameraCenter - (cameraSize / 2.0f));
        pauseText.setPosition(cameraCenter);

        window.draw(pauseOverlay);
        window.draw(pauseText);
    }

    if(player->checkIfDead()) {
        sf::Vector2f cameraCenter = camera.getCenter();
        sf::Vector2f cameraSize = camera.getSize();

        restartOverlay.setSize(cameraSize);
        restartOverlay.setPosition(cameraCenter - (cameraSize / 2.0f));
        restartText.setPosition(cameraCenter);
        
        window.draw(restartOverlay);
        window.draw(restartText);
    }

    window.display();
}