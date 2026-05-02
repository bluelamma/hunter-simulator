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

    // Sprite was moving one pixel to the right for whatever reason, that corrects it
    if(startFrame == 3 && row == 1) {
        left += 1;
    }

    sprite.setTextureRect(sf::IntRect({left, top}, {frameWidth, frameHeight}));
}

// -----------------------
// ----- Projectile ------
// -----------------------
Projectile::Projectile(float startX, float startY, float radius, sf::Vector2f direction, float speed) : GameObject(startX, startY), radius(radius) {
    shape.setFillColor(sf::Color::Black);
    shape.setRadius(radius);
    shape.setPosition(sf::Vector2f({startX, startY}));

    velocity = direction * speed;
}

void Projectile::draw(sf::RenderWindow &window) {
    window.draw(shape);
}

void Projectile::update(float dt, sf::RenderWindow &window) {
    shape.move(velocity * dt);
}

// ----------------------
// -------- Hare --------
// ----------------------
Hare::Hare(float startX, float startY, Player *player) 
    : GameObject(startX, startY), animation(46.0f, 36.0f, 0.15f), facingRow(0), moveTimer(0.0f), moveInterval(0.0f), velocity({0.0f, 0.0f}), speed(50.0f), playerTarget(player) {
    if (!texture.loadFromFile("textures/Hare.png")) {
        std::cerr << "Couldn't load hare texture \n";
    } else {
        sprite.setTexture(texture, true); 
    }

    sprite.setScale(sf::Vector2f({1.5f, 1.5f}));
}

void Hare::draw(sf::RenderWindow &window) {
    window.draw(sprite);
}

void Hare::update(float dt, sf::RenderWindow &window) {
    int startFrame = 0;
    int endFrame = 1;
    isMoving = false;

    sf::Vector2f harePos = sprite.getPosition();
    sf::Vector2f playerPos = playerTarget->getPosition();

    // Distance between hare and the player
    float dx = harePos.x - playerPos.x;
    float dy = harePos.y - playerPos.y;
    float distance = std::sqrt(dx * dx + dy * dy);

    float fleeRadius = playerTarget->isAttacking() ? 800.0f : 300.0f;


    if (distance <= fleeRadius) {  
        // Vector pointing away from the player
        sf::Vector2f dir(dx, dy); 

        float length = std::sqrt(dir.x * dir.x + dir.y * dir.y);
        if (length != 0.0f) {
            dir.x /= length;
            dir.y /= length;
        }

        velocity = dir * (speed * 5.0f);

        if (velocity.x < 0) {
            facingRow = 0; // Left
        } else if (velocity.x > 0) {
            facingRow = 1; // Right
        }

        endFrame = 1;

        moveTimer = 0.0f;
    } else {
        moveTimer += dt;

        if (moveTimer >= moveInterval) {
            moveTimer = 0.0f;

            float dirX = static_cast<float>((rand() % 3) - 1); 
            float dirY = static_cast<float>((rand() % 3) - 1);

            float length = std::sqrt(dirX * dirX + dirY * dirY);
            if (length != 0.0f) {
                dirX /= length;
                dirY /= length;
            }

            velocity = sf::Vector2f(dirX * speed, dirY * speed);

            if(rand() % 5 <= 1) {
                velocity = sf::Vector2f(0.0f, 0.0f);
            }

            if (velocity.x < 0) {
                facingRow = 0; // Left
            } else if (velocity.x > 0) {
                facingRow = 1; // Right
            }

            moveInterval = 1.0f + static_cast<float>(rand() % 3);
            }
        }
    
    if(velocity.x != 0.0f || velocity.y != 0.0f) {
            isMoving = true;
        } 

    if(isMoving) {
        sprite.move(velocity * dt);
    } else {
        endFrame = 0;
    }

    animation.update(facingRow, startFrame, endFrame, dt, sprite);
}

