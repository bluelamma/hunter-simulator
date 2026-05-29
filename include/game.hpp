#pragma once

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <vector>
#include <memory>
#include <cmath>
#include <cstdlib>
#include <algorithm>
#include <sstream>
#include <iomanip>

// Used by loadLocation and mapLoader
enum class LocationID {
    Overworld,
    Cave,
    Home,
    Shop
};


class Player;
class TileMap;
class TextDisplay;

class GameObject {
protected:
    float x;
    float y;
    float width;
    float height;
    bool solid;
    sf::Texture texture;
    sf::Sprite sprite;
public:
    bool active;
    GameObject(float startX, float startY);

    void setSpriteScale(sf::Vector2f scale); // for resizing objects 

    sf::FloatRect getGlobalBounds() const;

    virtual ~GameObject() = default;
    virtual void draw(sf::RenderWindow &window) = 0;
    virtual void update(float dt, sf::RenderWindow &window) {};

    // static makes it shared across all gameObjects 
    static TileMap* world;
};

class Game {
private:
    int tileSize = 64; // Used to make spawn point of things on specified tiles, load maps correctly

    bool isPaused;

    sf::RenderWindow window;
    std::vector<std::unique_ptr<GameObject>> GameObjects;

    sf::View camera;
    std::unique_ptr<Player> player;
    std::unique_ptr<TextDisplay> textDisplay;
    LocationID currentLocation; // Tracks which map is active

    void init();
    void loadLocation(LocationID dest, sf::Vector2f spawnPoint);
public:
    Game();
    ~Game();
    void run();
    void render();
    void update(float dt); // camera here
};