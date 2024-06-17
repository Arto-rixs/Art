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
        sf::Socket::Status status = socket.connect("127.0.0.1", 13345);
        if (status != sf::Socket::Done) {
            logMessage("Failed to connect to Lua server: " + std::to_string(status));
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
    float open;
    float close;
    float high;
    float low;
    };


void updateButtonPosition(sf::RectangleShape& button, float xPercent, float yPercent, float widthPercent, float heightPercent, const sf::RenderWindow& window) {
    float x = xPercent * window.getSize().x;
    float y = yPercent * window.getSize().y;
    float width = widthPercent * window.getSize().x;
    float height = heightPercent * window.getSize().y;
    button.setPosition(x, y);
    button.setSize(sf::Vector2f(width, height));
}

void drawCandlestickChart(sf::RenderWindow& window, const std::vector<Candle>& data, const sf::Vector2f& position, const sf::Vector2f& size) {
    if (data.empty()) return;

    float xStep = size.x / data.size();
    float yMin = std::min_element(data.begin(), data.end(), [](const Candle& a, const Candle& b) { return a.low < b.low; })->low;
    float yMax = std::max_element(data.begin(), data.end(), [](const Candle& a, const Candle& b) { return a.high < b.high; })->high;
    float yRange = yMax - yMin;

    for (size_t i = 0; i < data.size(); ++i) {
        float x = position.x + i * xStep;
        float yOpen = position.y + size.y - (data[i].open - yMin) / yRange * size.y;
        float yClose = position.y + size.y - (data[i].close - yMin) / yRange * size.y;
        float yHigh = position.y + size.y - (data[i].high - yMin) / yRange * size.y;
        float yLow = position.y + size.y - (data[i].low - yMin) / yRange * size.y;

        // Проверка на выход свечи за рамки mainArea
        if (x < position.x || x > position.x + size.x || yHigh < position.y || yLow > position.y + size.y)
            continue; // Пропускаем отрисовку свечи, если она выходит за границы mainArea

        // Коррекция координат свечи, чтобы она не выходила за рамки mainArea
        if (yOpen < position.y) yOpen = position.y;
        if (yOpen > position.y + size.y) yOpen = position.y + size.y;
        if (yClose < position.y) yClose = position.y;
        if (yClose > position.y + size.y) yClose = position.y + size.y;
        if (yHigh < position.y) yHigh = position.y;
        if (yHigh > position.y + size.y) yHigh = position.y + size.y;
        if (yLow < position.y) yLow = position.y;
        if (yLow > position.y + size.y) yLow = position.y + size.y;

        sf::RectangleShape body(sf::Vector2f(xStep * 0.8f, std::abs(yClose - yOpen)));
        body.setPosition(x + xStep * 0.1f, std::min(yOpen, yClose));
        body.setFillColor(data[i].close >= data[i].open ? sf::Color::Green : sf::Color::Red);

        sf::Vertex line[] = {
            sf::Vertex(sf::Vector2f(x + xStep * 0.5f, yLow), sf::Color::White),
            sf::Vertex(sf::Vector2f(x + xStep * 0.5f, yHigh), sf::Color::White)
        };

        window.draw(line, 2, sf::Lines);
        window.draw(body);
    }
}



int main() {
    std::thread networkThread(networkThreadFunc);

    sf::RenderWindow window(sf::VideoMode(1920, 1080), L"Клиен-Терминал");
    tgui::GuiSFML gui(window);

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

    // Создание кнопки "Сделки"
    tgui::Button::Ptr dealsButton = tgui::Button::create();
    dealsButton->setSize(100, 40);
    dealsButton->setPosition(70, 5);
    dealsButton->setText("Сделки");
    gui.add(dealsButton);

    // Создание кнопки "Таблица-сделок"
    tgui::Button::Ptr dealsTableButton = tgui::Button::create();
    dealsTableButton->setSize(122, 40);
    dealsTableButton->setPosition(70 + 4 * 110 + 10, 5);
    dealsTableButton->setText("Таблица-сделок");
    gui.add(dealsTableButton);

    // Панель терминала
    sf::RectangleShape terminalPanel(sf::Vector2f(window.getSize().x, 100));
    terminalPanel.setFillColor(sf::Color(50, 50, 50));
    terminalPanel.setPosition(0, window.getSize().y - 100);

    sf::Text terminalText;
    terminalText.setFont(font);
    terminalText.setCharacterSize(18);
    terminalText.setFillColor(sf::Color::White);
    terminalText.setPosition(10, window.getSize().y - 90);

    // Создаем дочернее окно
    tgui::ChildWindow::Ptr windowPtr = tgui::ChildWindow::create();
    windowPtr->setPosition(220, 200);
    windowPtr->setSize(500, 300);
    windowPtr->setTitle("Список сделок");
    gui.add(windowPtr);

    // Создаем ListView в дочернем окне
    tgui::ListView::Ptr dealsListView = tgui::ListView::create();
    dealsListView->addColumn("ID", 100);
    dealsListView->addColumn("Symbol", 100);
    dealsListView->addColumn("Quantity", 100);
    dealsListView->addColumn("Price", 100);
    windowPtr->add(dealsListView);

    // Кнопка для открытия и закрытия окна
    tgui::Button::Ptr toggleButton = tgui::Button::create();
    toggleButton->setPosition(200, 10);
    toggleButton->setText("Таблица-Сделок");
    gui.add(toggleButton);

    // Настройка действий кнопки для открытия и закрытия окна
    bool isWindowOpen = false;
    toggleButton->onPress([&]() {
        if (isWindowOpen) {
            windowPtr->setVisible(false);
            isWindowOpen = false;
        } else {
            windowPtr->setVisible(true);
            isWindowOpen = true;
        }
    });

    std::vector<sf::Text> terminalMessages;
    const int maxVisibleMessages = 2; // Максимальное количество видимых сообщений

      //Рисуем свечи
   std::vector<Candle> sampleData = {
{78.0f, 80.0f, 81.0f, 77.0f},
{80.0f, 82.0f, 83.0f, 79.0f},
 };

    
    // Создание поля ввода команды
    tgui::EditBox::Ptr commandInput = tgui::EditBox::create();
    commandInput->setSize(300, 40);
    commandInput->setPosition(10, window.getSize().y - 50);
    commandInput->setDefaultText("Введите команду...");
    gui.add(commandInput);

    // Создание кнопки отправки команды
    tgui::Button::Ptr sendCommandButton = tgui::Button::create();
    sendCommandButton->setSize(100, 40);
    sendCommandButton->setPosition(320, window.getSize().y - 50);
    sendCommandButton->setText("Отправить");
    gui.add(sendCommandButton);

    // Установка действий на кнопки
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

            if (event.type == sf::Event::Closed) {
                window.close();
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
      // Отрисовка графика свечей
        drawCandlestickChart(window, sampleData, sf::Vector2f(80, 70), sf::Vector2f(800, 600));

        gui.draw();
        window.display();
    }

    running = false;
    networkThread.join();
    logFile.close();
    return 0;
}
