#include "entities.hpp"
#include "world.hpp"
#include "player.hpp"
#include "pickup.hpp"

// -----------------------
// ------ Creature -------
// -----------------------
Creature::Creature(float startX, float startY, Player *player, float animWidth, float animHeight, float animHoldTime)
    : GameObject(startX, startY), player(player), animation(animWidth, animHeight, animHoldTime), 
      velocity({0.0f, 0.0f}), speed(0.0f), isMoving(false), facingRow(0), hp(0), maxHp(0), difference(1.0f), isDead(false), isAttacking(false) {}

void Creature::draw(sf::RenderWindow &window) {
    window.draw(sprite);
    window.draw(hitbox);
}

void Creature::takeDamage(int amount) {
    hp -= amount;
    if (hp <= 0) {
        isDead = true; 
    }
}

int Creature::getHp() const { return hp; }
float Creature::getDiff() const { return difference; }
float Creature::getDeathTimer() const { return deathTimer; }
bool Creature::checkIfDead() const { return isDead; }
sf::FloatRect Creature::getBounds() const { return hitbox.getGlobalBounds(); }



// ----------------------
// -------- Hare --------
// ----------------------
Hare::Hare(float startX, float startY, Player *player) 
    : Creature(startX, startY, player, 46.0f, 36.0f, 0.15f), 
      moveTimer(0.0f), moveInterval(0.0f), panicTimer(0.0f), bounceTimer(0.0f) {
    
    if (!texture.loadFromFile("textures/Hare.png")) {
        std::cerr << "Couldn't load hare texture \n";
    } else {
        sprite.setTexture(texture, true); 
    }

    deathTimer = 0.0f;
    sf::Vector2f harePos = sprite.getPosition();
    
    if (rand() % 20 <= 1) {
        hp = rand() % 50 + 40;
    } else {
        hp = rand() % 30 + 30;
    }
    
    maxHp = hp;
    difference = maxHp / 45.0f;
    speed = 50.0f * difference;

    hitbox.setSize(sf::Vector2f(64.0f * 0.5f * difference, 64.0f * 0.5f * difference));
    hitbox.setPosition(sf::Vector2f(harePos.x, harePos.y));
    hitbox.setFillColor(sf::Color(255, 0, 0, 0));

    sprite.setScale(sf::Vector2f({0.75f * difference, 0.75f * difference}));
}

