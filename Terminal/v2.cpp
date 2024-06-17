#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <fstream>
#include <locale>
#include <codecvt>
#include <SFML/Network.hpp>
#include <SFML/Graphics.hpp>
#include "json.hpp"
#include <TGUI/TGUI.hpp>

using json = nlohmann::json;

std::vector<std::wstring> responses;
std::mutex dataMutex;
std::mutex commandMutex;
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
        if (socket.connect("127.0.0.1", 13345) != sf::Socket::Done) {
            logMessage("Failed to connect to Lua server");
            return;
        }
        logMessage("Connected to server");

        char buffer[4096];
        std::size_t received;

        while (running) {
            std::string command;
            {
                std::lock_guard<std::mutex> lock(commandMutex);
                command = commandToSend;
            }

            if (!command.empty()) {
                std::string message = command + "\n";
                logMessage("Sending command to server: " + message);
                if (socket.send(message.c_str(), message.size()) != sf::Socket::Done) {
                    logMessage("Failed to send command to server");
                } else {
                    logMessage("Command sent successfully");

                    if (socket.receive(buffer, sizeof(buffer), received) == sf::Socket::Done) {
                        std::string receivedData(buffer, received);
                        logMessage("Received data: " + receivedData);
                        std::lock_guard<std::mutex> lock(dataMutex);
                        responses.push_back(utf8_to_wstring(receivedData));
                    }
                }

                {
                    std::lock_guard<std::mutex> lock(commandMutex);
                    commandToSend.clear();
                }
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

struct Candle {
    std::string date;
    float open, close, high, low;
};

struct GraphSettings {
    float scaleX = 1.0f;
    float scaleY = 1.0f;
    float offsetX = 0.0f;
    float offsetY = 0.0f;
};

void drawCandlestickChart(sf::RenderWindow& window, const std::vector<Candle>& data, const sf::Vector2f& position, const sf::Vector2f& size, GraphSettings& settings) {
    if (data.empty()) return;

    float xStep = size.x / data.size() * settings.scaleX;
    auto [yMinIt, yMaxIt] = std::minmax_element(data.begin(), data.end(), [](const Candle& a, const Candle& b) {
        return a.low < b.low; 
    });
    float yMin = yMinIt->low;
    float yMax = yMaxIt->high;
    float yRange = (yMax - yMin) * settings.scaleY;

    sf::Font font;
    if (!font.loadFromFile("/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf")) {
        logMessage("Failed to load font 'LiberationSans-Regular.ttf'");
        return;
    }

    sf::Text hoverText;
    hoverText.setFont(font);
    hoverText.setCharacterSize(14);
    hoverText.setFillColor(sf::Color::White);

    for (size_t i = 0; i < data.size(); ++i) {
        float x = position.x + i * xStep - settings.offsetX;
        float yOpen = position.y + size.y - (data[i].open - yMin) / yRange * size.y + settings.offsetY;
        float yClose = position.y + size.y - (data[i].close - yMin) / yRange * size.y + settings.offsetY;
        float yHigh = position.y + size.y - (data[i].high - yMin) / yRange * size.y + settings.offsetY;
        float yLow = position.y + size.y - (data[i].low - yMin) / yRange * size.y + settings.offsetY;

        if (x < position.x || x > position.x + size.x || yHigh < position.y || yLow > position.y + size.y)
            continue;

        yOpen = std::clamp(yOpen, position.y, position.y + size.y);
        yClose = std::clamp(yClose, position.y, position.y + size.y);
        yHigh = std::clamp(yHigh, position.y, position.y + size.y);
        yLow = std::clamp(yLow, position.y, position.y + size.y);

        sf::RectangleShape body(sf::Vector2f(xStep * 0.8f, std::abs(yClose - yOpen)));
        body.setPosition(x + xStep * 0.1f, std::min(yOpen, yClose));
        body.setFillColor(data[i].close >= data[i].open ? sf::Color::Green : sf::Color::Red);

        sf::Vertex line[] = {
            sf::Vertex(sf::Vector2f(x + xStep * 0.5f, yLow), sf::Color::White),
            sf::Vertex(sf::Vector2f(x + xStep * 0.5f, yHigh), sf::Color::White)
        };

        window.draw(line, 2, sf::Lines);
        window.draw(body);

        if (sf::Mouse::getPosition(window).x >= x && sf::Mouse::getPosition(window).x <= x + xStep) {
            hoverText.setPosition(sf::Mouse::getPosition(window).x + 10, sf::Mouse::getPosition(window).y);
            hoverText.setString("Date: " + data[i].date + "\nOpen: " + std::to_string(data[i].open) + "\nClose: " + std::to_string(data[i].close) + "\nHigh: " + std::to_string(data[i].high) + "\nLow: " + std::to_string(data[i].low));
        }
    }
    window.draw(hoverText);
}

int main() {
    std::thread networkThread(networkThreadFunc);

    sf::RenderWindow window(sf::VideoMode(1920, 1080), L"Клиен-Терминал");
    tgui::GuiSFML gui(window);

    GraphSettings graphSettings;

    sf::Font font;
    if (!font.loadFromFile("/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf")) {
        logMessage("Failed to load font 'LiberationSans-Regular.ttf'");
        return 1;
    }

    sf::Text text;
    text.setFont(font);
    text.setCharacterSize(24);
    text.setFillColor(sf::Color::White);

    sf::RectangleShape topPanel(sf::Vector2f(window.getSize().x, 50));
    topPanel.setFillColor(sf::Color(30, 30, 30));

    sf::RectangleShape sidePanel(sf::Vector2f(60, window.getSize().y));
    sidePanel.setFillColor(sf::Color(30, 30, 30));

    sf::RectangleShape mainArea(sf::Vector2f(window.getSize().x - 60, window.getSize().y - 150));
    mainArea.setFillColor(sf::Color(20, 20, 20));
    mainArea.setPosition(60, 50);

    tgui::Button::Ptr dealsButton = tgui::Button::create();
    dealsButton->setSize(100, 40);
    dealsButton->setPosition(70, 5);
    dealsButton->setText("Сделки");
    gui.add(dealsButton);

    tgui::Button::Ptr dealsTableButton = tgui::Button::create();
    dealsTableButton->setSize(122, 40);
    dealsTableButton->setPosition(70 + 4 * 110 + 10, 5);
    dealsTableButton->setText("Таблица-сделок");
    gui.add(dealsTableButton);

    sf::RectangleShape terminalPanel(sf::Vector2f(window.getSize().x, 100));
    terminalPanel.setFillColor(sf::Color(50, 50, 50));
    terminalPanel.setPosition(0, window.getSize().y - 100);

    sf::Text terminalText;
    terminalText.setFont(font);
    terminalText.setCharacterSize(18);
    terminalText.setFillColor(sf::Color::White);
    terminalText.setPosition(10, window.getSize().y - 90);

    tgui::ChildWindow::Ptr windowPtr = tgui::ChildWindow::create();
    windowPtr->setPosition(220, 200);
    windowPtr->setSize(500, 300);
    windowPtr->setTitle("Список сделок");
    gui.add(windowPtr);

    tgui::ListView::Ptr dealsListView = tgui::ListView::create();
    dealsListView->addColumn("ID", 100);
    dealsListView->addColumn("Symbol", 100);
    dealsListView->addColumn("Quantity", 100);
    dealsListView->addColumn("Price", 100);
    windowPtr->add(dealsListView);

    tgui::Button::Ptr toggleButton = tgui::Button::create();
    toggleButton->setPosition(200, 10);
    toggleButton->setText("Таблица-Сделок");
    gui.add(toggleButton);

    bool isWindowOpen = false;
    toggleButton->onPress([&]() {
        isWindowOpen = !isWindowOpen;
        windowPtr->setVisible(isWindowOpen);
    });

    std::vector<sf::Text> terminalMessages;
    const int maxVisibleMessages = 2;

    std::vector<Candle> sampleData = {
        {"2023-01-01", 78.0f, 80.0f, 81.0f, 77.0f},
        {"2023-01-02", 80.0f, 82.0f, 83.0f, 79.0f},
        // Добавьте остальные данные
    };

    tgui::EditBox::Ptr commandInput = tgui::EditBox::create();
    commandInput->setSize(300, 40);
    commandInput->setPosition(10, window.getSize().y - 50);
    commandInput->setDefaultText("Введите команду...");
    gui.add(commandInput);

    tgui::Button::Ptr sendCommandButton = tgui::Button::create();
    sendCommandButton->setSize(100, 40);
    sendCommandButton->setPosition(320, window.getSize().y - 50);
    sendCommandButton->setText("Отправить");
    gui.add(sendCommandButton);

    dealsButton->onPress([&]() {
        logMessage("Deals button pressed, sending command: cdelku");
        {
            std::lock_guard<std::mutex> lock(commandMutex);
            commandToSend = "cdelku";
        }
    });

    dealsTableButton->onPress([&]() {
        logMessage("DealsTable button pressed, sending command: OnAllTrade");
        {
            std::lock_guard<std::mutex> lock(commandMutex);
            commandToSend = "OnAllTrade";
        }
    });

    sendCommandButton->onPress([&]() {
        std::string command = commandInput->getText().toStdString();
        logMessage("SendCommand button pressed, sending command: " + command);
        {
            std::lock_guard<std::mutex> lock(commandMutex);
            commandToSend = command;
        }
        commandInput->setText("");
    });

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
                running = false;
                networkThread.join();
            }

            if (event.type == sf::Event::MouseWheelScrolled) {
                if (event.mouseWheelScroll.delta > 0) {
                    graphSettings.scaleX *= 1.1f;
                    graphSettings.scaleY *= 1.1f;
                } else {
                    graphSettings.scaleX /= 1.1f;
                    graphSettings.scaleY /= 1.1f;
                }
            }

            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Left) {
                    graphSettings.offsetX -= 10.0f;
                } else if (event.key.code == sf::Keyboard::Right) {
                    graphSettings.offsetX += 10.0f;
                } else if (event.key.code == sf::Keyboard::Up) {
                    graphSettings.offsetY -= 10.0f;
                } else if (event.key.code == sf::Keyboard::Down) {
                    graphSettings.offsetY += 10.0f;
                }
            }

            gui.handleEvent(event);
        }

        {
            std::lock_guard<std::mutex> lock(dataMutex);
            for (const auto& response : responses) {
                sf::Text message(response, font, 18);
                message.setFillColor(sf::Color::White);
                message.setPosition(10, window.getSize().y - 90 - terminalMessages.size() * 20);
                terminalMessages.push_back(message);
                if (terminalMessages.size() > maxVisibleMessages) {
                    terminalMessages.erase(terminalMessages.begin());
                }
            }
            responses.clear();
        }

        window.clear();
        window.draw(topPanel);
        window.draw(sidePanel);
        window.draw(mainArea);
        window.draw(terminalPanel);

        for (const auto& message : terminalMessages) {
            window.draw(message);
        }

        drawCandlestickChart(window, sampleData, sf::Vector2f(80, 70), sf::Vector2f(800, 600), graphSettings);

        gui.draw();
        window.display();
    }

    running = false;
    networkThread.join();
    logFile.close();
    return 0;
}
