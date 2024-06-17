#include <iostream>
#include <fstream>
#include <eigen3/Eigen/Dense> 
#include <vector>
#include <cmath>
#include <random>
#include <algorithm>

using namespace Eigen;
using namespace std;

class NeuralNetwork {
private:
    int размер_входного_слоя_нейронной_сети; 
    int размер_скрытого_слоя_нейронной_сети;
    int размер_выходного_слоя_нейронной_сети;

    MatrixXd Матрица_весов_между_входным_и_скрытым_слоем;
    MatrixXd Вектор_смещения_для_скрытого_слоя;
    MatrixXd Матрица_весов_между_скрытым_и_выходным_слоем;
    MatrixXd Вектор_смещения_для_выходного_слоя;

    double коэффициент_скорости_обучения;

public:
    NeuralNetwork(int размер_входного_слоя_нейронной_сети, int размер_скрытого_слоя_нейронной_сети, int размер_выходного_слоя_нейронной_сети, double learning_rate)
        : размер_входного_слоя_нейронной_сети(размер_входного_слоя_нейронной_сети), размер_скрытого_слоя_нейронной_сети(размер_скрытого_слоя_нейронной_сети), размер_выходного_слоя_нейронной_сети(размер_выходного_слоя_нейронной_сети), коэффициент_скорости_обучения(learning_rate) {
        // Инициализация весов
        random_device rd;
        mt19937 gen(rd());
        normal_distribution<> dist(0.0, 0.01);

        // Инициализация весов случайными значениями
        Матрица_весов_между_входным_и_скрытым_слоем = MatrixXd::Zero(размер_входного_слоя_нейронной_сети, размер_скрытого_слоя_нейронной_сети).unaryExpr([&](double dummy) { return dist(gen); });
        Вектор_смещения_для_скрытого_слоя = MatrixXd::Zero(1, размер_скрытого_слоя_нейронной_сети).unaryExpr([&](double dummy) { return dist(gen); });
        Матрица_весов_между_скрытым_и_выходным_слоем = MatrixXd::Zero(размер_скрытого_слоя_нейронной_сети, размер_выходного_слоя_нейронной_сети).unaryExpr([&](double dummy) { return dist(gen); });
        Вектор_смещения_для_выходного_слоя = MatrixXd::Zero(1, размер_выходного_слоя_нейронной_сети).unaryExpr([&](double dummy) { return dist(gen); });
    }

    // Функция активации - ReLU
    MatrixXd relu(const MatrixXd& X) {
        return X.array().max(0.0);
    }

    // Производная функции активации ReLU
    MatrixXd relu_derivative(const MatrixXd& X) {
        return (X.array() > 0.0).cast<double>();
    }

    // Обучение нейронной сети методом обратного распространения ошибки
    void train(const MatrixXd& X, const MatrixXd& y, int epochs) {
        cout << "Training..." << endl;
        for (int i = 0; i < epochs; ++i) {
            // Прямое распространение
            MatrixXd z1 = X * Матрица_весов_между_входным_и_скрытым_слоем + Вектор_смещения_для_скрытого_слоя.replicate(X.rows(), 1);
            MatrixXd a1 = relu(z1);
            MatrixXd z2 = a1 * Матрица_весов_между_скрытым_и_выходным_слоем + Вектор_смещения_для_выходного_слоя.replicate(a1.rows(), 1);
            MatrixXd exp_scores = z2.array().exp();
            MatrixXd probs = exp_scores.array() / exp_scores.array().rowwise().sum().replicate(1, размер_выходного_слоя_нейронной_сети);

            // Вычисление градиентов
            MatrixXd delta3 = probs - y;
            MatrixXd dW2 = a1.transpose() * delta3;
            MatrixXd db2 = delta3.colwise().sum();
            MatrixXd delta2 = delta3 * Матрица_весов_между_скрытым_и_выходным_слоем.transpose();
            delta2 = delta2.array() * relu_derivative(z1).array();
            MatrixXd dW1 = X.transpose() * delta2;
            MatrixXd db1 = delta2.colwise().sum();

            // Обновление весов
            Матрица_весов_между_входным_и_скрытым_слоем -= коэффициент_скорости_обучения * dW1;
            Вектор_смещения_для_скрытого_слоя -= коэффициент_скорости_обучения * db1;
            Матрица_весов_между_скрытым_и_выходным_слоем -= коэффициент_скорости_обучения * dW2;
            Вектор_смещения_для_выходного_слоя -= коэффициент_скорости_обучения * db2;
        }
        cout << "Training completed." << endl;
    }

    // Прогнозирование на основе обученной модели
    MatrixXd predict(const MatrixXd& X) {
        cout << "Predicting..." << endl;
        MatrixXd z1 = X * Матрица_весов_между_входным_и_скрытым_слоем + Вектор_смещения_для_скрытого_слоя.replicate(X.rows(), 1);
        MatrixXd a1 = relu(z1);
        MatrixXd z2 = a1 * Матрица_весов_между_скрытым_и_выходным_слоем + Вектор_смещения_для_выходного_слоя.replicate(a1.rows(), 1);
        MatrixXd exp_scores = z2.array().exp();
        MatrixXd predicted = exp_scores.array() / exp_scores.array().rowwise().sum().replicate(1, размер_выходного_слоя_нейронной_сети);
        cout << "Prediction completed." << endl;
        return predicted;
    }
};

int main() {
    // Загрузка данных
    cout << "Loading data..." << endl;
    MatrixXd data(70, 4); // Создаем матрицу для хранения данных (13 строк, 4 столбца)
    ifstream file("brent.csv"); // Открываем файл
    if (!file.is_open()) {
        cerr << "Failed to open the file." << endl;
        return 1;
    }

    string line; // Переменная для хранения строки из файла
    int row = 0; // Номер текущей строки в матрице
    bool first_line = true; // Флаг для определения первой строки
    while (getline(file, line)) { // Читаем файл построчно
        if (first_line) { // Проверяем, является ли текущая строка первой
            first_line = false; // Устанавливаем флаг в false, чтобы пропустить следующую строку
            continue; // Пропускаем текущую итерацию цикла
        }
        stringstream ss(line); // Создаем поток для работы со строкой
        string item; // Переменная для хранения элемента (числа)
        int col = 0; // Номер текущего столбца в матрице
        while (getline(ss, item, ',')) { // Разделяем строку по запятой
            data(row, col) = stod(item); // Преобразуем строку в число и сохраняем его в матрицу
            ++col; // Переходим к следующему столбцу
        }
        ++row; // Переходим к следующей строке
    }
    cout << "Data loaded." << endl;

    // Параметры нейронной сети
    int размер_входного_слоя_нейронной_сети = 4;
    int размер_скрытого_слоя_нейронной_сети = 10;
    int размер_выходного_слоя_нейронной_сети = 4;
    double коэффициент_скорости_обучения = 0.01;

    // Создание и обучение нейронной сети
    NeuralNetwork nn(размер_входного_слоя_нейронной_сети, размер_скрытого_слоя_нейронной_сети, размер_выходного_слоя_нейронной_сети, коэффициент_скорости_обучения);
    nn.train(data, data, 1000);

    // Прогнозирование
    MatrixXd predicted = nn.predict(data);
    cout << "Predicted:\n" << predicted << endl;

    return 0;
}
