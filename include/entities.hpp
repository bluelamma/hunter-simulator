#pragma once

#include "game.hpp"
#include "animation.hpp"
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
    virtual void grantRewards(std::vector<std::unique_ptr<GameObject>>& newDrops) = 0;

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
    float chargeTimer;
    float regenTimer;
    
    bool directionCalculated;
public:
    Hare(float startX, float startY, Player *player);
    
    void update(float dt, sf::RenderWindow &window) override;
    void grantRewards(std::vector<std::unique_ptr<GameObject>>& newDrops) override;
};

class Boar : public Creature {
private:
    int variant;

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
    Boar(float startX, float startY, Player *player, int variant);

    void update(float dt, sf::RenderWindow &window) override;
    void grantRewards(std::vector<std::unique_ptr<GameObject>>& newDrops) override;
};

class Bear : public Creature {
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
    bool aggroed;
    bool boss;

    sf::SoundBuffer bearBuffer;
    sf::Sound bearSound;
public:
    Bear(float startX, float startY, Player *player, bool boss);

    void update(float dt, sf::RenderWindow &window) override;
    void grantRewards(std::vector<std::unique_ptr<GameObject>>& newDrops) override;

    bool checkIfBoss() const;
};