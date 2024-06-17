// network.cpp
#include "network.hpp"
#include <locale>
#include <codecvt>

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
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
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

                    sf::Socket::Status receiveStatus = socket.receive(buffer, sizeof(buffer), received);
                    if (receiveStatus == sf::Socket::Done) {
                        std::string receivedData(buffer, received);
                        logMessage("Received response from server: " + receivedData);

                        std::lock_guard<std::mutex> lock(dataMutex);
                        responses.push_back(utf8_to_wstring(receivedData));
                    } else {
                        logMessage("Failed to receive response from server");
                    }
                }
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        socket.disconnect();
    } catch (const std::exception& e) {
        logMessage(std::string("Exception in network thread: ") + e.what());
    }
}
