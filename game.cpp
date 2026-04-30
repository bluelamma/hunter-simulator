#include "game.hpp"

Game::Game() : window(sf::VideoMode({800, 800}),  "Hunter Simulator") {}

void Game::run() {
    while (window.isOpen()) {
        while (auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>())
                window.close();
        }
        render();
    }
}

void Game::render() {
    window.clear(sf::Color::Black);
    window.display();
}