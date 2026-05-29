#include "../../include/entities.hpp"
#include "../../include/world.hpp"
#include "../../include/player.hpp"
#include "../../include/pickup.hpp"

// ----------------------
// -------- Boar --------
// ----------------------
Boar::Boar(float startX, float startY, Player *player, int variant) 
    : Creature(startX, startY, player, 45.0f, 28.0f, 0.25f), moveTimer(0.0f), moveInterval(0.75f), 
    bounceTimer(0.0f), attackTimer(1.0f), turningLocked(false), idleTimer(0.0f), pursuitTimer(0.0f), variant(variant) {
    if(variant == 0) {
        if (!texture.loadFromFile("../assets/textures/boar.png")) {
            std::cerr << "Couldn't load boar texture \n";
        } else {
            sprite.setTexture(texture, true); 
        }
    } else if (variant == 1) {
        if (!texture.loadFromFile("../assets/textures/boar_black.png")) {
            std::cerr << "Couldn't load boar_black texture \n";
        } else {
            sprite.setTexture(texture, true); 
        }
    }

    deathTimer = 0.0f;
    sf::Vector2f boarPos = sprite.getPosition();
    
    if (rand() % 20 < 1 + variant) {
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
    if (attackDamage > 300) {
        attackDamage = 300;
    }

    if(variant == 1) {
        maxHp *= 1.5;
        hp *= 1.5;
        attackDamage *= 1.25;
    }

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
                    if (!GameObject::world->isSolid(feetX, feetY, false, false)) {
                        sprite.setPosition(nextPos);
                    } else {
                        bool slid = false;

                        // Try moving horizontally
                        if (!GameObject::world->isSolid(nextPos.x + offsetX, currentPos.y + offsetY, false, false)) {
                            sprite.setPosition({nextPos.x, currentPos.y});
                            slid = true;
                        } 
                        // Try moving vertically 
                        else if (!GameObject::world->isSolid(currentPos.x + offsetX, nextPos.y + offsetY, false, false)) {
                            sprite.setPosition({currentPos.x, nextPos.y});
                            slid = true;
                        }

                        // No bounce if aggroed, chase logic overwrites velocity anyway
                        if (!aggroed) {
                            bounceTimer = 1.5f; 

                            bool hitHorizontal = GameObject::world->isSolid(feetX, currentPos.y + offsetY, false, false);
                            bool hitVertical = GameObject::world->isSolid(currentPos.x + offsetX, feetY, false, false);

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
    if (variant == 0) {
        player->addExperience(100 * difference); 
        player->addCash(25.0f * difference);
    } else if (variant == 1) {
        player->addExperience(150 * difference); 
        player->addCash(40.0f * difference);
    }

    if(rand() % 5 <= 1) {
        sf::FloatRect bounds = getBounds();
        auto meat = std::make_unique<RawMeat>(bounds.position.x, bounds.position.y, player, difference);
        meat->setSpriteScale(sf::Vector2f{0.5f * difference, 0.5f * difference});
        newDrops.push_back(std::move(meat));
    }
}