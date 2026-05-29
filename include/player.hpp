#pragma once

#include "game.hpp"
#include "animation.hpp"
#include "entities.hpp"

class Projectile : public GameObject {
private: 
    sf::CircleShape shape;
    sf::Vector2f velocity;
    float radius;
public: 
    Projectile(float startX, float startY, float radius, sf::Vector2f direction, float speed);
    void draw(sf::RenderWindow &window);
    void update(float dt, sf::RenderWindow &window);
    sf::FloatRect getBounds() const;
};

class Player : public GameObject { 
private: 
    Animation animation;

    //Score 
    int score;
        
    // Leveling stuff
    int level;
    int experience;
    int experienceThreshold;

    // Movement
    int facingRow; // 0 = Player facing left // 1 = Player facing right
    float movement_cooldown;
    float movementBlocked_cooldown;
    float shot_cooldown;
    bool isMoving;
    bool movementBlocked;
    int speed;
    int overrideStartFrame;
    int overrideEndFrame;

    // Sound stuff
    sf::SoundBuffer shootBuffer;
    sf::Sound shootSound;
    sf::SoundBuffer deathBuffer;
    sf::Sound deathSound;

    // Combat stuff
    int hp;
    int maxHp;
    int damage;
    bool isDead;
    float base_shot_cooldown;
    float bullet_velocity;
    std::vector<std::unique_ptr<Projectile>> projectiles;

    // Economy stuff
    float cash;
    float nextDamageCost;
    float nextReloadCost;
    float nextVelocityCost;
    float cigarettesCost;
public:
    Player(float startX, float startY);
    void setPosition(sf::Vector2f Pos);
    void draw(sf::RenderWindow &window) override;
    void update(float dt, sf::RenderWindow &window) override;

    void addExperience(int amount);
    void levelUp();
    void addCash(float amount); 
    bool spendCash(float amount);

    void takeDamage(int amount);
    void healHp(int amount);

    // Allows objects to change animation
    void setAnimation(float dt, int startFrame, int endFrame, float totalDuration, float frameDuration);

    // Upgrading stuff
    void upgradeDamage(int amount);
    void upgradeReloadSpeed(float amount);
    void upgradeBulletVelocity(float amount);
    void raiseSpeed(int amount);

    // Getters
    bool isAttacking() const;
    float getCash() const;
    float getNextDamageCost() const;
    float getNextReloadCost() const;
    float getNextVelocityCost() const;
    float getNextCigarettesCost() const;
    int getDamage() const;
    int getHp() const;
    int getMaxHp() const;
    int getExperienceThreshold() const;
    int getSpeed() const;
    int getExperience() const;
    int getScore() const;
    int getLevel() const;
    bool checkIfDead() const;
    sf::FloatRect getBounds() const;
    sf::Vector2f getPosition() const; // Necessary for entities and camera to get position of the player
    std::vector<std::unique_ptr<Projectile>> &getProjectiles();
};