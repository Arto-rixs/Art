#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cmath>
#include <eigen3/Eigen/Dense> // Для работы с матрицами и векторами
#include <eigen3/unsupported/Eigen/CXX11/Tensor>  // Для работы с тензорами

using namespace Eigen;
using namespace std;

MatrixXd W1, W2, b1, b2; // Объявление переменных в глобальной области видимости

// Функция для загрузки данных из файла CSV
MatrixXd load_csv(const string &file_path) {
    ifstream file(file_path);
    string line;
    vector<vector<double>> values;
    bool skip_first_line = true;
    while (getline(file, line)) {
        if (skip_first_line) {
            skip_first_line = false;
            continue; // Пропускаем первую строку с названиями столбцов
        }
        stringstream ss(line);
        vector<double> row;
        string cell;
        while (getline(ss, cell, ',')) {
            try {
                row.push_back(stod(cell));
            } catch (const std::invalid_argument& ia) {
                cerr << "Invalid argument: " << ia.what() << " for string: " << cell << endl;
                throw; // Пробрасываем исключение дальше
            }
        }
        values.push_back(row);
    }
    MatrixXd data(values.size(), values[0].size());
    for (size_t i = 0; i < values.size(); ++i) {
        for (size_t j = 0; j < values[i].size(); ++j) {
            data(i, j) = values[i][j];
        }
    }
    return data;
}

// Функция для нормализации данных
MatrixXd normalize_data(const MatrixXd &data) {
    MatrixXd normalized_data = data;
    for (int i = 0; i < data.cols(); ++i) {
        double min_val = data.col(i).minCoeff();
        double max_val = data.col(i).maxCoeff();
        normalized_data.col(i) = (data.col(i).array() - min_val) / (max_val - min_val);
    }
    return normalized_data;
}

// Функция для создания последовательностей данных
tuple<MatrixXd, MatrixXd> create_sequences(const MatrixXd &data, int seq_length) {
    MatrixXd X(data.rows() - seq_length, seq_length * data.cols());
    MatrixXd y(data.rows() - seq_length, data.cols());
    for (int i = 0; i < data.rows() - seq_length; ++i) {
        for (int j = 0; j < seq_length; ++j) {
            X.row(i).segment(j * data.cols(), data.cols()) = data.block(i + j, 0, 1, data.cols());
        }
        y.row(i) = data.row(i + seq_length);
    }
    return make_tuple(X, y);
}

// Функция для обучения нейронной сети
void train_model(const MatrixXd &X, const MatrixXd &y) {
    int input_size = X.cols();
    int output_size = y.cols();
    int hidden_size = 100;
    int epochs = 100;
    double learning_rate = 0.001;

    // Инициализация весов
    W1 = MatrixXd::Random(input_size, hidden_size);
    W2 = MatrixXd::Random(hidden_size, output_size);
    b1 = MatrixXd::Zero(1, hidden_size);
    b2 = MatrixXd::Zero(1, output_size);

    // Обучение нейронной сети
    for (int epoch = 0; epoch < epochs; ++epoch) {
        // Прямой проход
        MatrixXd Z1 = X * W1 + MatrixXd::Ones(X.rows(), 1) * b1;
        MatrixXd A1 = Z1.array().exp() / (Z1.array().exp().rowwise().sum());
        MatrixXd Z2 = A1 * W2 + MatrixXd::Ones(A1.rows(), 1) * b2;
        MatrixXd A2 = Z2.array().exp().rowwise() / Z2.array().exp().rowwise().sum(); // Исправление здесь

        // Обратный проход
        MatrixXd dZ2 = A2 - y;
        MatrixXd dW2 = A1.transpose() * dZ2;
        MatrixXd db2 = dZ2.colwise().sum();
        MatrixXd dA1 = dZ2 * W2.transpose();
        MatrixXd dZ1 = dA1.array() * (A1.array() * (1 - A1.array())); // Производная сигмоиды
        MatrixXd dW1 = X.transpose() * dZ1;
        MatrixXd db1 = dZ1.colwise().sum();

        // Градиентный спуск
        W1 -= learning_rate * dW1;
        b1 -= learning_rate * db1;
        W2 -= learning_rate * dW2;
        b2 -= learning_rate * db2;
    }
}

int main() {
    // Загрузка и предобработка данных
    MatrixXd data = load_csv("brent.csv");
    MatrixXd normalized_data = normalize_data(data);
    int seq_length = 5; // Определение длины последовательности
    auto [X, y] = create_sequences(normalized_data, seq_length);

    // Обучение модели
    train_model(X, y);

    return 0;
}
