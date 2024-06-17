#include <TGUI/TGUI.hpp>

int main() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "TGUI ListView in Window Example");

    tgui::GuiSFML gui(window);

    // Создаем дочернее окно
    tgui::ChildWindow::Ptr windowPtr = tgui::ChildWindow::create();
    windowPtr->setPosition(220, 70);
    windowPtr->setSize(500, 300);
    windowPtr->setTitle("Deals List");
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
    toggleButton->setPosition(10, 10);
    toggleButton->setText("Toggle Deals List");
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

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            gui.handleEvent(event);

            if (event.type == sf::Event::Closed) {
                window.close();
            }
        }

        window.clear();
        gui.draw();
        window.display();
    }

    return 0;
}
