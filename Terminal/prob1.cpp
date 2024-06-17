#include <iostream>  // ввод и вывод даных на экран
#include <string>  //  работа со строкамиь - хроник данные в строке
#include <vector> // хронилише данных в определяемом формате 
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
        while (running) {
            if (socket.connect("127.0.0.1", 13345) != sf::Socket::Done) {
                logMessage("Failed to connect to Lua server. Retrying in 5 seconds...");
                std::this_thread::sleep_for(std::chrono::seconds(5));
                continue;
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

                            // Логирование полученных данных
                            logMessage("Received data: " + receivedData);

                            // Проверка, что данные не являются "Heartbeat"
                            if (receivedData != "Heartbeat\n") {
                                try {
                                    json jsonData = json::parse(receivedData);
                                    std::lock_guard<std::mutex> lock(dataMutex);
                                    responses.push_back(utf8_to_wstring(jsonData.dump()));
                                } catch (const std::exception& e) {
                                    logMessage(std::string("Failed to parse JSON data: ") + e.what());
                                }
                            }
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
        }
    } catch (const std::exception& e) {
        logMessage(std::string("Exception in network thread: ") + e.what());
    } catch (...) {
        logMessage("Unknown exception in network thread");
    }
}

struct Trade {
    std::string date;
    float price;
};

struct Candle {
    std::string date;
    float open;
    float close;
    float high;
    float low;
};

struct GraphSettings {
    float scaleX = 1.0f;
    float scaleY = 1.0f;
    float offsetX = 0.0f;
    float offsetY = 0.0f;
};

Trade parseTradeData(const json& tradeData) {
    Trade trade;

    int year = tradeData["datetime"]["year"];
    int month = tradeData["datetime"]["month"];
    int day = tradeData["datetime"]["day"];
    int hour = tradeData["datetime"]["hour"];
    int minute = tradeData["datetime"]["min"];
    int second = tradeData["datetime"]["sec"];

    trade.date = std::to_string(year) + "-" +
                 std::to_string(month) + "-" +
                 std::to_string(day) + " " +
                 std::to_string(hour) + ":" +
                 std::to_string(minute) + ":" +
                 std::to_string(second);

    trade.price = tradeData["price"];

    return trade;
}

std::vector<Candle> createCandlesFromTrades(const std::vector<Trade>& trades, int periodInSeconds) {
    std::vector<Candle> candles;
    if (trades.empty()) {
        return candles;
    }

    auto start = trades[0];
    Candle currentCandle;
    currentCandle.date = start.date;
    currentCandle.open = start.price;
    currentCandle.high = start.price;
    currentCandle.low = start.price;
    currentCandle.close = start.price;

    for (const auto& trade : trades) {
        // Calculate the time difference in seconds between the current trade and the start of the candle
        std::tm tmStart = {}, tmTrade = {};
        std::istringstream ssStart(start.date), ssTrade(trade.date);
        ssStart >> std::get_time(&tmStart, "%Y-%m-%d %H:%M:%S");
        ssTrade >> std::get_time(&tmTrade, "%Y-%m-%d %H:%M:%S");

        auto timeStart = std::mktime(&tmStart);
        auto timeTrade = std::mktime(&tmTrade);
        auto timeDiff = std::difftime(timeTrade, timeStart);

        if (timeDiff >= periodInSeconds) {
            // Push the completed candle and start a new one
            candles.push_back(currentCandle);
            start = trade;
            currentCandle.date = trade.date;
            currentCandle.open = trade.price;
            currentCandle.high = trade.price;
            currentCandle.low = trade.price;
            currentCandle.close = trade.price;
        } else {
            // Update the current candle
            currentCandle.close = trade.price;
            if (trade.price > currentCandle.high) {
                currentCandle.high = trade.price;
            }
            if (trade.price < currentCandle.low) {
                currentCandle.low = trade.price;
            }
        }
    }

    // Push the last candle
    candles.push_back(currentCandle);

    return candles;
}