void Hare::update(float dt, sf::RenderWindow &window) {
    startFrame = 0;
    endFrame = 1;
    isMoving = false;

    sf::Vector2f harePos = sprite.getPosition();
    sf::Vector2f playerPos = player->getPosition();

    if(!isDead) {
        // Distance between hare and the player
        float dx = harePos.x - playerPos.x;
        float dy = harePos.y - playerPos.y;
        float distance = std::sqrt(dx * dx + dy * dy);

        float fleeRadius = player->isAttacking() ? 600.0f : 250.0f;
        bool isFleeing = (distance <= fleeRadius || panicTimer > 0.0f);

        // Updates timers
        if (panicTimer > 0.0f) panicTimer -= dt;
        if (bounceTimer > 0.0f) bounceTimer -= dt; 

        if (isFleeing) {  
            bounceTimer = 0.0f; // Cancels bounce if panicking so it can run away properly

            sf::Vector2f dir(dx, dy); 
            float length = std::sqrt(dir.x * dir.x + dir.y * dir.y);
            if (length != 0.0f) {
                dir.x /= length;
                dir.y /= length;
            }

            if (hp <= maxHp * 0.3) {
                velocity = dir * (speed * 0.5f);
            } else {
                velocity = dir * (speed * 6.0f);
            }

            if (velocity.x < 0) facingRow = 0; 
            else if (velocity.x > 0) facingRow = 1; 

            endFrame = 1;
            moveTimer = 0.0f;
        } 
        else if (bounceTimer > 0.0f) {
            // DVD state
            endFrame = 1;
        } 
        else {
            // Wandering state
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

                if (velocity.x < 0) facingRow = 0;
                else if (velocity.x > 0) facingRow = 1;

                moveInterval = 1.0f + static_cast<float>(rand() % 3);
            }
        }
        
        if (velocity.x != 0.0f || velocity.y != 0.0f) {
            isMoving = true;
        } 

        if (isMoving) {
            sf::Vector2f currentPos = sprite.getPosition();
            sf::Vector2f nextPos = currentPos + (velocity * dt);

            if (GameObject::world != nullptr) {
                sf::FloatRect bounds = sprite.getGlobalBounds();
                    
                float offsetX = 5.0f;
                float offsetY = bounds.size.y; 

                if (facingRow == 0) offsetX = bounds.size.x / 2.0f - 10.0f; 
                else offsetX = bounds.size.x / 2.0f + 10.0f; 

                float feetX = nextPos.x + offsetX;
                float feetY = nextPos.y + offsetY;

                // Collision check
                if (!GameObject::world->isSolid(feetX, feetY, false)) {
                    sprite.setPosition(nextPos);
                } else {
                    if (!isFleeing) {
                        bounceTimer = 1.5f; // Enter bounce state to prevent getting stuck on water

                        bool hitHorizontal = GameObject::world->isSolid(feetX, currentPos.y + offsetY, false);
                        bool hitVertical = GameObject::world->isSolid(currentPos.x + offsetX, feetY, false);

                        if (hitHorizontal) velocity.x *= -1.0f; 
                        if (hitVertical) velocity.y *= -1.0f;   
                        
                        if (!hitHorizontal && !hitVertical) {
                            velocity.x *= -1.0f;
                            velocity.y *= -1.0f;
                        }

                        // If it hits water while moving straight, gives it a diagonal push
                        if (velocity.x == 0.0f) velocity.x = (rand() % 2 == 0 ? 1.0f : -1.0f) * speed;
                        if (velocity.y == 0.0f) velocity.y = (rand() % 2 == 0 ? 1.0f : -1.0f) * speed;

                        if (velocity.x < 0) facingRow = 0;
                        else if (velocity.x > 0) facingRow = 1;

                    } else {
                        if (!GameObject::world->isSolid(currentPos.x + offsetX, feetY, false)) {
                            sprite.setPosition({currentPos.x, nextPos.y});
                        } else if (!GameObject::world->isSolid(feetX, currentPos.y + offsetY, false)) {
                            sprite.setPosition({nextPos.x, currentPos.y});
                        } else {
                            endFrame = 0;
                        }
                    }
                }
            }
        } else {
            endFrame = 0;
        }
    } else {
        // Death animation
        startFrame = 2;
        endFrame = 2;
        deathTimer += dt;

        float fadeRatio = 1.0f - (deathTimer / 3.0f);
        
        // Prevents the alpha from dropping below 0
        if (fadeRatio < 0.0f) fadeRatio = 0.0f;

        // 255, 255, 255 maintains the texture's original colors // alpha - 0 = invisible, 255 = visible
        sprite.setColor(sf::Color(255, 255, 255, 255 - 255 * (deathTimer / 2)));

        if(deathTimer >= 2.0f) {
            active = false;
        }
    }

    hitbox.setPosition(sf::Vector2f(harePos.x + 0.0f, harePos.y + 0.0f));
    // Animation::update(int row, int startFrame, int endFrame, float dt, sf::Sprite &sprite);
    animation.update(facingRow, startFrame, endFrame, dt, sprite);
}

void Hare::grantRewards(std::vector<std::unique_ptr<GameObject>>& newDrops) {
    player->addExperience(50 * difference); 
    player->addCash(6.0f * difference);
}

