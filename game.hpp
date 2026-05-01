#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <memory>

class Animation {
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
    virtual void update(float dt) {};
    virtual void setScale(sf::Vector2f value) {};
};

class Player : public GameObject { 
protected: 
    Animation animation;
    int facingRow; // row 0 = Player turned left // row 1 = Player turned right
public:
    Player(float startX, float startY);
    void draw(sf::RenderWindow &window) override;
    void update(float dt) override;
    void setScale(sf::Vector2f value) override;
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