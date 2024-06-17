#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <fstream>
#include <SFML/Network.hpp>
#include <SFML/Graphics.hpp>
#include "json.hpp"
#include <TGUI/TGUI.hpp>

using json = nlohmann::json;

double tablica;

std::vector<std::wstring> responses;
std::mutex dataMutex;
std::atomic<bool> running(true);
std::string commandToSend;

std::ofstream logFile("client_log.txt");

void logMessage(const std::string& message) {
    logFile << message << std::endl;
    logFile.flush();
}

std::wstring utf8_to_wstring(const std::string& str) {
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    return converter.from_bytes(str);
}

void networkThreadFunc() {
    try {
        sf::TcpSocket socket;
        sf::Socket::Status status = socket.connect("127.0.0.1", 13345);
        if (status != sf::Socket::Done) {
            logMessage("Failed to connect to Lua server: " + std::to_string(status));
            return;
        }
        logMessage("Connected to server");

        char buffer[4096];
        std::size_t received;

        while (running) {
            if (!commandToSend.empty()) {
                std::string message = commandToSend + "\n";
                logMessage("Sending command to server: " + message);
                if (socket.send(message.c_str(), message.size()) != sf::Socket::Done) {
                    logMessage("Failed to send command to server");
                } else {
                    logMessage("Command sent successfully");

                    // Получение ответа от сервера
                    sf::Socket::Status receiveStatus = socket.receive(buffer, sizeof(buffer), received);
                    if (receiveStatus == sf::Socket::Done) {
                        std::string receivedData(buffer, received);
                        logMessage("Received data: " + receivedData);
                        std::lock_guard<std::mutex> lock(dataMutex);
                        responses.push_back(utf8_to_wstring(receivedData));
                    } else if (receiveStatus != sf::Socket::NotReady) {
                        logMessage("Failed to receive data from Lua server: " + std::to_string(receiveStatus));
                    }
                }
                commandToSend.clear();
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        socket.disconnect();
    } catch (const std::exception& e) {
        logMessage(std::string("Exception in network thread: ") + e.what());
    } catch (...) {
        logMessage("Unknown exception in network thread");
    }
}

sf::RectangleShape createButton(float x, float y, const sf::Color& color, const sf::Vector2f& size = sf::Vector2f(100, 40)) {
    sf::RectangleShape button(size);
    button.setFillColor(color);
    button.setPosition(x, y);
    return button;
}

void updateButtonPosition(sf::RectangleShape& button, float xPercent, float yPercent, float widthPercent, float heightPercent, const sf::RenderWindow& window) {
    float x = xPercent * window.getSize().x;
    float y = yPercent * window.getSize().y;
    float width = widthPercent * window.getSize().x;
    float height = heightPercent * window.getSize().y;
    button.setPosition(x, y);
    button.setSize(sf::Vector2f(width, height));
}

void drawGraph(sf::RenderWindow& window, const std::vector<float>& data, const sf::Vector2f& position, const sf::Vector2f& size) {
    if (data.empty()) return;

    sf::VertexArray lines(sf::LinesStrip, data.size());
    float xStep = size.x / (data.size() - 1);
    float yMin = *std::min_element(data.begin(), data.end());
    float yMax = *std::max_element(data.begin(), data.end());
    float yRange = yMax - yMin;

    for (size_t i = 0; i < data.size(); ++i) {
        float x = position.x + i * xStep;
        float y = position.y + size.y - (data[i] - yMin) / yRange * size.y;
        lines[i] = sf::Vertex(sf::Vector2f(x, y), sf::Color::Green);
    }

    window.draw(lines);
}

int main() {
    std::thread networkThread(networkThreadFunc);

    sf::RenderWindow window(sf::VideoMode(1920, 1080), L"Клиен-Терминал");

    sf::Font font;
    if (!font.loadFromFile("/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf")) {
        logMessage("Failed to load font 'LiberationSans-Regular.ttf'");
        return 1;
    }

    sf::Text text;
    text.setFont(font);
    text.setCharacterSize(24);
    text.setFillColor(sf::Color::White);

    // Верхняя панель
    sf::RectangleShape topPanel(sf::Vector2f(window.getSize().x, 50));
    topPanel.setFillColor(sf::Color(30, 30, 30));

    // Боковая панель
    sf::RectangleShape sidePanel(sf::Vector2f(60, window.getSize().y));
    sidePanel.setFillColor(sf::Color(30, 30, 30));

    // Основная область
    sf::RectangleShape mainArea(sf::Vector2f(window.getSize().x - 60, window.getSize().y - 150)); // Уменьшено для терминала
    mainArea.setFillColor(sf::Color(20, 20, 20));
    mainArea.setPosition(60, 50);

    // Кнопки на боковой панели
    std::vector<sf::RectangleShape> sideButtons;
    for (int i = 0; i < 10; ++i) {
        sf::RectangleShape button = createButton(5, 60 + i * 55, sf::Color(50, 50, 50), sf::Vector2f(50, 50));
        sideButtons.push_back(button);
    }

    // Кнопки на верхней панели
    std::vector<sf::RectangleShape> topButtons;
    for (int i = 0; i < 4; ++i) {
        sf::RectangleShape button = createButton(70 + i * 110, 5, sf::Color(50, 50, 50), sf::Vector2f(100, 40));
        topButtons.push_back(button);
    }

    // Первая кнопка верхней панели - кнопка "Сделки"
    topButtons[0].setFillColor(sf::Color::Blue);
    sf::Text buttonText(L"Сделки", font, 20);
    buttonText.setFillColor(sf::Color::White);
    buttonText.setPosition(80, 10);

    // Новая кнопка "Таблица-сделок"
    sf::RectangleShape allTradeButton = createButton(70 + 4 * 110, 5, sf::Color(50, 50, 50), sf::Vector2f(150, 40));
    sf::Text allTradeButtonText(L"Таблица-сделок", font, 15);
    allTradeButtonText.setFillColor(sf::Color::White);
    allTradeButtonText.setPosition(70 + 4 * 110 + 10, 10);

    sf::RectangleShape terminalPanel(sf::Vector2f(window.getSize().x, 100));
    terminalPanel.setFillColor(sf::Color(50, 50, 50));
    terminalPanel.setPosition(0, window.getSize().y - 100);

    sf::Text terminalText;
    terminalText.setFont(font);
    terminalText.setCharacterSize(18);
    terminalText.setFillColor(sf::Color::White);
    terminalText.setPosition(10, window.getSize().y - 90);

    std::vector<sf::Text> terminalMessages;
    const int maxVisibleMessages = 5; // Максимальное количество видимых сообщений

    std::vector<float> graphData = {78.63, 78.64, 78.65, 78.63, 78.62, 78.60, 78.61, 78.59, 78.58, 78.57};

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
                running = false;
                networkThread.join();
            } else if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    if (topButtons[0].getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y)) {
                        commandToSend = "cdelku";
                        logMessage("Button pressed, sending command 'cdelku' to server");
                    } else if (allTradeButton.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y)) {
                        commandToSend = "AllTrade";
                        logMessage("Button pressed, sending command 'AllTrade' to server");
                    }
                }
            }
        }

        window.clear();
        window.draw(topPanel);
        window.draw(sidePanel);
        window.draw(mainArea);
        for (const auto& button : sideButtons) {
            window.draw(button);
        }
        for (const auto& button : topButtons) {
            window.draw(button);
        }
        window.draw(buttonText);
        window.draw(allTradeButton);
        window.draw(allTradeButtonText);
        window.draw(terminalPanel);

        std::lock_guard<std::mutex> lock(dataMutex);
        for (const auto& response : responses) {
            terminalMessages.push_back(sf::Text(response, font, 18));
        }
        responses.clear();

        while (terminalMessages.size() > maxVisibleMessages) {
            terminalMessages.erase(terminalMessages.begin());
        }

        float yOffset = 0;
        for (const auto& message : terminalMessages) {
            sf::Text tempText = message;
            tempText.setPosition(10, window.getSize().y - 90 + yOffset);
            window.draw(tempText);
            yOffset += 20; // Регулируйте этот отступ по своему усмотрению
        }

        drawGraph(window, graphData, sf::Vector2f(60, 50), sf::Vector2f(window.getSize().x - 60, window.getSize().y - 150));

        window.display();
    }

    logFile.close();
    return 0;
}
