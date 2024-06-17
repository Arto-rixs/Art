#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>
#include <stdexcept> // Для std::runtime_error

class NeuralNetwork {
private:
    int numInputs = 4;
    int numHidden = 10;
    int numOutputs = 3;
    std::vector<std::vector<double>> weightsInputHidden;
    std::vector<std::vector<double>> weightsHiddenOutput;
    std::vector<double> inputLayer;
    std::vector<double> hiddenLayer;
    std::vector<double> outputLayer;
    std::vector<double> target; // Размер целевого вектора установлен динамически

public:
    NeuralNetwork() {
        weightsInputHidden.resize(numInputs);
        for (int i = 0; i < numInputs; ++i) {
            weightsInputHidden[i].resize(numHidden);
            for (int j = 0; j < numHidden; ++j) {
                weightsInputHidden[i][j] = ((double)rand() / RAND_MAX) - 0.5;
            }
        }

        weightsHiddenOutput.resize(numHidden);
        for (int i = 0; i < numHidden; ++i) {
            weightsHiddenOutput[i].resize(numOutputs);
            for (int j = 0; j < numOutputs; ++j) {
                weightsHiddenOutput[i][j] = ((double)rand() / RAND_MAX) - 0.5;
            }
        }

        inputLayer = std::vector<double>(numInputs, 0.0);
        hiddenLayer = std::vector<double>(numHidden, 0.0);
        outputLayer = std::vector<double>(numOutputs, 0.0);
        target = std::vector<double>(numOutputs, 0.0); // Инициализация размера целевого вектора
    }

    double activationFunction(double x) {
        return 1.0 / (1.0 + exp(-x));
    }

    void train(const std::vector<double>& input) {
        // Определение целевых значений
        double minPrice = input[2]; // Low
        double maxPrice = input[1]; // High
        target[0] = maxPrice - minPrice; // Диапазон цен

        // Направление движения цены: если текущая цена закрытия больше предыдущей, то движение вверх, иначе вниз
        double previousClose = input[3]; // Close
        double currentClose = input[0]; // Open
        target[1] = (currentClose > previousClose) ? 1.0 : 0.0; // 1.0 - движение вверх, 0.0 - движение вниз

        // Опережение: можно попытаться предсказать будущие изменения цены, например, на следующий день
        // Для примера, можно сравнивать текущую цену закрытия с ценой закрытия на следующий день
        // Это просто пример, в реальности нужно использовать данные из будущего, которые мы не можем знать
        double nextDayClose = input[0]; // Close (на следующий день)
        double futureMovement = (nextDayClose > currentClose) ? 1.0 : 0.0; // 1.0 - движение вверх, 0.0 - движение вниз
        // Предполагаем, что сеть может использовать текущую информацию для предсказания будущего, хотя это нереально
        target[2] = futureMovement;

        // Обучение нейронной сети
        for (int j = 0; j < numHidden; ++j) {
            double sum = 0.0;
            for (int i = 0; i < numInputs; ++i) {
                sum += input[i] * weightsInputHidden[i][j];
            }
            hiddenLayer[j] = activationFunction(sum);
        }

        for (int k = 0; k < numOutputs; ++k) {
            double sum = 0.0;
            for (int j = 0; j < numHidden; ++j) {
                sum += hiddenLayer[j] * weightsHiddenOutput[j][k];
            }
            outputLayer[k] = activationFunction(sum);
        }

        std::vector<double> outputErrors(numOutputs, 0.0);
        for (int k = 0; k < numOutputs; ++k) {
            outputErrors[k] = (target[k] - outputLayer[k]) * outputLayer[k] * (1.0 - outputLayer[k]);
        }

        std::vector<double> hiddenErrors(numHidden, 0.0);
        for (int j = 0; j < numHidden; ++j) {
            double error = 0.0;
            for (int k = 0; k < numOutputs; ++k) {
                error += outputErrors[k] * weightsHiddenOutput[j][k];
            }
            hiddenErrors[j] = error * hiddenLayer[j] * (1.0 - hiddenLayer[j]);
        }

        for (int j = 0; j < numHidden; ++j) {
            for (int k = 0; k < numOutputs; ++k) {
                weightsHiddenOutput[j][k] += 0.1 * outputErrors[k] * hiddenLayer[j];
            }
        }

        for (int i = 0; i < numInputs; ++i) {
            for (int j = 0; j < numHidden; ++j) {
                weightsInputHidden[i][j] += 0.1 * hiddenErrors[j] * input[i];
            }
        }
    }

    std::vector<double> predict(const std::vector<double>& input) {
        for (int i = 0; i < numInputs; ++i) {
            inputLayer[i] = input[i];
        }

        for (int j = 0; j < numHidden; ++j) {
            double sum = 0.0;
            for (int i = 0; i < numInputs; ++i) {
                sum += input[i] * weightsInputHidden[i][j];
            }
            hiddenLayer[j] = activationFunction(sum);
        }

        for (int k = 0; k < numOutputs; ++k) {
            double sum = 0.0;
            for (int j = 0; j < numHidden; ++j) {
                sum += hiddenLayer[j] * weightsHiddenOutput[j][k];
            }
            outputLayer[k] = activationFunction(sum);
        }

        return outputLayer;
    }
};

std::vector<std::vector<double>> readCSV(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Ошибка открытия файла!");
    }

    std::vector<std::vector<double>> data;

    // Пропустить первую строку (заголовки)
    std::string header;
    std::getline(file, header);

    // Чтение числовых данных
    std::string line;
    while (std::getline(file, line)) {
        std::vector<double> row;
        std::istringstream ss(line);
        double value;
        while (ss >> value) {
            row.push_back(value);
        }
        data.push_back(row);
    }

    file.close();
    return data;
}

int main() {
    NeuralNetwork nn;

    try {
        std::vector<std::vector<double>> inputData = readCSV("brent.csv");

        if (inputData.empty()) {
            std::cerr << "Не удалось прочитать данные из файла!" << std::endl;
            return 1;
        }

        std::cout << "Считанные данные:" << std::endl;
        for (const auto& row : inputData) {
            for (double value : row) {
                std::cout << value << " ";
            }
            std::cout << std::endl;
        }

        std::cout << "Начало обучения..." << std::endl;

        // Пример обучения нейронной сети
        for (int epoch = 1; epoch <= 10000; ++epoch) {
            std::cout << "Эпоха " << epoch << std::endl;
            for (const auto& input : inputData) {
                nn.train(input);
            }
        }

        std::cout << "Обучение завершено." << std::endl;

        // Пример предсказания на тех же входных данных
        std::cout << "Предсказания после обучения:" << std::endl;

        // Выполним прогноз только на первый день данных
        std::vector<double> prediction = nn.predict(inputData.back());

        // Выводим результаты
        std::cout << "Предсказанные значения:" << std::endl;
        for (double value : prediction) {
            std::cout << value << " ";
        }
        std::cout << std::endl;

        // Определим точность предсказания
        double error = 0.0;
        double nonZeroCount = 0; // Счетчик ненулевых значений для избежания деления на ноль
        for (int i = 0; i < prediction.size(); ++i) {
            if (inputData.back()[i] != 0) { // Проверяем, что значение не нулевое
                error += fabs(prediction[i] - inputData.back()[i]) / inputData.back()[i];
                nonZeroCount++;
            }
        }
        double accuracy;
        if (nonZeroCount == 0) {
            accuracy = 100.0;
        } else {
            accuracy = (1.0 - error / nonZeroCount) * 100.0;
        }
        std::cout << "Точность предсказания: " << accuracy << "%" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}
