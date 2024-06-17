#include <iostream>
#include <fstream>
#include <string>
#include <json/json.h>
#include "webdriverxx.h"
#include "webdriverxx/browsers/chrome.h"

using namespace webdriverxx;

int main() {
    // Настройка WebDriver и запуск браузера
    WebDriver driver = Start(Chrome());
    driver.Navigate("URL_ВАШЕГО_ГРАФИКА");

    // Ожидание загрузки страницы (увеличьте время, если страница загружается медленно)
    std::this_thread::sleep_for(std::chrono::seconds(5));

    // Получение элементов свечей
    auto candle_elements = driver.FindElements(ByCss("class-name-of-candles")); // Замените на правильный селектор

    // Функция для извлечения данных о свече
    auto get_candle_data = [&driver](const WebElement& candle_element) {
        // Наведение указателя мыши на свечу
        driver.MoveTo(candle_element).Perform();
        std::this_thread::sleep_for(std::chrono::seconds(1));  // Ожидание обновления данных

        // Получение данных о свече
        WebElement date_element = driver.FindElement(ByCss("class-name-of-date"));  // Замените на правильный селектор
        WebElement open_element = driver.FindElement(ByCss("class-name-of-open"));  // Замените на правильный селектор
        WebElement high_element = driver.FindElement(ByCss("class-name-of-high"));  // Замените на правильный селектор
        WebElement low_element = driver.FindElement(ByCss("class-name-of-low"));  // Замените на правильный селектор
        WebElement close_element = driver.FindElement(ByCss("class-name-of-close"));  // Замените на правильный селектор

        Json::Value candle_data;
        candle_data["date"] = date_element.GetText();
        candle_data["open"] = std::stof(open_element.GetText());
        candle_data["high"] = std::stof(high_element.GetText());
        candle_data["low"] = std::stof(low_element.GetText());
        candle_data["close"] = std::stof(close_element.GetText());

        return candle_data;
    };

    // Сбор данных
    Json::Value candles_data(Json::arrayValue);
    for (const auto& candle_element : candle_elements) {
        candles_data.append(get_candle_data(candle_element));
    }

    // Запись данных в файл
    std::ofstream file("candles_data.json");
    file << candles_data.toStyledString();
    file.close();

    driver.Quit();
    return 0;
}
