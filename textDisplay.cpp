#include "textDisplay.hpp"

TextDisplay::TextDisplay(Player *player) : player(player), pauseText(font), restartText(font), cashText(font) {
    if (!font.openFromFile("fonts/pixelFont.ttf")) { 
        std::cerr << "Failed to load the font\n";
    }

    // -- Pause -- 
    pauseText.setFont(font);
    pauseText.setString("PAUSED\n\nPress ESC to Resume\nPress Q to Quit");
    pauseText.setCharacterSize(48);
    pauseText.setFillColor(sf::Color::White);

    // Centers the text alignment
    sf::FloatRect pauseTextBounds = pauseText.getLocalBounds();
    pauseText.setOrigin({pauseTextBounds.size.x / 2.0f, pauseTextBounds.size.y / 2.0f});

    // Dark overlay
    pauseOverlay.setFillColor(sf::Color(0, 0, 0, 150));


    // -- Restart --
    restartText.setFont(font);
    restartText.setString("\n\nPress R to Restart\nPress Q to Quit");
    restartText.setCharacterSize(28);
    restartText.setFillColor(sf::Color(255, 255, 255));

    // Centers the text alignment
    sf::FloatRect restartTextBounds = restartText.getLocalBounds();
    restartText.setOrigin({restartTextBounds.size.x / 2.0f, restartTextBounds.size.y / 2.0f - 50.0f});
    
    // Transparent
    restartOverlay.setFillColor(sf::Color(255, 0, 0, 80));


    // -- Cash display --
    cashText.setFont(font);
    cashText.setCharacterSize(36); // Adjust size as needed
    cashText.setFillColor(sf::Color::Yellow); // Using yellow to represent cash
    cashText.setOutlineColor(sf::Color::Black);
    cashText.setOutlineThickness(2.0f);
    // ------------------------

}

void TextDisplay::drawHud(sf::RenderWindow& window) {
        // Left side (hp, xp)
        // Prevents negative percentages if HP drops below 0
        float hpPercent = player->getHp() > 0 ? static_cast<float>(player->getHp()) / player->getMaxHp() : 0.0f;
        float xpPercent = static_cast<float>(player->getExperience()) / player->getExperienceThreshold();

        // XP bar
        sf::RectangleShape xpBg(sf::Vector2f({200.0f, 8.0f}));
        xpBg.setPosition(sf::Vector2f({20.0f, 22.0f}));
        xpBg.setFillColor(sf::Color(0, 0, 50, 150)); // Dark blue background

        sf::RectangleShape xpBar(sf::Vector2f({200.0f * xpPercent, 8.0f}));
        xpBar.setPosition(sf::Vector2f({20.0f, 22.0f}));
        xpBar.setFillColor(sf::Color(0, 150, 255, 255)); // Bright blue fill

        // HP bar
        sf::RectangleShape hpBg(sf::Vector2f({200.0f, 20.0f}));
        hpBg.setPosition(sf::Vector2f({20.0f, 30.0f})); 
        hpBg.setFillColor(sf::Color(50, 0, 0, 150)); // Dark red background

        sf::RectangleShape hpBar(sf::Vector2f({200.0f * hpPercent, 20.0f}));
        hpBar.setPosition(sf::Vector2f({20.0f, 30.0f}));
        hpBar.setFillColor(sf::Color(255, 0, 0, 255)); // Bright red fill

        // Right side (cash)
        std::stringstream stream;
        stream << "Cash: $" << std::fixed << std::setprecision(2) << player->getCash();
        cashText.setString(stream.str());

        // 'window' now correctly refers to the parameter passed in
        sf::FloatRect textBounds = cashText.getLocalBounds();
        float screenWidth = window.getSize().x;
        cashText.setPosition(sf::Vector2f(screenWidth - textBounds.size.x - 20.0f, 20.0f));

        // Draws the hud
        window.draw(xpBg);
        window.draw(xpBar);
        window.draw(hpBg);
        window.draw(hpBar); 
        window.draw(cashText);
}

void TextDisplay::drawPause(sf::RenderWindow &window, sf::Vector2f cameraCenter, sf::Vector2f cameraSize) {
    pauseText.setFont(font);
    pauseText.setString("PAUSED\n\nPress ESC to Resume\nPress Q to Quit");
    pauseText.setCharacterSize(48);
    pauseText.setFillColor(sf::Color::White);

    // Centers the text alignment
    sf::FloatRect pauseTextBounds = pauseText.getLocalBounds();
    pauseText.setOrigin({pauseTextBounds.size.x / 2.0f, pauseTextBounds.size.y / 2.0f});

    // Dark overlay
    pauseOverlay.setFillColor(sf::Color(0, 0, 0, 150));

    pauseOverlay.setSize(cameraSize);
    pauseOverlay.setPosition(cameraCenter - (cameraSize / 2.0f));
    pauseText.setPosition(cameraCenter);

    window.draw(pauseOverlay);
    window.draw(pauseText);
}

void TextDisplay::drawRestart(sf::RenderWindow &window, sf::Vector2f cameraCenter, sf::Vector2f cameraSize) {
    restartText.setString("\nScore: " + std::to_string(player->getScore()) + 
                          "\nLevel: " + std::to_string(player->getLevel()) + 
                          "\n\nPress R to Restart\nPress Q to Quit");

    restartOverlay.setSize(cameraSize);
    restartOverlay.setPosition(cameraCenter - (cameraSize / 2.0f));
    restartText.setPosition(cameraCenter);
        
    window.draw(restartOverlay);
    window.draw(restartText);
}