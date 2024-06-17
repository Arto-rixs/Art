// window_gui.cpp
#include "window_gui.hpp"

void setupGUI(tgui::Gui& gui, sf::RenderWindow& window, std::vector<sf::Text>& terminalMessages, sf::Font& font, std::mutex& dataMutex, std::vector<std::wstring>& responses, std::mutex& commandMutex, std::string& commandToSend) {
    auto panel = tgui::Panel::create({"100%", "100%"});
    gui.add(panel);

    auto topPanel = tgui::Panel::create({"100%", "15%"});
    topPanel->getRenderer()->setBackgroundColor(tgui::Color::Black);
    panel->add(topPanel);

    auto sidePanel = tgui::Panel::create({"20%", "85%"});
    sidePanel->setPosition({"0%", "15%"});
    sidePanel->getRenderer()->setBackgroundColor(tgui::Color::Black);
    panel->add(sidePanel);

    auto mainArea = tgui::Panel::create({"80%", "85%"});
    mainArea->setPosition({"20%", "15%"});
    mainArea->getRenderer()->setBackgroundColor(tgui::Color::White);
    panel->add(mainArea);

    auto terminalPanel = tgui::Panel::create({"80%", "30%"});
    terminalPanel->setPosition({"20%", "70%"});
    terminalPanel->getRenderer()->setBackgroundColor(tgui::Color::Black);
    panel->add(terminalPanel);

    auto onAllTradeButton = tgui::Button::create("OnAllTrade");
    onAllTradeButton->setPosition({"5%", "5%"});
    onAllTradeButton->setSize({"90%", "10%"});
    sidePanel->add(onAllTradeButton);

    auto sendCommandButton = tgui::Button::create("SendCommand");
    sendCommandButton->setPosition({"5%", "85%"});
    sendCommandButton->setSize({"90%", "10%"});
    sidePanel->add(sendCommandButton);

    auto commandInput = tgui::EditBox::create();
    commandInput->setPosition({"5%", "70%"});
    commandInput->setSize({"90%", "10%"});
    sidePanel->add(commandInput);

    onAllTradeButton->onPress([&]() {
        {
            std::lock_guard<std::mutex> lock(commandMutex);
            commandToSend = "OnAllTrade";
        }
    });

    sendCommandButton->onPress([&]() {
        std::string command = commandInput->getText().toStdString();
        {
            std::lock_guard<std::mutex> lock(commandMutex);
            commandToSend = command;
        }
        commandInput->setText("");
    });
}

void drawGraphics(sf::RenderWindow& window, tgui::Gui& gui, const std::vector<sf::Text>& terminalMessages, const std::vector<std::pair<std::string, std::pair<float, float>>>& sampleData) {
    window.clear();

    // Draw terminal messages
    for (const auto& message : terminalMessages) {
        window.draw(message);
    }

    // Drawing candlestick chart
    drawCandlestickChart(window, sampleData, sf::Vector2f(80, 70), sf::Vector2f(800, 600));

    gui.draw();
    window.display();
}