// ----------------------
// -------- Boar --------
// ----------------------
Boar::Boar(float startX, float startY, Player *player) 
    : Creature(startX, startY, player, 45.0f, 28.0f, 0.25f), moveTimer(0.0f), moveInterval(0.75f), 
    bounceTimer(0.0f), attackTimer(1.0f), turningLocked(false), idleTimer(0.0f), pursuitTimer(0.0f) {
    if (!texture.loadFromFile("textures/boar.png")) {
        std::cerr << "Couldn't load boar texture \n";
    } else {
        sprite.setTexture(texture, true); 
    }

    deathTimer = 0.0f;
    sf::Vector2f boarPos = sprite.getPosition();
    
    if (rand() % 20 < 1) {
        hp = rand() % 450 + 300;
        maxHp = hp;
        difference = maxHp / 150.0f;
    } else {
        hp = rand() % 150 + 100;
        maxHp = hp;
        difference = maxHp / 100.0f;
    }

    if (rand() % 20 <= 1) {
        attackCooldown = 0.6f;
        speed = 80.0f;
    } else if (rand() % 10 <= 1) {
        attackCooldown = 0.7f;
        speed = 70.0f;
    } else if (rand() % 5 <= 1) {
        attackCooldown = 0.8f;
        speed = 65.0f;
    } else if (rand() % 5 <= 1) {
        attackCooldown = 0.9f;
        speed = 60.0f;
    } else {
        attackCooldown = 1.0f;
        speed = 55.0f;
    }

    attackDamage = 50 * difference;
    aggroRadius = 150.0f + static_cast<float>(rand() % 200);

    hitbox.setSize(sf::Vector2f(32.0f * difference, 18.0f * difference));
    hitbox.setFillColor(sf::Color(255, 0, 0, 0));

    sprite.setScale(sf::Vector2f({1.0f * difference, 1.0f * difference}));
}

