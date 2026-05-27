#include "player.hpp"
#include "world.hpp"

// -----------------------
// ----- Projectile ------
// -----------------------
Projectile::Projectile(float startX, float startY, float radius, sf::Vector2f direction, float speed) : GameObject(startX, startY), radius(radius) {
    shape.setFillColor(sf::Color::Black);
    shape.setRadius(radius);
    shape.setPosition(sf::Vector2f({startX, startY}));

    velocity = direction * speed;
}

void Projectile::draw(sf::RenderWindow &window) {
    window.draw(shape);
}

void Projectile::update(float dt, sf::RenderWindow &window) {
    shape.move(velocity * dt);

    // Collision with objects
    sf::Vector2f pos = shape.getPosition();

    if (GameObject::world->isSolid(pos.x + radius, pos.y + radius, true, false)) {
        active = false; 
    }
}

sf::FloatRect Projectile::getBounds() const {
    return shape.getGlobalBounds();
}


// ----------------------
// ------- Player -------
// ----------------------
Player::Player(float startX, float startY) 
    : GameObject(startX, startY), animation(31, 41, 0.25f), facingRow(0), nextDamageCost(25.0f), 
    nextReloadCost(25.0f), nextVelocityCost(10.0f), cigarettesCost(50.0f), shootSound(shootBuffer), deathSound(deathBuffer) {

    if (!texture.loadFromFile("textures/Player.png")) {
        std::cerr << "Couldn't load player texture \n";
    } else {
        sprite.setTexture(texture, true); 
        sprite.setTextureRect(sf::IntRect({0, 0}, {31, 41})); 
    }

    if (shootBuffer.loadFromFile("sounds/shootingSound.mp3")) {
        shootSound.setBuffer(shootBuffer);
    }
    if (deathBuffer.loadFromFile("sounds/deathSound.mp3")) {
        deathSound.setBuffer(deathBuffer);
    }

    speed = 200.0f;

    movement_cooldown = 0.0f;
    movementBlocked_cooldown = 0.0f;
    shot_cooldown = 0.0f;
    isMoving = false;
    isDead = false;
    movementBlocked = false;

    hp = 100;
    maxHp = 100;
    damage = 30;
    level = 0;
    experience = 0;
    score = 0;
    experienceThreshold = 200;
    speed = 200;
    cash = 0.0f;

    base_shot_cooldown = 0.75f;
    bullet_velocity = 600.0f;

    overrideStartFrame = 0;
    overrideEndFrame = 0;

    sprite.setScale(sf::Vector2f({2.0f, 2.0f}));
}

void Player::draw(sf::RenderWindow &window) {
    window.draw(sprite);

    for(const auto &projectile : projectiles) {
        projectile->draw(window);
    }
}


void Player::update(float dt, sf::RenderWindow &window) {
    if(experience >= experienceThreshold) {
        levelUp();
    }

    if (movementBlocked == true) {
        movementBlocked_cooldown -= dt;
        animation.update(facingRow, overrideStartFrame, overrideEndFrame, dt, sprite);
        if (movementBlocked_cooldown <= 0) {
            movementBlocked = false;
            animation.setFrameDuration(0.25f);
        }
    }


    if(hp <= 0 && !isDead) {
        isDead = true;
        deathSound.play();
    }

    float speedDt = speed * dt;

    if (!isDead && !movementBlocked) {
        int startFrame = 0;
        int endFrame = 0;   

        if (movement_cooldown <= 0 && !movementBlocked) {
            isMoving = false;

            sf::Vector2f nextPos = sprite.getPosition();

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::W)) {
                nextPos.y -= speedDt; 
                isMoving = true;
            } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::S)) {
                nextPos.y += speedDt;
                isMoving = true;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::A)) {
                nextPos.x -= speedDt;
                isMoving = true;
                facingRow = 1;
            } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::D)) {
                nextPos.x += speedDt;
                isMoving = true;
                facingRow = 0;
            }

            // Collision detection
            if (isMoving) {
                startFrame = 1;
                endFrame = 2;

                if (GameObject::world != nullptr) {

                    sf::FloatRect bounds = sprite.getGlobalBounds();
                    
                    // To keep the collision point at the bottom-center of the player
                    float offsetX = bounds.size.x / 2.0f; 
                    float offsetY = bounds.size.y; 

                    // coordinates of the feet for the next position
                    float feetX = nextPos.x + offsetX;
                    float feetY = nextPos.y + offsetY;
                    
                    sf::Vector2f currentPos = sprite.getPosition();

                    // Movement without obstructions
                    if (!GameObject::world->isSolid(feetX, feetY, false, false)) {
                        sprite.setPosition(nextPos);
                    } else {
                        // Try moving to the side
                        if (!GameObject::world->isSolid(nextPos.x + offsetX, currentPos.y + offsetY, false, false)) {
                            sprite.setPosition({nextPos.x, currentPos.y});
                        } 
                        // Try moving up or down
                        else if (!GameObject::world->isSolid(currentPos.x + offsetX, nextPos.y + offsetY, false, false)) {
                            sprite.setPosition({currentPos.x, nextPos.y});
                        } 
                        // Movement completely blocked
                        else {
                            startFrame = 0;
                            endFrame = 0;
                        }
                    }
                }
            }

            // Attack
            if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
                sprite.move(sf::Vector2f(0.0f, 0.0f));
                startFrame = 3;
                endFrame = 3;
                movement_cooldown = 0.15f;
                isMoving = false;

                sf::Vector2f current_position = sprite.getPosition();

                // Spawn of the bullet relative to the player's sprite 
                // Adjustments necessary in case of changing player's scale
                float spawnX = current_position.x;
                float spawnY = current_position.y + 20.0f;

                sf::Vector2i mousePosInt = sf::Mouse::getPosition(window);
                // Makes it work with movable camera
                sf::Vector2f mousePos = window.mapPixelToCoords(mousePosInt);

                // Turns the player depending on if he's clicking on the left or right side relative to the sprite
                if (mousePos.x < spawnX + 30.0f) {
                    facingRow = 1;
                } else {
                    facingRow = 0;
                }

                // adds offset so it shoots from the gun instead of the center
                if (facingRow == 0) {
                    spawnX += 60.0f;
                }

                // direction vector // calculates the direcion the bullet is supposed to go
                sf::Vector2f direction = mousePos - sf::Vector2f(spawnX, spawnY);

                // makes direction x, y = 1 at max so I can multiply it by speed
                float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
                if(length != 0.0f) {
                    direction.x /= length;
                    direction.y /= length;
                }

                if (shot_cooldown <= 0) {
                    shootSound.play();
                    projectiles.push_back(std::make_unique<Projectile>(spawnX, spawnY, 2.0f, direction, bullet_velocity));
                    shot_cooldown = base_shot_cooldown; 
                }
            }

            if (shot_cooldown > 0) {
                shot_cooldown -= dt;
            }

            animation.update(facingRow, startFrame, endFrame, dt, sprite);
        } else {
            movement_cooldown -= dt;
            startFrame = 3;
            endFrame = 3;
        }
    } else if (!movementBlocked) {
        if(facingRow == 0) {
            sprite.setTextureRect(sf::IntRect({0, 82}, {60, 18}));
        } else {
            sprite.setTextureRect(sf::IntRect({60, 82}, {60, 18}));
        }
        
    }

    // Makes the projectiles actually appear
    for (const auto &projectile : projectiles) {
        projectile->update(dt, window);
    }

    // Deletes the projectiles that hit something
    projectiles.erase(
        std::remove_if(projectiles.begin(), projectiles.end(),
            [](const std::unique_ptr<Projectile>& p) { return !p->active; }),
        projectiles.end()
    );
}

