#pragma once

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <memory>
#include <cmath>
#include <cstdlib> // For randomness
#include <algorithm> // Required for std::clamp

// Was necessary to split the files like I did and still make it work
class Player;
class TileMap;

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

    // static makes it shared across all gameObjects 
    static TileMap* world;
};

class Game {
private:
    int tileSize = 64; // Used to make spawn point of entities on specified tiles

    sf::RenderWindow window;
    std::vector<std::unique_ptr<GameObject>> GameObjects;

    sf::View camera;
    Player* trackedPlayer;

    void init(); // loads the player and map, sets camera size
public:
    Game();
    void run();
    void render();
    void update(float dt);
};