// ----------------------
// ------- Player -------
// ----------------------
Player::Player(float startX, float startY) 
: GameObject(startX, startY), animation(31, 41, 0.25f), facingRow(0) {
    if (!texture.loadFromFile("textures/Player.png")) {
        std::cerr << "Couldn't load player texture \n";
    } else {
        sprite.setTexture(texture, true); 
    }

    sprite.setScale(sf::Vector2f({5.0f, 5.0f}));
}

void Player::draw(sf::RenderWindow &window) {
    window.draw(sprite);

    for(const auto &projectile : projectiles) {
        projectile->draw(window);
    }
}

bool Player::isAttacking() const {
    return movement_cooldown > 0.0f;
}

sf::Vector2f Player::getPosition() const {
    return sprite.getPosition();
}

void Player::update(float dt, sf::RenderWindow &window) {
    float speed = 200.0f * dt;
    int startFrame = 0;
    int endFrame = 0;

    if (movement_cooldown <= 0) {
        isMoving = false;

        // Movement
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::W)) {
            sprite.move(sf::Vector2f(0.0f, -speed)); 
            isMoving = true;
        } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::S)) {
            sprite.move(sf::Vector2f(0.0f, speed));
            isMoving = true;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::A)) {
            sprite.move(sf::Vector2f(-speed, 0.0f));
            isMoving = true;
            facingRow = 1;
        } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::D)) {
            sprite.move(sf::Vector2f(speed, 0.0f));
            isMoving = true;
            facingRow = 0;
        }

        if (isMoving) {
            startFrame = 1;
            endFrame = 2;
        }

        // Attack
        if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
            sprite.move(sf::Vector2f(0.0f, 0.0f));
            startFrame = 3;
            endFrame = 3;
            movement_cooldown = 0.15f;
            isMoving = false;

            sf::Vector2f current_position = sprite.getPosition();

            // Spawn of the bullet relative to the player's sprite
            float spawnX = current_position.x;
            float spawnY = current_position.y + 50.0f;

            if (facingRow == 0) {
                spawnX += 140.0f;
            }

            // Calculate the direcion the bullet is supposed to go
            sf::Vector2i mousePosInt = sf::Mouse::getPosition(window);
            sf::Vector2f mousePos(static_cast<float>(mousePosInt.x), static_cast<float>(mousePosInt.y));
            
            // direction vector
            sf::Vector2f direction = mousePos - sf::Vector2f(spawnX, spawnY);

            // make length 1 
            float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
            if(length != 0.0f) {
                direction.x /= length;
                direction.y /= length;
            }

            if (shot_cooldown <= 0) {
                projectiles.push_back(std::make_unique<Projectile>(spawnX, spawnY, 5.0f, direction, 600.0f));
                shot_cooldown = 0.75; 
            }
        }
    } else {
        movement_cooldown -= dt;
        startFrame = 3;
        endFrame = 3;
    }

    if (shot_cooldown > 0) {
        shot_cooldown -= dt;
    }

    animation.update(facingRow, startFrame, endFrame, dt, sprite);

    // Makes the projectiles actually appear
    for (const auto &projectile : projectiles) {
        projectile->update(dt, window);
    }
}

// ----------------------
// ----- Game Logic -----
// ----------------------
Game::Game() : window(sf::VideoMode({1920, 1080}),  "Hunter Simulator") {}

void Game::init() {
    srand(static_cast<unsigned int>(time(nullptr)));
    GameObjects.clear();

    auto player = std::make_unique<Player>(0.0f, 500.0f);
    Player *playerPtr = player.get();

    GameObjects.emplace_back(std::move(player));
    GameObjects.emplace_back(std::make_unique<Hare>(800.0f, 600.0f, playerPtr));
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
        object->update(dt, window);
    }
}

void Game::render() {
    window.clear(sf::Color({80, 244, 51}));

    for(const auto &object : GameObjects) {
        object->draw(window);
    }

    window.display();
}