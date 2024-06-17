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

bool тринер = true;
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

// Функция для преобразования std::wstring в std::string
std::string wstringToString(const std::wstring& wstr) {
    std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
    return conv.to_bytes(wstr);
}

// Функция для преобразования std::string в std::wstring
std::wstring stringToWstring(const std::string& str) {
    std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
    return conv.from_bytes(str);
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
    float open;
    float high;
    float low;
    float close;
};

struct GraphSettings {
    float scaleX = 1.0f;
    float scaleY = 1.0f;
    float offsetX = 0.0f;
    float offsetY = 0.0f;
};

Candle parseCandleData(const std::string& data) {
    std::stringstream ss(data);
    std::string trade_num, date;
    float open, high, low, close;

    std::getline(ss, trade_num, ';');
    std::getline(ss, date, ';');
    ss >> open;
    ss.ignore(1, ';');
    ss >> high;
    ss.ignore(1, ';');
    ss >> low;
    ss.ignore(1, ';');
    ss >> close;

    return {date, open, high, low, close,};
}

void drawCandlestickChart(sf::RenderWindow& window, const std::vector<Candle>& data, const sf::Vector2f& position, const sf::Vector2f& size, GraphSettings& settings) {
    if (data.empty()) return;

    float xStep = size.x / (data.size() * settings.scaleX);
    
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
    hoverText.setFillColor(sf::Color(0, 119, 190));

    for (size_t i = 0; i < data.size(); ++i) {
        float x = position.x + i * xStep - settings.offsetX;
        float yOpen = position.y + size.y - (data[i].open - yMin) / yRange * size.y + settings.offsetY;
        float yHigh = position.y + size.y - (data[i].high - yMin) / yRange * size.y + settings.offsetY;
        float yLow = position.y + size.y - (data[i].low - yMin) / yRange * size.y + settings.offsetY;
        float yClose = position.y + size.y - (data[i].close - yMin) / yRange * size.y + settings.offsetY;

        if (x < position.x || x > position.x + size.x || yHigh < position.y || yLow > position.y + size.y)
            continue;

        yOpen = std::clamp(yOpen, position.y, position.y + size.y);
        yHigh = std::clamp(yHigh, position.y, position.y + size.y);
        yLow = std::clamp(yLow, position.y, position.y + size.y);
        yClose = std::clamp(yClose, position.y, position.y + size.y);

        sf::RectangleShape body(sf::Vector2f(xStep * 0.8f, std::abs(yClose - yOpen)));
        body.setPosition(x + xStep * 0.1f, std::min(yOpen, yClose));
        body.setFillColor(data[i].close >= data[i].open ? sf::Color::Green : sf::Color::Red);
        body.setOutlineThickness(2);
        body.setOutlineColor(sf::Color(70, 130, 180, 128));

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

    sf::RectangleShape topPanel(sf::Vector2f(window.getSize().x, 70));
    topPanel.setFillColor(sf::Color(30, 30, 30));

    sf::RectangleShape sidePanel(sf::Vector2f(60, window.getSize().y));
    sidePanel.setFillColor(sf::Color(30, 30, 30));

    sf::RectangleShape mainArea(sf::Vector2f(window.getSize().x - 60, window.getSize().y - 150));
    mainArea.setFillColor(sf::Color(61, 89, 171, 100));
    mainArea.setPosition(60, 70);

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

    sf::RectangleShape terminalPanel(sf::Vector2f(window.getSize().x, 80));
    terminalPanel.setFillColor(sf::Color(50, 50, 50));
    terminalPanel.setPosition(0, window.getSize().y - 80);

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
    {"1988-06-24", 15.64f, 15.66f, 15.56f, 15.65f},
    {"1988-06-27", 15.10f, 15.10f, 15.10f, 15.10f},
    {"1988-06-28", 15.34f, 15.36f, 15.27f, 15.27f},
    {"1988-06-29", 15.47f, 15.47f, 15.47f, 15.47f},
    {"1988-06-30", 14.80f, 14.90f, 14.80f, 14.85f},
    {"1988-07-01", 14.60f, 14.60f, 14.60f, 14.60f},
    {"1988-07-04", 14.50f, 14.50f, 14.30f, 14.35f},
    {"1988-07-05", 14.45f, 14.58f, 14.30f, 14.30f},
    {"1988-07-06", 14.60f, 14.68f, 14.49f, 14.68f},
    {"1988-07-07", 14.95f, 15.45f, 14.90f, 15.45f},
    {"1988-07-08", 15.12f, 15.12f, 15.00f, 15.05f},
    {"1988-07-11", 14.70f, 14.70f, 14.50f, 14.50f},
    {"1988-07-12", 15.12f, 15.12f, 15.00f, 14.10f},
    {"1988-07-13", 14.05f, 14.17f, 13.90f, 14.17f},
    {"1988-07-14", 13.85f, 14.06f, 13.79f, 14.06f},
    {"1988-07-15", 14.30f, 14.40f, 14.18f, 14.18f},
    {"1988-07-18", 14.25f, 14.85f, 14.20f, 14.80f},
    {"1988-07-19", 15.15f, 15.25f, 14.88f, 15.08f},
    {"1988-07-20", 14.80f, 15.13f, 14.80f, 15.13f},
    {"1988-07-21", 15.35f, 15.62f, 15.30f, 15.45f},
    {"1988-07-22", 15.60f, 15.71f, 15.45f, 15.71f},
    {"1988-07-25", 15.70f, 15.90f, 15.68f, 15.69f},
    {"1988-07-26", 15.52f, 15.52f, 15.29f, 15.32f},
    {"1988-07-27", 15.25f, 15.44f, 15.22f, 15.42f},
    {"1988-07-28", 15.50f, 15.71f, 15.50f, 15.67f},
    {"1988-07-29", 15.60f, 15.70f, 15.60f, 15.70f},
    {"1988-08-01", 15.64f, 15.64f, 15.55f, 15.56f},
    {"1988-08-02", 15.35f, 15.42f, 15.35f, 15.38f},
    {"1988-08-03", 15.00f, 15.25f, 14.96f, 15.00f},
    {"1988-08-04", 14.58f, 14.60f, 14.48f, 14.48f},
    {"1988-08-06", 14.75f, 14.75f, 14.63f, 14.63f},
    {"1988-08-08", 15.20f, 15.35f, 15.20f, 15.30f},
    {"1988-08-09", 15.45f, 15.45f, 15.27f, 15.34f},
    {"1988-08-10", 15.08f, 15.08f, 15.00f, 15.08f},
    {"1988-08-11", 15.10f, 15.15f, 15.05f, 15.09f},
    {"2023-02-06", 93.30f, 96.00f, 92.81f, 93.55f},
    {"2023-02-07", 93.39f, 93.86f, 92.06f, 92.35f},
    {"2023-02-08", 92.24f, 92.24f, 88.12f, 89.26f},
    {"2023-02-09", 89.27f, 89.53f, 84.97f, 85.22f},
    {"2023-02-10", 85.15f, 90.00f, 84.17f, 89.67f},
    {"2023-02-11", 89.55f, 90.20f, 86.17f, 86.56f},
    {"2023-02-12", 86.59f, 87.35f, 85.56f, 86.18f},
    {"2023-02-13", 86.15f, 87.80f, 85.92f, 86.90f},
    {"2023-02-14", 86.76f, 86.93f, 82.16f, 83.37f},
    {"2023-02-15", 83.34f, 86.29f, 83.28f, 85.07f},
    {"2023-02-16", 85.31f, 85.73f, 83.54f, 85.48f},
    {"2023-02-17", 88.14f, 90.33f, 87.86f, 89.90f},
    {"2023-02-18", 89.90f, 89.91f, 88.25f, 89.10f},
    {"2023-02-19", 89.06f, 90.63f, 88.00f, 89.08f},
    {"2023-02-20", 89.00f, 89.15f, 85.38f, 85.65f},
    {"2023-02-21", 85.74f, 85.79f, 82.40f, 83.13f},
    {"2023-02-22", 83.00f, 83.05f, 81.20f, 81.58f},
    {"2023-02-23", 81.43f, 81.95f, 80.04f, 80.21f},
    {"2023-02-24", 80.30f, 80.97f, 78.67f, 80.86f},
    {"2023-02-25", 80.91f, 82.53f, 80.34f, 82.09f},
    {"2023-02-26", 82.20f, 83.11f, 81.56f, 82.77f},
    {"2023-02-27", 82.85f, 83.85f, 82.55f, 83.33f},
    {"2023-02-28", 83.20f, 83.25f, 80.35f, 80.64f},
    {"2023-03-01", 80.60f, 81.49f, 80.00f, 80.88f}
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
                sf::Text message(wstringToString(response), font, 18);
                message.setFillColor(sf::Color::White);
                sampleData.push_back(parseCandleData(wstringToString(response)));
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

        drawCandlestickChart(window, sampleData, sf::Vector2f(80, 70), sf::Vector2f(mainArea.getSize().x, mainArea.getSize().y), graphSettings);

        gui.draw();
        window.display();
    }

    running = false;
    networkThread.join();
    logFile.close();
    return 0;
}
