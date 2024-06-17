#include <iostream>
#include <ncurses.h>
#include <unistd.h>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <vector>
#include <cmath>
#include <sstream>
#include <chrono>

using namespace std;

class NeuralNetwork {
private:
    vector<int> layers;
    vector<vector<double>> weights;
    vector<vector<double>> biases;
    double learningRate = 0.1;
    vector<vector<double>> layerOutputs;

public:
    NeuralNetwork(int inputLayer, int hiddenLayer, int outputLayer) {
        layers.push_back(inputLayer);
        layers.push_back(hiddenLayer);
        layers.push_back(hiddenLayer);
        layers.push_back(outputLayer);

        // Initialize weights
        for (int i = 0; i < layers.size() - 1; i++) {
            vector<double> layerWeights;
            vector<double> layerBiases;
            for (int j = 0; j < layers[i] * layers[i + 1]; j++) {
                layerWeights.push_back((rand() % 2000 - 1000) / 1000.0); // Random weights between -1 and 1
            }
            for (int j = 0; j < layers[i + 1]; j++) {
                layerBiases.push_back(0.0);
            }
            weights.push_back(layerWeights);
            biases.push_back(layerBiases);
        }
    }

    double Sigmoid(double x) {
        return 1 / (1 + exp(-x));
    }

    vector<double> FeedForward(vector<double> inputs) {
        layerOutputs.clear();
        layerOutputs.push_back(inputs);
        vector<double> layerOutput = inputs;
        for (int i = 0; i < layers.size() - 1; i++) {
            vector<double> newLayerOutput(layers[i + 1]);
            for (int j = 0; j < layers[i + 1]; j++) {
                double sum = 0.0;
                for (int k = 0; k < layers[i]; k++) {
                    sum += layerOutput[k] * weights[i][j * layers[i] + k];
                }
                newLayerOutput[j] = Sigmoid(sum + biases[i][j]);
            }
            layerOutput = newLayerOutput;
            layerOutputs.push_back(layerOutput);
        }
        return layerOutput;
    }

    void Train(vector<double> inputs, vector<double> targets) {
        // Feedforward
        vector<double> layerOutput = FeedForward(inputs);

        // Backpropagation
        vector<vector<double>> errors(layers.size() - 1);
        errors[errors.size() - 1] = vector<double>(targets.size());
        for (int i = 0; i < targets.size(); i++) {
            errors[errors.size() - 1][i] = targets[i] - layerOutput[i];
        }
        for (int i = errors.size() - 2; i >= 0; i--) {
            errors[i] = vector<double>(layers[i + 1]);
            for (int j = 0; j < layers[i + 1]; j++) {
                double error = 0.0;
                for (int k = 0; k < layers[i + 2]; k++) {
                    error += errors[i + 1][k] * weights[i + 1][k * layers[i + 1] + j];
                }
                errors[i][j] = error;
            }
        }

        // Update weights and biases
        for (int i = 0; i < layers.size() - 1; i++) {
            for (int j = 0; j < layers[i + 1]; j++) {
                for (int k = 0; k < layers[i]; k++) {
                    weights[i][j * layers[i] + k] += learningRate * errors[i][j] * layerOutputs[i][k] * (1 - layerOutputs[i][k]);
                }
                biases[i][j] += learningRate * errors[i][j];
            }
        }
    }

    int GetMaxOutputIndex() {
        int maxIndex = 0;
        double maxOutput = 0;
        for (int i = 0; i < layers.back(); i++) {
            if (layerOutputs.back()[i] > maxOutput) {
                maxOutput = layerOutputs.back()[i];
                maxIndex = i;
            }
        }
        return maxIndex;
    }
};

bool gameOver;
const int width = 20, height = 20;
int x, y, fruitX, fruitY, score;
int tailX[100], tailY[100];
int nTail;
enum eDirection { STOP = 0, LEFT, RIGHT, UP, DOWN };
eDirection dir;
chrono::time_point<chrono::steady_clock> lastGameTime;

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
    srand(time(NULL)); // Initialize random number generator
    fruitX = (rand() % (width - 2)) + 1;
    fruitY = (rand() % (height - 2)) + 1;
    score = 0;
    nTail = 0;
    lastGameTime = chrono::steady_clock::now();
}

void Draw() {
    clear();
    for (int i = 0; i < width + 2; i++)
        mvprintw(0, i, "#");
    for (int i = 0; i < height + 2; i++) {
        for (int j = 0; j < width + 2; j++) {
            if (i == 0 || i == 21)
                mvprintw(i, j, "#");
            else if (j == 0 || j == 21)
                mvprintw(i, j, "#");
            else if (i == y && j == x)
                mvprintw(i, j, "O");
            else if (i == fruitY && j == fruitX)
                mvprintw(i, j, "*");
            else {
                for (int k = 0; k < nTail; k++) {
                    if (tailX[k] == j && tailY[k] == i)
                        mvprintw(i, j, "o");
                }
            }
        }
    }
    mvprintw(23, 0, "Score:%d", score);
    refresh();
}

void Input() {
    keypad(stdscr, TRUE);
    halfdelay(1);
    int c = getch();
    switch (c) {
        case 'a':
            dir = LEFT;
            break;
        case 'd':
            dir = RIGHT;
            break;
        case 'w':
            dir = UP;
            break;
        case 's':
            dir = DOWN;
            break;
        case 'x':
            gameOver = true;
            break;
    }
}

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
    for (int i = 0; i < nTail; i++) {
        if (tailX[i] == x && tailY[i] == y)
            gameOver = true;
    }
    if (x == fruitX && y == fruitY) {
        score += 10;
        fruitX = (rand() % (width - 2)) + 1;
        fruitY = (rand() % (height - 2)) + 1;
        nTail++;
    }

    auto currentTime = chrono::steady_clock::now();
    if (gameOver || score == 100 || chrono::duration_cast<chrono::seconds>(currentTime - lastGameTime).count() >= 40) {
        vector<double> inputs = {(double)x, (double)y, (double)fruitX, (double)fruitY, 0.0, 0.0, 0.0, 0.0};
        if (dir == LEFT) inputs[4] = 1.0;
        else if (dir == RIGHT) inputs[5] = 1.0;
        else if (dir == UP) inputs[6] = 1.0;
        else if (dir == DOWN) inputs[7] = 1.0;
        vector<double> outputs = {0.0, 0.0, 0.0, 0.0};
        outputs[nn.GetMaxOutputIndex()] = 1.0;
        nn.Train(inputs, outputs);

        gameOver = false;
        x = width / 2;
        y = height / 2;
        fruitX = (rand() % (width - 2)) + 1;
        fruitY = (rand() % (height - 2)) + 1;
        score = 0;
        nTail = 0;
        lastGameTime = currentTime;
    }
}

int main() {
    NeuralNetwork nn(8, 16, 4);
    Setup();
    while (!gameOver) {
        Draw();
        Input();
        Logic(nn);
    }
    endwin();
    return 0;
}