void Boar::update(float dt, sf::RenderWindow &window) {
    startFrame = 1;
    endFrame = 2;
    isMoving = false;

    sf::Vector2f boarPos = sprite.getPosition();
    sf::Vector2f playerPos = player->getPosition();

    if(idleTimer >= 0) {
        idleTimer -= dt;
    } 

    if(hp != maxHp && !aggroed) {
        aggroed = true;
        pursuitTimer = 3.0f; 
    }

    if(!isDead) {
        if(idleTimer != 0) {
            attackTimer += dt;
            bool isCurrentlyAttacking = (attackTimer < 0.5f);
            float impactTime = 0.3f; 

            if (isCurrentlyAttacking) {
                startFrame = 3;
                endFrame = 4;
                
                if (attackTimer < impactTime) {
                    isMoving = true; 
                    
                    float dirX = playerPos.x - boarPos.x;
                    float dirY = playerPos.y - boarPos.y;
                    float length = std::sqrt(dirX * dirX + dirY * dirY);
                    
                    if (length != 0.0f) {
                        velocity = sf::Vector2f((dirX / length) * (speed * 8.0f), 
                                                (dirY / length) * (speed * 8.0f));
                    }
                } else {
                    isMoving = false;
                    idleTimer = static_cast<float>(rand() % 5) / 10.0f;
                    velocity = sf::Vector2f(0.0f, 0.0f);
                }
            } else {
                
                // Aggro logic
                float diffX = playerPos.x - boarPos.x;
                float diffY = playerPos.y - boarPos.y;
                float distanceToPlayer = std::sqrt(diffX * diffX + diffY * diffY);
                float updatedRadius = player->isAttacking() ? aggroRadius * 1.75f : aggroRadius;

                if (!player->checkIfDead() && distanceToPlayer < updatedRadius) {
                    // Player is in range: stays aggroed and keeps resetting the chase timer
                    aggroed = true;
                    pursuitTimer = 3.0f; 
                } else if (aggroed) {
                    // Player is out of range: counts down the pursuit timer
                    pursuitTimer -= dt;
                    if (pursuitTimer <= 0.0f) {
                        aggroed = false;
                        hp = maxHp; // Regains HP instantly
                    }
                }

                if (velocity.x == 0.0f) velocity.x = (rand() % 2 == 0 ? 1.0f : -1.0f) * speed;

                if (aggroed && !player->checkIfDead()) {
                    // Chasing the player
                    if (distanceToPlayer != 0.0f) {
                        velocity = sf::Vector2f((diffX / distanceToPlayer) * (speed * 6.5f), 
                                                (diffY / distanceToPlayer) * (speed * 6.5f));
                    }
                } else {
                    // Wandering mode
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

                        moveInterval = 1.0f + static_cast<float>(rand() % 3);
                    }
                }

                if (velocity.x != 0.0f || velocity.y != 0.0f) {
                    isMoving = true;
                } 

                if(velocity.x < 0) {
                    facingRow = 1;
                } else if (velocity.x > 0) {
                    facingRow = 0;
                }
            }

            // Check collisions
            if (isMoving) {
                sf::Vector2f currentPos = sprite.getPosition();
                sf::Vector2f nextPos = currentPos + (velocity * dt);

                if (GameObject::world != nullptr) {
                    sf::FloatRect bounds = sprite.getGlobalBounds();
                            
                    float offsetX = bounds.size.x / 2.0f;
                    float offsetY = bounds.size.y; 

                    float feetX = nextPos.x + offsetX;
                    float feetY = nextPos.y + offsetY;

                    // Try normal movement
                    if (!GameObject::world->isSolid(feetX, feetY, false)) {
                        sprite.setPosition(nextPos);
                    } else {
                        bool slid = false;

                        // Try moving horizontally
                        if (!GameObject::world->isSolid(nextPos.x + offsetX, currentPos.y + offsetY, false)) {
                            sprite.setPosition({nextPos.x, currentPos.y});
                            slid = true;
                        } 
                        // Try moving vertically 
                        else if (!GameObject::world->isSolid(currentPos.x + offsetX, nextPos.y + offsetY, false)) {
                            sprite.setPosition({currentPos.x, nextPos.y});
                            slid = true;
                        }

                        // No bounce if aggroed, chase logic overwrites velocity anyway
                        if (!aggroed) {
                            bounceTimer = 1.5f; 

                            bool hitHorizontal = GameObject::world->isSolid(feetX, currentPos.y + offsetY, false);
                            bool hitVertical = GameObject::world->isSolid(currentPos.x + offsetX, feetY, false);

                            if (hitHorizontal) velocity.x *= -1.0f; 
                            if (hitVertical) velocity.y *= -1.0f;   
                                    
                            if (!hitHorizontal && !hitVertical) {
                                velocity.x *= -1.0f;
                                velocity.y *= -1.0f;
                            }

                            if (velocity.x == 0.0f) velocity.x = (rand() % 2 == 0 ? 1.0f : -1.0f) * speed;
                            if (velocity.y == 0.0f) velocity.y = (rand() % 2 == 0 ? 1.0f : -1.0f) * speed;

                            if (velocity.x < 0) facingRow = 0;
                            else if (velocity.x > 0) facingRow = 1;    
                        }
                    } 
                }
            } else if (!isCurrentlyAttacking) {
                startFrame = 0; 
                endFrame = 0;
            }

            // Handles attack and damage
            if (!player->checkIfDead()) {
                auto intersection = hitbox.getGlobalBounds().findIntersection(player->getBounds());
                
                if (intersection.has_value() && attackTimer >= attackCooldown) {
                    attackTimer = 0.0f;               
                }

                if (attackTimer >= impactTime && (attackTimer - dt) < impactTime) {
                    float dx = playerPos.x - boarPos.x;
                    float dy = playerPos.y - boarPos.y;
                    float distToPlayer = std::sqrt(dx * dx + dy * dy);

                    if (intersection.has_value() || distToPlayer < 80.0f) {
                        player->takeDamage(attackDamage); 
                    }
                }
            }
        }
    } else {
        // Death animation
        startFrame = 5;
        endFrame = 5;
        deathTimer += dt;

        float fadeRatio = 1.0f - (deathTimer / 3.0f);
        if (fadeRatio < 0.0f) fadeRatio = 0.0f;

        sprite.setColor(sf::Color(255, 255, 255, 255 - 255 * (deathTimer / 2)));

        if(deathTimer >= 2.0f) {
            active = false;
        }
    }

    if(facingRow == 0) {
        hitbox.setPosition(sf::Vector2f(boarPos.x + 8.0f, boarPos.y + 10.0f));
    } else {
        hitbox.setPosition(sf::Vector2f(boarPos.x + 12.0f * difference, boarPos.y + 10.0f));
    }

    animation.update(facingRow, startFrame, endFrame, dt, sprite);
}

void Boar::grantRewards(std::vector<std::unique_ptr<GameObject>>& newDrops) {
    player->addExperience(100 * difference); 
    player->addCash(25.0f * difference);

    if(rand() % 5 <= 1) {
        sf::FloatRect bounds = getBounds();
        auto meat = std::make_unique<RawMeat>(bounds.position.x, bounds.position.y, player, difference);
        meat->setSpriteScale(sf::Vector2f{0.5f * difference, 0.5f * difference});
        newDrops.push_back(std::move(meat));
    }
}

