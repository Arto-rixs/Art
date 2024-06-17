#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>

// Функция для создания фона с градиентом
void createGradientBackground(sf::RenderWindow& window, sf::VertexArray& background) {
    sf::Color topColor(0, 10, 30);  // Темно-синий цвет
    sf::Color bottomColor(0, 50, 90);  // Синий цвет
    
    background.setPrimitiveType(sf::Quads);
    background.resize(4);
    
    background[0].position = sf::Vector2f(0, 0);
    background[1].position = sf::Vector2f(window.getSize().x, 0);
    background[2].position = sf::Vector2f(window.getSize().x, window.getSize().y);
    background[3].position = sf::Vector2f(0, window.getSize().y);
    
    background[0].color = topColor;
    background[1].color = topColor;
    background[2].color = bottomColor;
    background[3].color = bottomColor;
}

// Функция для создания линии с эффектом свечения
void createGlowingLine(sf::RenderWindow& window, sf::VertexArray& line, sf::Color color, float thickness) {
    float glowRadius = thickness * 5;
    
    for (int i = 0; i < 10; ++i) {
        float alpha = 255 * (1.0f - (float)i / 10.0f);
        sf::Color glowColor = color;
        glowColor.a = static_cast<sf::Uint8>(alpha);
        
        sf::Vertex v1(sf::Vector2f(0, window.getSize().y * i / 10.0f), glowColor);
        sf::Vertex v2(sf::Vector2f(window.getSize().x, window.getSize().y * i / 10.0f), glowColor);
        
        line.append(v1);
        line.append(v2);
    }
}

int main() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "Gradient Background with Glowing Line");
    
    sf::VertexArray background;
    createGradientBackground(window, background);
    
    sf::VertexArray line(sf::TrianglesStrip);
    createGlowingLine(window, line, sf::Color(0, 120, 255), 2.0f); // Синий цвет линии
    
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }
        
        window.clear();
        window.draw(background);
        window.draw(line);
        window.display();
    }

    return 0;
}
