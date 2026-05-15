#pragma once

#include "game.hpp"
#include "player.hpp"

class Creature : public GameObject {
protected: 
    Animation animation;
    Player *player;  
    sf::Vector2f velocity; 
    float speed; 
    bool isMoving; 
    int facingRow; 
    int startFrame;
    int endFrame;

    float difference; 
    int maxHp;
    int hp;
    sf::RectangleShape hitbox;

    bool isAttacking;
    bool isDead;
    float deathTimer;

public:
    Creature(float startX, float startY, Player *player, float animWidth, float animHeight, float animHoldTime);
    virtual ~Creature() = default;

    virtual void draw(sf::RenderWindow &window) override;
    virtual void takeDamage(int amount);
    virtual void update(float dt, sf::RenderWindow &window) override = 0; 

    int getHp() const;
    float getDiff() const;
    float getDeathTimer() const;
    bool checkIfDead() const;
    sf::FloatRect getBounds() const;
};

class Hare : public Creature {
private:
    float moveTimer;       
    float moveInterval;    
    float panicTimer;      
    float bounceTimer;
public:
    Hare(float startX, float startY, Player *player);
    
    void update(float dt, sf::RenderWindow &window) override;
};

class Boar : public Creature {
private:
    float moveTimer;       
    float moveInterval;       
    float bounceTimer;

    float idleTimer;

    float attackTimer;
    float attackCooldown;
    int attackDamage;
    bool turningLocked;

    float pursuitTimer;
    float aggroRadius;
    float aggroCheckTimer;
    bool aggroed;
public:
    Boar(float startX, float startY, Player *player);

    void update(float dt, sf::RenderWindow &window) override;
};