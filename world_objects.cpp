#include "world_objects.hpp"

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
        ss << "'1' damage++ ($" << std::fixed << std::setprecision(2) << player->getNextDamageCost() << ")\n"
           << "'2' reloadSpeed-- ($" << player->getNextReloadCost() << ")\n"
           << "'3' bulletSpeed++ ($" << player->getNextVelocityCost() << ")";
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
    : GameObject(startX, startY), player(player), isPlayerNear(false), buyingCooldown(0.0f), promptText(font) {
    
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
    if (buyingCooldown > 0.0f) {
        buyingCooldown -= dt;
    }

    sf::Vector2f playerPos = player->getPosition();
    float playerCenterX = playerPos.x + 31.0f;
    float playerCenterY = playerPos.y + 32.0f;

    isPlayerNear = hitbox.getGlobalBounds().contains({playerCenterX, playerCenterY});

    if (isPlayerNear) {
        if (player->getSpeedThreshold() < 500.0f)  {
            std::stringstream ss;
            ss << "'1' hp++ ($" << std::fixed << std::setprecision(2) << 10.0f << ")\n"
               << "'2' speedCap++ ($" << player->getNextCigarettesCost() << ")\n";
            promptText.setString(ss.str());

            promptText.setPosition(sf::Vector2f({hitbox.getPosition().x, hitbox.getPosition().y - 60.0f}));

            if (buyingCooldown <= 0.0f) {
                // Health
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Num1)) {
                    if (player->spendCash(10.0f)) {
                        player->healHp(100); 
                        buyingCooldown = 0.5f;
                    }
                } 
                // Speed
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Num2)) {
                    if (player->spendCash(player->getNextCigarettesCost())) {
                        player->raiseSpeedThreshold(50.0f); 
                        buyingCooldown = 0.5f;
                    }
                } 
            }
        } else {
            std::stringstream ss;
            ss << "'1' hp++ ($" << std::fixed << std::setprecision(2) << 10.0f << ")\n"
               << "'[-]' speedCap++ (" << "[MAX]" << ")\n";
            promptText.setString(ss.str());

            promptText.setPosition(sf::Vector2f({hitbox.getPosition().x, hitbox.getPosition().y - 60.0f}));

            if (buyingCooldown <= 0.0f) {
                // Health
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Num1)) {
                    if (player->spendCash(10.0f)) {
                        player->healHp(100); 
                        buyingCooldown = 0.5f;
                    }
                } 
            }
        }
    }
}

void Stall::draw(sf::RenderWindow &window) {
    window.draw(hitbox);
    window.draw(sprite);
    
    if (isPlayerNear) {
        window.draw(promptText);
    }
}