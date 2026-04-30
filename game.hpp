#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

class Game {
private:
    sf::RenderWindow window;
public:
    Game();
    void run();
    void render();
};