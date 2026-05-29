#include "../../include/entities.hpp"
#include "../../include/world.hpp"
#include "../../include/player.hpp"
#include "../../include/pickup.hpp"

// ----------------------
// -------- Bear --------
// ----------------------
Bear::Bear(float startX, float startY, Player *player, bool boss) 
    : Creature(startX, startY, player, 63.0f, 40.0f, 0.35f), // size of the sprite
      moveTimer(0.0f), moveInterval(1.0f), bounceTimer(0.0f), attackTimer(1.5f), 
      turningLocked(false), idleTimer(0.0f), pursuitTimer(0.0f), aggroed(false), boss(boss), bearSound(bearBuffer) {
    
    if (!boss) {
        if (!texture.loadFromFile("../assets/textures/bear.png")) {
            std::cerr << "Couldn't load bear texture \n";
        } else {
            sprite.setTexture(texture, true); 
        }

            hp = rand() % 1000 + 400;
            maxHp = hp;
            difference = maxHp / 300.0f;
            attackDamage = 70 * difference;
    } else {
            if (!texture.loadFromFile("../assets/textures/bear_black.png")) {
                std::cerr << "Couldn't load bear texture \n";
            } else {
            sprite.setTexture(texture, true); 
            }

            hp = 2000;
            maxHp = hp;
            difference = maxHp / 450.0f;
            attackDamage = 80 * difference;
    }

    if (bearBuffer.loadFromFile("../assets/sounds/bearSound.mp3")) {
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
        
        // plays attack animation for 0.5s after an attack is triggered
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

            // --- Roaming && Chasing Logic ---
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

                if (!GameObject::world->isSolid(feetX, feetY, false, false)) {
                    sprite.setPosition(nextPos);
                } else {
                    // Bounce Logic - Inverts velocity based on which axis hit the wall
                    bool hitHorizontal = GameObject::world->isSolid(feetX, currentPos.y + offsetY, false, false);
                    bool hitVertical = GameObject::world->isSolid(currentPos.x + offsetX, feetY, false, false);

                    if (hitHorizontal) velocity.x *= -1.0f; 
                    if (hitVertical) velocity.y *= -1.0f;   
                            
                    // Corner case flip
                    if (!hitHorizontal && !hitVertical) {
                        velocity.x *= -1.0f;
                        velocity.y *= -1.0f;
                    }

                    // Sliding application so it doesn't get stuck visually
                    if (!GameObject::world->isSolid(nextPos.x + offsetX, currentPos.y + offsetY, false, false)) {
                        sprite.setPosition({nextPos.x, currentPos.y});
                    } else if (!GameObject::world->isSolid(currentPos.x + offsetX, nextPos.y + offsetY, false, false)) {
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
                player->takeDamage(attackDamage);
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
            player->addExperience(300 * difference); 
            player->addCash(100.0f * difference);
        }

        sf::FloatRect bounds = getBounds();
        auto meat = std::make_unique<RawMeat>(bounds.position.x, bounds.position.y, player, difference);
        meat->setSpriteScale(sf::Vector2f{0.5f * difference, 0.5f * difference});
        newDrops.push_back(std::move(meat));
}

bool Bear::checkIfBoss() const { return boss; }