void Player::setAnimation(float dt, int startFrame, int endFrame, float totalDuration, float frameDuration) {
    overrideStartFrame = startFrame;
    overrideEndFrame = endFrame;
    movementBlocked = true;
    movementBlocked_cooldown = totalDuration;

    animation.setFrameDuration(frameDuration);
}

void Player::levelUp() {
    experience -= experienceThreshold;
    level += 1;
    experienceThreshold = 200 + level * 100;

    maxHp = 100 + 15 * level;
    hp += 15;
}

void Player::raiseSpeed(int amount) {
    speed += amount;

    cigarettesCost += 100.0f;
}

bool Player::spendCash(float amount) {
    if (cash >= amount) {
        cash -= amount;
        return true;
    }
    return false;
}

void Player::upgradeDamage(int amount) {
    damage += amount;
    nextDamageCost *= 1.5f;
}

void Player::upgradeReloadSpeed(float amount) {
    if (base_shot_cooldown > 0.15f) {
        base_shot_cooldown -= amount;
        nextReloadCost *= 1.5f;
    }
}

void Player::upgradeBulletVelocity(float amount) {
    bullet_velocity += amount;
    nextVelocityCost += 10.0f;
}

void Player::setPosition(sf::Vector2f Pos) {
    sprite.setPosition(Pos);
}

void Player::addExperience(int amount) {
    experience += amount;
    score += amount;
}

void Player::addCash(float amount) {
    cash += amount;
}

void Player::takeDamage(int amount) {
    hp -= amount;
}

void Player::healHp(int amount) {
    if(hp + amount >= maxHp) {
        hp = maxHp;
    } else {
        hp += amount;
    }
}



// Getters
float Player::getNextDamageCost() const { return nextDamageCost; }

float Player::getNextReloadCost() const { return nextReloadCost; }

float Player::getNextVelocityCost() const { return nextVelocityCost; }

float Player::getNextCigarettesCost() const { return cigarettesCost; }

bool Player::isAttacking() const { return movement_cooldown > 0.0f; }

bool Player::checkIfDead() const { return isDead; }

sf::Vector2f Player::getPosition() const { return sprite.getPosition(); }

int Player::getDamage() const { return damage; }

int Player::getHp() const { return hp; }

int Player::getMaxHp() const { return maxHp; }

int Player::getExperienceThreshold() const { return experienceThreshold; }

int Player::getSpeed() const { return speed; }

int Player::getExperience() const { return experience; }

int Player::getScore() const { return score; }

int Player::getLevel() const { return level; }

float Player::getCash() const { return cash; }

sf::FloatRect Player::getBounds() const { return sprite.getGlobalBounds(); }

std::vector<std::unique_ptr<Projectile>>& Player::getProjectiles() { return projectiles; }