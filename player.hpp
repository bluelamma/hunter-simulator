#pragma once

#include "game.hpp"
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
        
    // Leveling stuff
    int level;
    int experience;
    int experienceThreshold;

    // Movement
    int facingRow; // 0 = Player facing left // 1 = Player facing right
    float movement_cooldown;
    float shot_cooldown;
    bool isMoving;
    int speed;

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
public:
    Player(float startX, float startY);
    void setPosition(sf::Vector2f Pos);
    void draw(sf::RenderWindow &window) override;
    void update(float dt, sf::RenderWindow &window) override;

    void addExperience(int amount);
    void levelUp();
    void addCash(float amount); 
    bool spendCash(float amount);

    // Upgrading stuff
    void upgradeDamage(int amount);
    void upgradeReloadSpeed(float amount);
    void upgradeBulletVelocity(float amount);

    // Getters
    bool isAttacking() const;
    float getCash() const;
    int getDamage() const;
    int getHp() const;
    int getMaxHp() const;
    int getExperienceThreshold() const;
    int getExperience() const;
    bool checkIfDead() const;
    sf::Vector2f getPosition() const; // Necessary for entities and camera to get position of the player
    std::vector<std::unique_ptr<Projectile>> &getProjectiles();
};