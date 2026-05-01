#include "game.hpp"

// -----------------------
// ----- Game object -----
// -----------------------
GameObject::GameObject(float startX, float startY) 
    : x(startX), y(startY), width(0.0f), height(0.0f), active(true), sprite(texture) {
        sprite.setPosition({x, y});
    }

// -----------------------
// ------ Animation ------
// -----------------------
Animation::Animation(int frameWidth, int frameHeight, float holdTime) 
    : frameWidth(frameWidth), frameHeight(frameHeight), holdTime(holdTime), timer(0.0f), currentFrame(0) {}

void Animation::update(int row, int startFrame, int endFrame, float dt, sf::Sprite &sprite) {
    if (currentFrame < startFrame || currentFrame > endFrame) {
        currentFrame = startFrame;
        timer = 0.0f;
    }
    
    timer += dt;

    if (timer >= holdTime) {
        timer -= holdTime;
        currentFrame++;

        if (currentFrame > endFrame) {
            currentFrame = startFrame;
        }
    }

    int left = frameWidth * currentFrame;
    int top = frameHeight * row;

    sprite.setTextureRect(sf::IntRect({left, top}, {frameWidth, frameHeight}));
}


// ----------------------
// ------- Player -------
// ----------------------
Player::Player(float startX, float startY) : GameObject(startX, startY), animation(31, 41, 0.25f), facingRow(0) {
    if (!texture.loadFromFile("textures/player.png")) {
        std::cerr << "Couldn't load player texture \n";
    } else {
        sprite.setTexture(texture, true); 
    }
}

void Player::draw(sf::RenderWindow &window) {
    window.draw(sprite);
    this->setScale(sf::Vector2f{3.0f, 3.0f});
}

void Player::update(float dt) {
    float speed = 200.0f * dt;
    int row = 0;
    int startFrame = 0;
    int endFrame = 0;

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::W)) {
        sprite.move(sf::Vector2f(0.f, -speed)); 
        startFrame = 1;
        endFrame = 2;
    } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::S)) {
        sprite.move(sf::Vector2f(0.f, speed));
        startFrame = 1;
        endFrame = 2;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::A)) {
        sprite.move(sf::Vector2f(-speed, 0.f));
        facingRow = 1;
        startFrame = 1;
        endFrame = 2;
    } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::D)) {
        sprite.move(sf::Vector2f(speed, 0.f));
        facingRow = 0;
        startFrame = 1;
        endFrame = 2;
    }

    animation.update(facingRow, startFrame, endFrame, dt, sprite);
}

void Player::setScale(sf::Vector2f value) {
    sprite.setScale(value);
}

// ----------------------
// ----- Game Logic -----
// ----------------------
Game::Game() : window(sf::VideoMode({800, 800}),  "Hunter Simulator") {}

void Game::init() {
    GameObjects.clear();

    GameObjects.emplace_back(std::make_unique<Player>(350.0f, 350.0f));
}

void Game::run() {
    this->init();

    sf::Clock clock;

    while (window.isOpen()) {

        sf::Time dt = clock.restart();

        while (auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>())
                window.close();
        }
        update(dt.asSeconds());
        render();
    }
}

void Game::update(float dt) {
    for (const auto &object : GameObjects) {
        object->update(dt);
    }
}

void Game::render() {
    window.clear(sf::Color::Black);

    for(const auto &object : GameObjects) {
        object->draw(window);
    }

    window.display();
}