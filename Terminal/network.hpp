// network.hpp
#pragma once
#include <SFML/Network.hpp>
#include <mutex>
#include <atomic>
#include <vector>
#include <string>
#include <fstream>
#include <thread>

extern std::vector<std::wstring> responses;
extern std::mutex dataMutex;
extern std::mutex commandMutex;
extern std::atomic<bool> running;
extern std::string commandToSend;
extern std::ofstream logFile;

void logMessage(const std::string& message);
std::wstring utf8_to_wstring(const std::string& str);
void networkThreadFunc();