// ----------------------
// -------- Bear --------
// ----------------------
Bear::Bear(float startX, float startY, Player *player, bool boss) 
    : Creature(startX, startY, player, 63.0f, 40.0f, 0.35f), // size of the sprite
      moveTimer(0.0f), moveInterval(1.0f), bounceTimer(0.0f), attackTimer(1.5f), 
      turningLocked(false), idleTimer(0.0f), pursuitTimer(0.0f), aggroed(false), boss(boss), bearSound(bearBuffer) {
    
    if (!boss) {
        if (!texture.loadFromFile("textures/bear.png")) {
            std::cerr << "Couldn't load bear texture \n";
        } else {
            sprite.setTexture(texture, true); 
        }

            hp = rand() % 1500 + 500;
            maxHp = hp;
            difference = maxHp / 500.0f;
            attackDamage = 90 * difference;
    } else {
            if (!texture.loadFromFile("textures/bear_black.png")) {
                std::cerr << "Couldn't load bear texture \n";
            } else {
            sprite.setTexture(texture, true); 
            }

            hp = 3000;
            maxHp = hp;
            difference = maxHp / 750.0f;
            attackDamage = 100 * difference;
    }

    if (bearBuffer.loadFromFile("sounds/bearSound.mp3")) {
        bearSound.setBuffer(bearBuffer);
    }

    deathTimer = 0.0f;
    sf::Vector2f bearPos = sprite.getPosition();
    
    attackCooldown = 1.5f;
    speed = 45.0f; 
    aggroRadius = 275.0f;

    hitbox.setSize(sf::Vector2f(48.0f * difference, 32.0f * difference));
    hitbox.setFillColor(sf::Color(255, 0, 0, 0));

    sprite.setScale(sf::Vector2f({1.0f * difference, 1.0f * difference}));
}

