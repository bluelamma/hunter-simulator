#include "entities.hpp"
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
}

sf::FloatRect Projectile::getBounds() const {
    return shape.getGlobalBounds();
}



// ----------------------
// -------- Hare --------
// ----------------------
Hare::Hare(float startX, float startY, Player *player) 
    : GameObject(startX, startY), animation(46.0f, 36.0f, 0.15f), facingRow(0), moveTimer(0.0f), moveInterval(0.0f), velocity({0.0f, 0.0f}), speed(50.0f), playerTarget(player) {
    if (!texture.loadFromFile("textures/Hare.png")) {
        std::cerr << "Couldn't load hare texture \n";
    } else {
        sprite.setTexture(texture, true); 
    }

    sf::Vector2f harePos = sprite.getPosition();
    if (rand() % 20 < 1) {
        hp = rand() % 50 + 40;
    } else {
        hp = rand() % 30 + 30;
    }
    maxHp = hp;
    difference = maxHp / 45.0f;
    speed *= difference;

    hitbox.setSize(sf::Vector2f(64.0f * 0.5f * difference, 64.0f * 0.5f * difference));
    hitbox.setPosition(sf::Vector2f(harePos.x + 0.0f, harePos.y + 0.0f));
    hitbox.setFillColor(sf::Color(255, 0, 0, 0)); // set last one to 100 for debugging

    sprite.setScale(sf::Vector2f({0.75f * difference, 0.75f * difference}));
}

void Hare::draw(sf::RenderWindow &window) {
    window.draw(sprite);
    window.draw(hitbox);
}

void Hare::takeDamage(int amount) {
    hp -= amount;
    panicTimer = 5.0f;
    
    if (hp <= 0) {
        active = false; 
    }
}

int Hare::getHp() const {
    return hp;
}

sf::FloatRect Hare::getBounds() const {
    return hitbox.getGlobalBounds(); 
}

float Hare::getDiff() const {
    return difference; 
}

void Hare::update(float dt, sf::RenderWindow &window) {
    int startFrame = 0;
    int endFrame = 1;
    isMoving = false;

    sf::Vector2f harePos = sprite.getPosition();
    sf::Vector2f playerPos = playerTarget->getPosition();

    // Distance between hare and the player
    float dx = harePos.x - playerPos.x;
    float dy = harePos.y - playerPos.y;
    float distance = std::sqrt(dx * dx + dy * dy);

    float fleeRadius = playerTarget->isAttacking() ? 600.0f : 250.0f;

    if(panicTimer > 0) {
        panicTimer -= dt;
    }

    if (distance <= fleeRadius || panicTimer >= 0.0f) {  
        // Vector pointing away from the player
        sf::Vector2f dir(dx, dy); 

        float length = std::sqrt(dir.x * dir.x + dir.y * dir.y);
        if (length != 0.0f) {
            dir.x /= length;
            dir.y /= length;
        }

        // If attacked runs 6 times faster than normally unless wounded
        if (hp <= maxHp * 0.3) {
            velocity = dir * (speed * 0.5f);
        } else {
            velocity = dir * (speed * 6.0f);
        }

        if (velocity.x < 0) {
            facingRow = 0; // Left
        } else if (velocity.x > 0) {
            facingRow = 1; // Right
        }


        endFrame = 1;
        moveTimer = 0.0f;
    } else {
        moveTimer += dt;

        if (moveTimer >= moveInterval) {
            moveTimer = 0.0f;

            float dirX = static_cast<float>((rand() % 3) - 1); 
            float dirY = static_cast<float>((rand() % 3) - 1);

            float length = std::sqrt(dirX * dirX + dirY * dirY);
            if (length != 0.0f) {
                dirX /= length;
                dirY /= length;
            }

            velocity = sf::Vector2f(dirX * speed, dirY * speed);

            if(rand() % 5 <= 1) {
                velocity = sf::Vector2f(0.0f, 0.0f);
            }

            if (velocity.x < 0) {
                facingRow = 0; // Left
            } else if (velocity.x > 0) {
                facingRow = 1; // Right
            }

            moveInterval = 1.0f + static_cast<float>(rand() % 3);
            }
        }
    
    if(velocity.x != 0.0f || velocity.y != 0.0f) {
            isMoving = true;
        } 

    if(isMoving) {
        sf::Vector2f currentPos = sprite.getPosition();
        sf::Vector2f nextPos = currentPos + (velocity * dt);

        if (GameObject::world != nullptr) {

            sf::FloatRect bounds = sprite.getGlobalBounds();
                
            float offsetX = 5.0f;
            float offsetY = bounds.size.y; 

            if (facingRow == 0) {
                offsetX = bounds.size.x / 2.0f - 10.0f; 
            } else {
                offsetX = bounds.size.x / 2.0f + 10.0f; 
            }

            // coordinates of the feet
            float feetX = nextPos.x + offsetX;
            float feetY = nextPos.y + offsetY;

            // Checks if the feet are touching solid
            if (!GameObject::world->isSolid(feetX, feetY)) {
                sprite.setPosition(nextPos);
            } else {
                // Try going up if doesn't work try going horizontally 
                if (!GameObject::world->isSolid(currentPos.x + offsetX, feetY)) {
                    sprite.setPosition({currentPos.x, nextPos.y});
                    moveTimer = moveInterval;
                } else if (!GameObject::world->isSolid(feetX, currentPos.y + offsetY)) {
                    sprite.setPosition({nextPos.x, currentPos.y});
                    moveTimer = moveInterval;
                } else {
                    endFrame = 0;
                    moveTimer = moveInterval;
                }
            }
        }
    } else {
        endFrame = 0;
    }

    hitbox.setPosition(sf::Vector2f(harePos.x + 0.0f, harePos.y + 0.0f));
    animation.update(facingRow, startFrame, endFrame, dt, sprite);
}



