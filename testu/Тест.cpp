#include <ncurses.h>
#include <unistd.h>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <cmath>
#include <fstream>
#include <chrono>

// Константы для размера поля и длины хвоста
const int width = 20, height = 20;
const int tailMaxLength = 100;

// Переменные для позиции фрукта и змейки
int fruitX, fruitY, score;
int x, y;
int nTail;
std::vector<int> tailX(tailMaxLength);
std::vector<int> tailY(tailMaxLength);
bool gameOver;
enum eDirection { STOP = 0, LEFT, RIGHT, UP, DOWN };
eDirection dir;

// Класс нейронной сети
class NeuralNetwork {
public:
    std::vector<double> weights;
    std::vector<double> layers[4];
    double sigmoid(double x) {
        return 1 / (1 + exp(-x));
    }
public:
    NeuralNetwork(int inputSize, int hiddenSize1, int hiddenSize2, int outputSize) {
        int totalWeights = (inputSize + 1) * hiddenSize1 + (hiddenSize1 + 1) * hiddenSize2 + (hiddenSize2 + 1) * outputSize;
        weights.resize(totalWeights);
        for (int i = 0; i < totalWeights; i++) {
            weights[i] = ((double)rand() / RAND_MAX - 0.5) * 2;
        }
    }

    // Функция для прямого распространения
    std::vector<double> feedForward(const std::vector<double> &input) {
        layers[0] = input;
        int weightIndex = 0;
        for (int i = 1; i < 4; i++) {
            layers[i].clear();
            int layerSize = (i == 1) ? 12 : (i == 2) ? 6 : 4;
            for (int j = 0; j < layerSize; j++) {
                double sum = weights[weightIndex++];
                for (int k = 0; k < layers[i - 1].size(); k++) {
                    sum += layers[i - 1][k] * weights[weightIndex++];
                }
                layers[i].push_back(sigmoid(sum));
            }
        }
        return layers[3];
    }

    // Функция для мутации весов
    void mutate(double mutationRate) {
        for (double &weight : weights) {
            if ((double)rand() / RAND_MAX < mutationRate) {
                weight += ((double)rand() / RAND_MAX - 0.5) * 2;
            }
        }
    }

    const std::vector<double>& getWeights() const {
        return weights;
    }

    void setWeights(const std::vector<double>& newWeights) {
        weights = newWeights;
    }
};

// Инициализация игры
void Setup() {
    initscr();
    clear();
    noecho();
    cbreak();
    curs_set(0);
    gameOver = false;
    dir = STOP;
    x = width / 2;
    y = height / 2;
    srand(time(NULL));
    do {
        fruitX = rand() % width;
        fruitY = rand() % height;
    } while (fruitX == 0 || fruitX == width - 1 || fruitY == 0 || fruitY == height - 1);
    score = 0;
    nTail = 0;
}

// Функция для отрисовки игрового поля
void Draw(int timeLeft, int epochCount) {
    clear();
    for (int i = 0; i < width + 2; i++)
        mvprintw(0, i, "#");
    for (int i = 0; i < height + 2; i++) {
        for (int j = 0; j < width + 2; j++) {
            if (i == 0 || i == height + 1)
                mvprintw(i, j, "#");
            else if (j == 0 || j == width + 1)
                mvprintw(i, j, "#");
            else if (i == y && j == x)
                mvprintw(i, j, "O");
            else if (i == fruitY && j == fruitX)
                mvprintw(i, j, "F");
            else {
                bool print = false;
                for (int k = 0; k < nTail; k++) {
                    if (tailX[k] == j && tailY[k] == i) {
                        mvprintw(i, j, "o");
                        print = true;
                    }
                }
                if (!print)
                    mvprintw(i, j, " ");
            }
        }
    }
    mvprintw(height + 3, 0, "Score: %d", score);
    mvprintw(height + 4, 0, "Time left: %ds", timeLeft);
    mvprintw(height + 5, 0, "Epoch: %d", epochCount);
    refresh();
}

// Функция для обработки ввода с клавиатуры
void Input() {
    keypad(stdscr, TRUE);
    halfdelay(1);
    int c = getch();
    switch (c) {
        case KEY_LEFT:
            if (dir != RIGHT)
                dir = LEFT;
            break;
        case KEY_RIGHT:
            if (dir != LEFT)
                dir = RIGHT;
            break;
        case KEY_UP:
            if (dir != DOWN)
                dir = UP;
            break;
        case KEY_DOWN:
            if (dir != UP)
                dir = DOWN;
            break;
        case 'x':
            gameOver = true;
            break;
        case ' ':
            gameOver = true;
            break;
    }
}