void drawCandlestickChart(sf::RenderWindow& window, const std::vector<Candle>& candles) {
    const float candleWidth = 10.0f;
    const float spacing = 5.0f;
    const float chartHeight = 400.0f;
    const float chartBottom = window.getSize().y - 50.0f;
    const float maxPrice = std::max_element(candles.begin(), candles.end(), [](const Candle& a, const Candle& b) {
        return a.high < b.high;
    })->high;

    const float minPrice = std::min_element(candles.begin(), candles.end(), [](const Candle& a, const Candle& b) {
        return a.low < b.low;
    })->low;

    for (size_t i = 0; i < candles.size(); ++i) {
        const Candle& candle = candles[i];
        float open = candle.open;
        float close = candle.close;
        float high = candle.high;
        float low = candle.low;

        float x = (candleWidth + spacing) * i;
        float openY = chartBottom - ((open - minPrice) / (maxPrice - minPrice) * chartHeight);
        float closeY = chartBottom - ((close - minPrice) / (maxPrice - minPrice) * chartHeight);
        float highY = chartBottom - ((high - minPrice) / (maxPrice - minPrice) * chartHeight);
        float lowY = chartBottom - ((low - minPrice) / (maxPrice - minPrice) * chartHeight);

        sf::RectangleShape candleBody(sf::Vector2f(candleWidth, std::abs(openY - closeY)));
        candleBody.setPosition(x, std::min(openY, closeY));
        candleBody.setFillColor((open > close) ? sf::Color::Red : sf::Color::Green);
        window.draw(candleBody);

        sf::RectangleShape candleWick(sf::Vector2f(1, highY - lowY));
        candleWick.setPosition(x + candleWidth / 2, lowY);
        candleWick.setFillColor(sf::Color::White);
        window.draw(candleWick);
    }
}

void displayThreadFunc() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "Network Data Viewer");

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        window.clear();

        std::vector<Trade> trades;
        {
            std::lock_guard<std::mutex> lock(dataMutex);
            for (const auto& response : responses) {
                json jsonData = json::parse(wstringToString(response));
                if (jsonData.is_array()) {
                    for (const auto& trade : jsonData) {
                        trades.push_back(parseTradeData(trade));
                    }
                }
            }
        }

        // Create candles from trades
        std::vector<Candle> candles = createCandlesFromTrades(trades, 60); // 60 seconds per candle

        drawCandlestickChart(window, candles);

        window.display();
    }
}

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
    std::thread networkThread(networkThreadFunc);

    sf::RenderWindow window(sf::VideoMode(1920, 1080), L"Клиен-Терминал");
    tgui::GuiSFML gui(window);
    window.setFramerateLimit(60);

    GraphSettings graphSettings;

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
        sf::Texture Картинка;
    if (!Картинка.loadFromFile("/home/arthur/Art/Terminal/фонГраф.png")) { // Замените "texture.jpg" на путь к вашей текстуре
        std::cerr << "Ошибка при загрузке текстуры" << std::endl;
        return -1;
    }
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
     mainArea.setTexture(&Картинка);
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

    std::vector<Trade> sampleData = {
        {"2023-01-01 12:00:00", 78.23f}
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

    FuturisticButton button1(sf::Vector2f(700, 10), sf::Vector2f(200, 50), L"", buttonTexture);
    FuturisticButton button2(sf::Vector2f(915, 10), sf::Vector2f(200, 50), L"", buttonTexture1);
    FuturisticButton button3(sf::Vector2f(1130, 10), sf::Vector2f(200, 50), L"", buttonTexture2);

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
            button1.handleEvent(event, window);
            button2.handleEvent(event, window);
            button3.handleEvent(event, window);
        }

        {
            std::lock_guard<std::mutex> lock(dataMutex);
            for (const auto& response : responses) {
                sf::Text message(wstringToString(response), font, 18);
                message.setFillColor(sf::Color::White);
                terminalMessages.push_back(message);

                if (terminalMessages.size() > maxVisibleMessages) {
                    terminalMessages.erase(terminalMessages.begin());
                }

                json jsonData = json::parse(wstringToString(response));
                if (jsonData.is_array()) {
                    for (const auto& trade : jsonData) {
                        sampleData.push_back(parseTradeData(trade));
                    }
                }
            }
            responses.clear();
        }

        window.clear();
        window.draw(topPanel);
        window.draw(sidePanel);
        window.draw(mainArea);
        button1.draw(window);
        button2.draw(window);
        button3.draw(window);
        window.draw(terminalPanel);

        for (const auto& message : terminalMessages) {
            window.draw(message);
        }

        // Create candles from trades
        std::vector<Candle> candles = createCandlesFromTrades(sampleData, 60); // 60 seconds per candle

        drawCandlestickChart(window, candles);

        gui.draw();
        window.display();
    }

    running = false;
    networkThread.join();
    logFile.close();
    return 0;
}
