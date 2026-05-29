#include "../../include/entities.hpp"
#include "../../include/world.hpp"
#include "../../include/player.hpp"
#include "../../include/pickup.hpp"

// ----------------------
// -------- Hare --------
// ----------------------
Hare::Hare(float startX, float startY, Player *player) 
    : Creature(startX, startY, player, 46.0f, 36.0f, 0.15f), 
      moveTimer(0.0f), moveInterval(0.0f), panicTimer(0.0f), bounceTimer(0.0f), chargeTimer(0.0), regenTimer(5.0f), directionCalculated(false) {
    
    if (!texture.loadFromFile("../assets/textures/Hare.png")) {
        std::cerr << "Couldn't load hare texture \n";
    } else {
        sprite.setTexture(texture, true); 
    }

    deathTimer = 0.0f;
    sf::Vector2f harePos = sprite.getPosition();
    
    if (rand() % 10 < 1) {
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
        if(hp != maxHp && hp > maxHp * 0.3f) {
            regenTimer -= dt;
            if (regenTimer < 0.0f) {
                hp = maxHp;
                regenTimer = 5.0f;
            }
        }

        // Distance between hare and the player
        float dx = harePos.x - playerPos.x;
        float dy = harePos.y - playerPos.y;
        float distance = std::sqrt(dx * dx + dy * dy);

        float fleeRadius = player->isAttacking() ? 600.0f : 250.0f;
        bool isFleeing = (distance <= fleeRadius || panicTimer > 0.0f);

        // Updates timers
        if (panicTimer > 0.0f) panicTimer -= dt;
        if (bounceTimer > 0.0f) bounceTimer -= dt; 
        if (chargeTimer > 0.0f) chargeTimer -= dt;

        if(chargeTimer > 0.0f) {
            // Direction towards the player
            if(!directionCalculated) {
                directionCalculated = true;
                sf::Vector2f dir(-dx, -dy); 
                float length = std::sqrt(dir.x * dir.x + dir.y * dir.y);
            
                if (length != 0.0f) {
                    dir.x /= length;
                    dir.y /= length;
                }

                // Slightly faster than normal fleeing
                velocity = dir * (speed * 8.0f); 
            }

            if (velocity.x < 0) facingRow = 0; 
            else if (velocity.x > 0) facingRow = 1; 

            endFrame = 1;
            moveTimer = 0.0f;
        } else if (isFleeing || (hp != maxHp && bounceTimer <= 0.0f)) {  
            if (isFleeing) bounceTimer = 0.0f;
            
            sf::Vector2f dir(dx, dy); 
            float length = std::sqrt(dir.x * dir.x + dir.y * dir.y);
            if (length != 0.0f) {
                dir.x /= length;
                dir.y /= length;
            }

            if (hp <= maxHp * 0.3f) {
                velocity = dir * (speed * 0.5f);
            } else {
                velocity = dir * (speed * 6.0f);
            }

            if (velocity.x < 0) facingRow = 0; 
            else if (velocity.x > 0) facingRow = 1; 

            endFrame = 1;
            moveTimer = 0.0f;
        } else if (bounceTimer > 0.0f) {
            // DVD state
            endFrame = 1;
        } else {
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

        // Apply movement
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
                if (!GameObject::world->isSolid(feetX, feetY, false, false)) {
                    sprite.setPosition(nextPos);
                } else {
                    if (!isFleeing) {
                        bounceTimer = 1.5f; // Enter bounce state to prevent getting stuck on water

                        bool hitHorizontal = GameObject::world->isSolid(feetX, currentPos.y + offsetY, false, false);
                        bool hitVertical = GameObject::world->isSolid(currentPos.x + offsetX, feetY, false, false);

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
                        if (!GameObject::world->isSolid(currentPos.x + offsetX, feetY, false, false)) {
                            sprite.setPosition({currentPos.x, nextPos.y});
                        } else if (!GameObject::world->isSolid(feetX, currentPos.y + offsetY, false, false)) {
                            sprite.setPosition({nextPos.x, currentPos.y});
                        } else if(isFleeing && hp > maxHp * 0.3f) {
                            chargeTimer = 2.0f;
                            directionCalculated = false;
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