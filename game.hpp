#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <memory>
#include <cmath>

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
protected: 
    Animation animation;
    std::vector<std::unique_ptr<Projectile>> projectiles;
    int facingRow; // row 0 = Player facing left // row 1 = Player facing right
    float movement_cooldown;
    float shot_cooldown;
    bool isMoving;
public:
    Player(float startX, float startY);
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