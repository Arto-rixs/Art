#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include "/home/arthur/Art/json.hpp"

using json = nlohmann::json;

// Структура для хранения данных о котировке
struct QuoteData {
    std::string sec_code;
    double bid;
    double ask;
};

// Глобальный вектор для хранения котировок
std::vector<QuoteData> quotes;

// Функция для обработки входящих соединений
void handleClient(sf::TcpSocket* client) {
    char data[1024];
    std::size_t received;
    sf::Socket::Status status = client->receive(data, sizeof(data), received);
    if (status == sf::Socket::Done) {
        data[received] = '\0'; // Завершаем строку нулевым символом
        try {
            std::cerr << "Received data: " << data << std::endl;
            json j = json::parse(data);
            QuoteData quote;
            quote.sec_code = j["sec_code"];
            quote.bid = j["bid"];
            quote.ask = j["ask"];
            quotes.push_back(quote);
            std::cerr << "Parsed quote - sec_code: " << quote.sec_code
                      << ", bid: " << quote.bid
                      << ", ask: " << quote.ask << std::endl;

            // Отправка подтверждения клиенту
            std::string response = "Data received";
            client->send(response.c_str(), response.size());
        } catch (const nlohmann::json::exception& ex) {
            std::cerr << "JSON parsing error: " << ex.what() << std::endl;
        }
    } else {
        std::cerr << "Failed to receive data: " << status << std::endl;
    }
    delete client; // Удаление клиента после отправки ответа
}

// Функция для запуска сервера
void runServer() {
    sf::TcpListener listener;
    listener.listen(12345);
    while (true) {
        sf::TcpSocket* client = new sf::TcpSocket;
        if (listener.accept(*client) == sf::Socket::Done) {
            std::thread(handleClient, client).detach();
        } else {
            delete client;
        }
    }
}

// Функция для рисования данных на экране
void drawData(sf::RenderWindow& window) {
    window.clear();
    // Здесь можно добавить код для рисования котировок из вектора quotes
    window.display();
}

int main() {
    std::thread serverThread(runServer);
    serverThread.detach();

    sf::RenderWindow window(sf::VideoMode(800, 600), "Trading Application");

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        drawData(window);
    }

    return 0;
}
