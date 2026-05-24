#include "pickup.hpp"


// -----------------------
// ------- Pickup --------
// -----------------------
Pickup::Pickup(float startX, float startY, Player *player) 
    : GameObject(startX, startY), player(player) {}


// -----------------------
// ------ Raw Meat -------
// -----------------------
RawMeat::RawMeat(float startX, float startY, Player *player, float multiplier) 
    : Pickup(startX, startY, player), multiplier(multiplier) {

    if (!texture.loadFromFile("textures/rawMeat.png")) {
        std::cerr << "Couldn't load RawMeat texture \n";
    } else {
        sprite.setTexture(texture, true); 
    }

    healingValue = 60 * multiplier;
    sellingValue = 5.0f * multiplier;
}

void RawMeat::draw(sf::RenderWindow &window) {
    window.draw(sprite);
}

void RawMeat::update(float dt, sf::RenderWindow &window) {
    if (!this->active || player == nullptr) {
        return; 
    }

    sf::FloatRect meatBounds = this->getGlobalBounds();
    sf::FloatRect playerBounds = player->getBounds(); 

    if (meatBounds.findIntersection(playerBounds) && sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::E)) { 
        if(player->getMaxHp() == player->getHp()) {
            player->addCash(sellingValue);
        } else {
            player->setAnimation(dt, 9, 10, 0.75f, 0.25f);
            player->healHp(healingValue);
        }

        this->active = false; 
    }
}