// Функция для обработки логики игры и нейронной сети
void Logic(NeuralNetwork &nn) {
    int prevX = tailX[0];
    int prevY = tailY[0];
    int prev2X, prev2Y;
    tailX[0] = x;
    tailY[0] = y;
    for (int i = 1; i < nTail; i++) {
        prev2X = tailX[i];
        prev2Y = tailY[i];
        tailX[i] = prevX;
        tailY[i] = prevY;
        prevX = prev2X;
        prevY = prev2Y;
    }

    switch (dir) {
        case LEFT:
            x--;
            break;
        case RIGHT:
            x++;
            break;
        case UP:
            y--;
            break;
        case DOWN:
            y++;
            break;
        default:
            break;
    }

    if (x >= width || x < 0 || y >= height || y < 0)
        gameOver = true;

    for (int i = 0; i < nTail; i++)
        if (tailX[i] == x && tailY[i] == y)
            gameOver = true;

    if (x == fruitX && y == fruitY) {
        score += 10;
        fruitX = rand() % width;
        fruitY = rand() % height;
        nTail++;
    }

    // Обработка входных данных для нейронной сети
    std::vector<double> input = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    if (fruitX < x) input[0] = 1; // фрукт слева
    if (fruitX > x) input[1] = 1; // фрукт справа
    if (fruitY < y) input[2] = 1; // фрукт выше
    if (fruitY > y) input[3] = 1; // фрукт ниже
    input[4] = 1.0 / (std::abs(fruitX - x) + std::abs(fruitY - y)); // расстояние до фрукта

    input[5] = (dir == LEFT || x == 0 || (std::find(tailX.begin(), tailX.begin() + nTail, x - 1) != tailX.begin() + nTail)) ? 1 : 0;
    input[6] = (dir == RIGHT || x == width - 1 || (std::find(tailX.begin(), tailX.begin() + nTail, x + 1) != tailX.begin() + nTail)) ? 1 : 0;
    input[7] = (dir == UP || y == 0 || (std::find(tailY.begin(), tailY.begin() + nTail, y - 1) != tailY.begin() + nTail)) ? 1 : 0;
    input[8] = (dir == DOWN || y == height - 1 || (std::find(tailY.begin(), tailY.begin() + nTail, y + 1) != tailY.begin() + nTail)) ? 1 : 0;
    input[9] = (dir == UP) ? 1 : 0;

    std::vector<double> output = nn.feedForward(input);

    double maxOutput = output[0];
    int maxIndex = 0;
    for (int i = 1; i < output.size(); i++) {
        if (output[i] > maxOutput) {
            maxOutput = output[i];
            maxIndex = i;
        }
    }

    switch (maxIndex) {
        case 0:
            if (dir != DOWN)
                dir = UP;
            break;
        case 1:
            if (dir != RIGHT)
                dir = LEFT;
            break;
        case 2:
            if (dir != LEFT)
                dir = RIGHT;
            break;
        case 3:
            if (dir != UP)
                dir = DOWN;
            break;
        default:
            break;
    }
}

// Функция для сохранения весов нейронной сети
void SaveWeights(const NeuralNetwork &nn, const std::string &filename) {
    std::ofstream outFile(filename);
    if (outFile.is_open()) {
        const std::vector<double>& weights = nn.getWeights();
        for (double weight : weights) {
            outFile << weight << " ";
        }
        outFile.close();
    } else {
        mvprintw(height + 6, 0, "Error: Unable to open file for writing.");
        refresh();
    }
}

// Функция для загрузки весов нейронной сети
void LoadWeights(NeuralNetwork &nn, const std::string &filename) {
    std::ifstream inFile(filename);
    if (inFile.is_open()) {
        std::vector<double> weights;
        double weight;
        while (inFile >> weight) {
            weights.push_back(weight);
        }
        inFile.close();
        nn.setWeights(weights);
    } else {
        mvprintw(height + 6, 0, "Error: Unable to open file for reading.");
        refresh();
    }
}

int main() {
    NeuralNetwork nn(10, 12, 6, 4);
    double mutationRate = 0.2;
    int epochCount = 0;

    do {
        epochCount++;
        Setup();
        auto startTime = std::chrono::steady_clock::now();
        int secondsRemaining = 1 * 60;

        while (!gameOver && secondsRemaining > 0) {
            auto endTime = std::chrono::steady_clock::now();
            std::chrono::duration<double> elapsedSeconds = endTime - startTime;
            secondsRemaining = 1 * 60 - static_cast<int>(elapsedSeconds.count());

            Draw(secondsRemaining, epochCount);
            Input();
            Logic(nn);
            usleep(100000);
        }

        std::vector<double> newWeights = nn.getWeights();

        if (gameOver) {
            for (double &weight : newWeights) {
                weight *= 0.4;
            }
        } else {
            for (double &weight : newWeights) {
                weight *= 1.2;
            }
        }

        nn.setWeights(newWeights);
        nn.mutate(mutationRate);
        SaveWeights(nn, "weights.txt");
        LoadWeights(nn, "weights.txt");

    } while (true);

    endwin();
    return 0;
}
