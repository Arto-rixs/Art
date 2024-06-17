#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include <iostream>

class FuturisticButton {
public:
    FuturisticButton(const sf::Vector2f& position, const sf::Vector2f& size, const sf::String& text, const sf::Texture& texture)
        : isHovered(false), isPressed(false) {
        // Задаём параметры для кнопки
        button.setSize(size);
        button.setPosition(position);
        button.setTexture(&texture);  // Устанавливаем текстуру для кнопки
        button.setOutlineThickness(2);
        button.setOutlineColor(sf::Color(0, 255, 255, 200));

        // Настраиваем шрифт и текст кнопки
        if (!font.loadFromFile("/usr/share/fonts/truetype/roboto/unhinted/RobotoTTF/Roboto-Regular.ttf")) {
            std::cerr << "Ошибка при загрузке шрифта" << std::endl;
        }

        // Настраиваем основной текст
        buttonText.setFont(font);
        buttonText.setString(text);
        buttonText.setCharacterSize(30);
        buttonText.setFillColor(sf::Color(100, 149, 237));
        buttonText.setPosition(
            position.x + (size.x - buttonText.getLocalBounds().width) / 2,
            position.y + (size.y - buttonText.getLocalBounds().height) / 2 - 5
        );

        // Настраиваем текст тени
        buttonTextShadow.setFont(font);
        buttonTextShadow.setString(text);
        buttonTextShadow.setCharacterSize(30);
        buttonTextShadow.setFillColor(sf::Color(0, 0, 0, 100));
        buttonTextShadow.setPosition(
            buttonText.getPosition().x + 2, // смещение по X
            buttonText.getPosition().y + 2  // смещение по Y
        );

        // Настраиваем светящийся контур
        buttonGlow.setSize(size);
        buttonGlow.setPosition(position);
        buttonGlow.setFillColor(sf::Color::Transparent);
        buttonGlow.setOutlineThickness(5);
        buttonGlow.setOutlineColor(sf::Color(0, 255, 255, 128));
    }

    void handleEvent(const sf::Event& event, const sf::RenderWindow& window) {
        sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));

        if (button.getGlobalBounds().contains(mousePos)) {
            if (!isHovered) {
                isHovered = true;
                button.setOutlineColor(sf::Color(0, 255, 255, 255));
                buttonGlow.setOutlineColor(sf::Color(0, 255, 255, 200));
                buttonText.setFillColor(sf::Color::White);
            }

            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                isPressed = true;
            } else if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left) {
                if (isPressed) {
                    std::cout << "Кнопка нажата!" << std::endl;
                    isPressed = false;
                }
            }
        } else {
            if (isHovered) {
                isHovered = false;
                button.setOutlineColor(sf::Color(0, 255, 255, 200));
                buttonGlow.setOutlineColor(sf::Color(0, 255, 255, 128));
                buttonText.setFillColor(sf::Color(100, 149, 237));
            }
        }
    }

    void draw(sf::RenderWindow& window) {
        window.draw(buttonGlow);      // Рисуем светящийся контур
        window.draw(button);
        window.draw(buttonTextShadow); // Рисуем тень
        window.draw(buttonText);       // Рисуем основной текст
    }

private:
    sf::RectangleShape button;
    sf::RectangleShape buttonGlow;    // Светящийся контур
    sf::Text buttonText;
    sf::Text buttonTextShadow;        // Текст для тени
    sf::Font font;
    bool isHovered;
    bool isPressed;
};

int main() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "Futuristic Buttons", sf::Style::Close);
    window.setFramerateLimit(60);

    sf::Texture buttonTexture;
    if (!buttonTexture.loadFromFile("/home/arthur/Art/Terminal/1.png")) { // Замените "texture.jpg" на путь к вашей текстуре
        std::cerr << "Ошибка при загрузке текстуры" << std::endl;
        return -1;
    }
        sf::Texture buttonTexture1;
    if (!buttonTexture1.loadFromFile("/home/arthur/Art/Terminal/2.png")) { // Замените "texture.jpg" на путь к вашей текстуре
        std::cerr << "Ошибка при загрузке текстуры" << std::endl;
        return -1;
    }
        sf::Texture buttonTexture2;
    if (!buttonTexture2.loadFromFile("/home/arthur/Art/Terminal/3.png")) { // Замените "texture.jpg" на путь к вашей текстуре
        std::cerr << "Ошибка при загрузке текстуры" << std::endl;
        return -1;
    }

    FuturisticButton button1(sf::Vector2f(300, 200), sf::Vector2f(200, 50), L"", buttonTexture);
    FuturisticButton button2(sf::Vector2f(300, 300), sf::Vector2f(200, 50), L"", buttonTexture1);
    FuturisticButton button3(sf::Vector2f(300, 400), sf::Vector2f(200, 50), L"", buttonTexture2);

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }

            button1.handleEvent(event, window);
            button2.handleEvent(event, window);
            button3.handleEvent(event, window);
        }

        window.clear();
        button1.draw(window);
        button2.draw(window);
        button3.draw(window);
        window.display();
    }

    return 0;
}
