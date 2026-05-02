#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <memory>
#include <cmath>
#include <cstdlib>

class Animation {
private:
    int frameWidth;
    int frameHeight;
    float holdTime;
    float timer;
    int currentFrame;
public: 
    Animation(int frameWidth, int frameHeight, float holdTime);

    void update(int row, int startFrame, int endFrame, float dt, sf::Sprite &sprite);
};



class GameObject {
protected:
    float x;
    float y;
    float width;
    float height;
    bool active;
    sf::Texture texture;
    sf::Sprite sprite;
public:
    GameObject(float startX, float startY);

    virtual ~GameObject() = default;
    virtual void draw(sf::RenderWindow &window) = 0;
    virtual void update(float dt, sf::RenderWindow &window) {};
};

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
    sf::Vector2f getPosition() const; // Necessary for entities to get position of the player
};

class Hare : public GameObject {
private:
    Animation animation;
    float moveTimer;       // Time since last direction change
    float moveInterval;    // How often the direction changes (in seconds)
    sf::Vector2f velocity; // Current moving direction
    float speed; 
    bool isMoving;
    Player *playerTarget;
protected:
    int facingRow; // 0 = Hare facing left // 1 = Hare facing right
public:
    Hare(float startX, float startY, Player *player);
    void draw(sf::RenderWindow &window) override;
    void update(float dt, sf::RenderWindow &window) override;
};



class Game {
private:
    sf::RenderWindow window;
    std::vector<std::unique_ptr<GameObject>> GameObjects;

    void init();
public:
    Game();
    void run();
    void render();
    void update(float dt);
};