#include "../../include/entities.hpp"
#include "../../include/world.hpp"
#include "../../include/player.hpp"
#include "../../include/pickup.hpp"

// -----------------------
// ------ Creature -------
// -----------------------
Creature::Creature(float startX, float startY, Player *player, float animWidth, float animHeight, float animHoldTime)
    : GameObject(startX, startY), player(player), animation(animWidth, animHeight, animHoldTime), 
      velocity({0.0f, 0.0f}), speed(0.0f), isMoving(false), facingRow(0), hp(0), maxHp(0), difference(1.0f), isDead(false), isAttacking(false) {}

void Creature::draw(sf::RenderWindow &window) {
    window.draw(sprite);
    window.draw(hitbox);
}

void Creature::takeDamage(int amount) {
    hp -= amount;
    if (hp <= 0) {
        isDead = true; 
    }
}

int Creature::getHp() const { return hp; }
float Creature::getDiff() const { return difference; }
float Creature::getDeathTimer() const { return deathTimer; }
bool Creature::checkIfDead() const { return isDead; }
sf::FloatRect Creature::getBounds() const { return hitbox.getGlobalBounds(); }