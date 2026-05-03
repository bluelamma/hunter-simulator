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
};

class Player : public GameObject { 
private: 
    Animation animation;
protected: 
    std::vector<std::unique_ptr<Projectile>> projectiles;
    int facingRow; // 0 = Player facing left // 1 = Player facing right
    float movement_cooldown;
    float shot_cooldown;
    bool isMoving;
public:
    Player(float startX, float startY);
    void draw(sf::RenderWindow &window) override;
    void update(float dt, sf::RenderWindow &window) override;
    bool isAttacking() const;
    sf::Vector2f getPosition() const; // Necessary for entities and camera to get position of the player
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
public:
    Hare(float startX, float startY, Player *player);
    void draw(sf::RenderWindow &window) override;
    void update(float dt, sf::RenderWindow &window) override;
};