// ----------------------
// ------- Player -------
// ----------------------
Player::Player(float startX, float startY) 
: GameObject(startX, startY), animation(31, 41, 0.25f), facingRow(0) {
    if (!texture.loadFromFile("textures/Player.png")) {
        std::cerr << "Couldn't load player texture \n";
    } else {
        sprite.setTexture(texture, true); 
        sprite.setTextureRect(sf::IntRect({0, 0}, {31, 41})); 
    }

    speed = 200.0f;

    movement_cooldown = 0.0f;
    shot_cooldown = 0.0f;
    isMoving = false;
    isDead = false;

    hp = 100;
    maxHp = 100;
    damage = 30;
    level = 0;
    experience = 0;
    experienceThreshold = 200;
    cash = 0.0f;
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

    // Movement and attacking
    float speedDt = speed * dt;
    int startFrame = 0;
    int endFrame = 0;   

    if (!isDead) {
        if (movement_cooldown <= 0) {
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
                    if (!GameObject::world->isSolid(feetX, feetY)) {
                        sprite.setPosition(nextPos);
                    } else {
                        // Try moving to the side
                        if (!GameObject::world->isSolid(nextPos.x + offsetX, currentPos.y + offsetY)) {
                            sprite.setPosition({nextPos.x, currentPos.y});
                        } 
                        // Try moving up or down
                        else if (!GameObject::world->isSolid(currentPos.x + offsetX, nextPos.y + offsetY)) {
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
                    projectiles.push_back(std::make_unique<Projectile>(spawnX, spawnY, 2.0f, direction, 600.0f));
                    shot_cooldown = 0.75; 
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
    } else {
        sprite.setTextureRect(sf::IntRect({0, 82}, {60, 18}));
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

void Player::levelUp() {
    experience -= experienceThreshold;
    level += 1;
    experienceThreshold = 200 + level * 100;

    if (speed < 500.0f) {
        speed += 25.0f;
    }

    hp = 100 + 10 * level;
    maxHp = 100 + 10 * level;
}

void Player::setPosition(sf::Vector2f Pos) {
    sprite.setPosition(Pos);
}

void Player::addExperience(int amount) {
    experience += amount;
}

bool Player::isAttacking() const {
    return movement_cooldown > 0.0f;
}

bool Player::checkIfDead() const {
    return isDead;
}

sf::Vector2f Player::getPosition() const {
    return sprite.getPosition();
}

int Player::getDamage() const {
    return damage;
}

int Player::getHp() const {
    return hp;
}

int Player::getMaxHp() const {
    return maxHp;
}

int Player::getExperienceThreshold() const {
    return experienceThreshold;
}

int Player::getExperience() const {
    return experience;
}

void Player::addCash(float amount) {
    cash += amount;
}

float Player::getCash() const {
    return cash;
}

std::vector<std::unique_ptr<Projectile>>& Player::getProjectiles() {
    return projectiles;
}