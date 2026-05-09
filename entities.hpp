#pragma once
#include "game.hpp"

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

class Hare : public GameObject {
private:
    Animation animation;
    Player *playerTarget;  // To check if is being attacked
    sf::Vector2f velocity; // Current moving direction
    float moveTimer;       // Time since last direction change
    float moveInterval;    // How often the direction changes (in seconds)
    float speed; 
    bool isMoving; 
    int facingRow; // 0 = Hare facing left // 1 = Hare facing right

    float difference; // Difference of stats from the assumed standard value used to calculate xp and cash gain
    int maxHp;
    int hp;
    float panicTimer; // to keep the thingy runnin
    sf::RectangleShape hitbox;
public:
    Hare(float startX, float startY, Player *player);
    void draw(sf::RenderWindow &window) override;
    void update(float dt, sf::RenderWindow &window) override;

    void takeDamage(int amount);
    int getHp() const;
    float getDiff() const;
    sf::FloatRect getBounds() const;
};