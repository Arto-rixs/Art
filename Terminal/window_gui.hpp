// window_gui.hpp
#pragma once
#include <SFML/Graphics.hpp>
#include <TGUI/TGUI.hpp>
#include <vector>
#include <string>
#include <mutex>

void setupGUI(tgui::Gui& gui, sf::RenderWindow& window, std::vector<sf::Text>& terminalMessages, sf::Font& font, std::mutex& dataMutex, std::vector<std::wstring>& responses, std::mutex& commandMutex, std::string& commandToSend);
void drawGraphics(sf::RenderWindow& window, tgui::Gui& gui, const std::vector<sf::Text>& terminalMessages, const std::vector<std::pair<std::string, std::pair<float, float>>>& sampleData);

// Объявление функции drawCandlestickChart
void drawCandlestickChart(sf::RenderWindow& window, const std::vector<std::pair<std::string, std::pair<float, float>>>& data, const sf::Vector2f& position, const sf::Vector2f& size);