void Bear::update(float dt, sf::RenderWindow &window) {
    startFrame = 0; 
    endFrame = 0;
    isMoving = false;

    sf::Vector2f bearPos = sprite.getPosition();
    sf::Vector2f playerPos = player->getPosition();

    if (!isDead) {
        // attackTimer acts as both the cooldown tracker and animation timer
        attackTimer += dt;
        
        // Play attack animation for 0.5s after an attack triggers
        bool isAttackingAnim = (attackTimer < 0.5f);

        if (isAttackingAnim) {
            startFrame = 3; // Attack frame
            endFrame = 3;
            // Stops for a moment while doing the attack animation
            velocity = sf::Vector2f(0.0f, 0.0f); 
        } else {
            // --- Aggro Logic ---
            float diffX = playerPos.x - bearPos.x;
            float diffY = playerPos.y - bearPos.y;
            float distanceToPlayer = std::sqrt(diffX * diffX + diffY * diffY);
            float updatedRadius = player->isAttacking() ? aggroRadius * 2.0f : aggroRadius;

            if ((!player->checkIfDead() && distanceToPlayer < updatedRadius) || hp != maxHp) {
                if (!aggroed) {
                    bearSound.play(); 
                }
                
                aggroed = true;
                pursuitTimer = 4.0f;
            } else if (aggroed) {
                pursuitTimer -= dt;
                if (pursuitTimer <= 0.0f) {
                    aggroed = false;
                    hp = maxHp; 
                }
            }

            // --- Roaming & Chasing Logic ---
            // Initialize a random starting direction if completely stationary
            if (velocity.x == 0.0f && velocity.y == 0.0f) {
                float angle = static_cast<float>(rand() % 360) * 3.14159f / 180.0f;
                velocity = sf::Vector2f(std::cos(angle) * speed, std::sin(angle) * speed);
            }

            if (aggroed && !player->checkIfDead() && distanceToPlayer != 0.0f) {
                // Chase the player
                velocity = sf::Vector2f((diffX / distanceToPlayer) * (speed * 10.0f), 
                                        (diffY / distanceToPlayer) * (speed * 10.0f));
            } else if (!aggroed) {
                // Roam constantly
                float currentSpeed = std::sqrt(velocity.x * velocity.x + velocity.y * velocity.y);
                if (currentSpeed != 0.0f) {
                    velocity.x = (velocity.x / currentSpeed) * speed;
                    velocity.y = (velocity.y / currentSpeed) * speed;
                }
            }

            isMoving = true;
            startFrame = 1; // Walking frames
            endFrame = 2;

            // Facing direction
            if (velocity.x < 0) {
                facingRow = 1; // Left
            } else if (velocity.x > 0) {
                facingRow = 0; // Right
            }
        }

        // Collision and movement
        if (isMoving && (velocity.x != 0.0f || velocity.y != 0.0f)) {
            sf::Vector2f currentPos = sprite.getPosition();
            sf::Vector2f nextPos = currentPos + (velocity * dt);

            if (GameObject::world != nullptr) {
                sf::FloatRect bounds = sprite.getGlobalBounds();
                
                float offsetX = bounds.size.x / 2.0f;
                float offsetY = bounds.size.y; 

                float feetX = nextPos.x + offsetX;
                float feetY = nextPos.y + offsetY;

                if (!GameObject::world->isSolid(feetX, feetY, false)) {
                    sprite.setPosition(nextPos);
                } else {
                    // Bounce Logic - Inverts velocity based on which axis hit the wall
                    bool hitHorizontal = GameObject::world->isSolid(feetX, currentPos.y + offsetY, false);
                    bool hitVertical = GameObject::world->isSolid(currentPos.x + offsetX, feetY, false);

                    if (hitHorizontal) velocity.x *= -1.0f; 
                    if (hitVertical) velocity.y *= -1.0f;   
                            
                    // Corner case flip
                    if (!hitHorizontal && !hitVertical) {
                        velocity.x *= -1.0f;
                        velocity.y *= -1.0f;
                    }

                    // Sliding application so it doesn't get stuck visually
                    if (!GameObject::world->isSolid(nextPos.x + offsetX, currentPos.y + offsetY, false)) {
                        sprite.setPosition({nextPos.x, currentPos.y});
                    } else if (!GameObject::world->isSolid(currentPos.x + offsetX, nextPos.y + offsetY, false)) {
                        sprite.setPosition({currentPos.x, nextPos.y});
                    }
                }
            } else {
                sprite.setPosition(nextPos); // Fallback if no world
            }
        }

        // Attack
        if (!player->checkIfDead()) {
            auto intersection = hitbox.getGlobalBounds().findIntersection(player->getBounds());
            
            // If touching the player and cooldown is ready
            if (intersection.has_value() && attackTimer >= attackCooldown) {
                player->takeDamage(attackDamage); // Instant damage
                attackTimer = 0.0f; // Resetting triggers the attack animation visually
            }
        }
    } else {
        // Death
        startFrame = 4;
        endFrame = 4;
        deathTimer += dt;

        float fadeRatio = 1.0f - (deathTimer / 3.0f);
        if (fadeRatio < 0.0f) fadeRatio = 0.0f;

        sprite.setColor(sf::Color(255, 255, 255, 255 - 255 * (deathTimer / 2)));

        if (deathTimer >= 2.0f) {
            active = false;
        }
    }

    // Hitbox and animation
    bearPos = sprite.getPosition(); // Update position after any movement
    
    if(facingRow == 0) {
        hitbox.setPosition(sf::Vector2f(bearPos.x + 4.0f * difference, bearPos.y + 16.0f));
    } else {
        hitbox.setPosition(sf::Vector2f(bearPos.x + 7.0f * difference, bearPos.y + 16.0f));
    }

    animation.update(facingRow, startFrame, endFrame, dt, sprite);
}

void Bear::grantRewards(std::vector<std::unique_ptr<GameObject>>& newDrops) {
    if (boss) {
        player->addExperience(3000 * difference); 
            player->addCash(2500.0f * difference);
        } else {
            player->addExperience(500 * difference); 
            player->addCash(250.0f * difference);
        }

        sf::FloatRect bounds = getBounds();
        auto meat = std::make_unique<RawMeat>(bounds.position.x, bounds.position.y, player, difference);
        meat->setSpriteScale(sf::Vector2f{0.5f * difference, 0.5f * difference});
        newDrops.push_back(std::move(meat));
}

bool Bear::checkIfBoss() const { return boss; }