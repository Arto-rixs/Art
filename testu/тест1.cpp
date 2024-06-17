#include <ncurses.h>
#include <unistd.h>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <cmath>
#include <fstream>
#include <chrono>
#include <algorithm>

const int width = 20, height = 20;
const int tailMaxLength = 100;
int fruitX, fruitY, score;
int x, y;
int nTail;
std::vector<int> tailX(tailMaxLength);
std::vector<int> tailY(tailMaxLength);
bool gameOver;
enum eDirection { STOP = 0, LEFT, RIGHT, UP, DOWN };
eDirection dir;

class NeuralNetwork {
public:
    std::vector<double> weights;
    std::vector<std::vector<double>> layers;
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
        layers.resize(4);
    }

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

    void mutate(double mutationRate) {
        for (int i = 0; i < weights.size(); i++) {
            if ((double)rand() / RAND_MAX < mutationRate) {
                weights[i] += ((double)rand() / RAND_MAX - 0.5) * 2;
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

    std::vector<double> input(10, 0);

    if (x < fruitX) input[0] = 1;
    if (x > fruitX) input[1] = 1;
    if (y < fruitY) input[2] = 1;
    if (y > fruitY) input[3] = 1;

    double distanceX = fruitX - x;
    double distanceY = fruitY - y;
    input[4] = distanceX / width;
    input[5] = distanceY / height;

    input[6] = (dir == LEFT || x == 0 || (std::find(tailX.begin(), tailX.begin() + nTail, x - 1) != tailX.begin() + nTail)) ? 1 : 0;
    input[7] = (dir == RIGHT || x == width - 1 || (std::find(tailX.begin(), tailX.begin() + nTail, x + 1) != tailX.begin() + nTail)) ? 1 : 0;
    input[8] = (dir == UP || y == 0 || (std::find(tailY.begin(), tailY.begin() + nTail, y - 1) != tailY.begin() + nTail)) ? 1 : 0;
    input[9] = (dir == DOWN || y == height - 1 || (std::find(tailY.begin(), tailY.begin() + nTail, y + 1) != tailY.begin() + nTail)) ? 1 : 0;

    std::vector<double> output = nn.feedForward(input);

    if (output.empty()) {
        gameOver = true;
        return;
    }

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
            dir = LEFT;
            break;
        case 1:
            dir = RIGHT;
            break;
        case 2:
            dir = UP;
            break;
        case 3:
            dir = DOWN;
            break;
    }
}

void saveWeights(const NeuralNetwork& nn, const std::string& filename) {
    std::ofstream outFile(filename);
    for (double weight : nn.getWeights()) {
        outFile << weight << " ";
    }
}

void loadWeights(NeuralNetwork& nn, const std::string& filename) {
    std::ifstream inFile(filename);
    std::vector<double> weights;
    double weight;
    while (inFile >> weight) {
        weights.push_back(weight);
    }
    nn.setWeights(weights);
}

int main() {
    srand(time(0));
    NeuralNetwork nn(10, 12, 6, 4);
    std::string weightFile = "snake_weights.txt";
    loadWeights(nn, weightFile);

    int epochs = 10000;
    int generationSize = 50;
    double mutationRate = 0.05;
    double survivalRate = 0.2;

    std::vector<NeuralNetwork> population(generationSize, nn);
    std::vector<int> scores(generationSize);

    for (int epoch = 0; epoch < epochs; epoch++) {
        for (int i = 0; i < generationSize; i++) {
            Setup();
            auto startTime = std::chrono::high_resolution_clock::now();
            int timeLimit = 300; 
            while (!gameOver) {
                auto currentTime = std::chrono::high_resolution_clock::now();
                int elapsedTime = std::chrono::duration_cast<std::chrono::seconds>(currentTime - startTime).count();
                int timeLeft = timeLimit - elapsedTime;
                if (timeLeft <= 0) {
                    gameOver = true;
                }
                Draw(timeLeft, epoch);
                Logic(population[i]);
                usleep(50000);
            }
            scores[i] = score;
        }

        std::vector<std::pair<int, int>> rankedScores;
        for (int i = 0; i < generationSize; i++) {
            rankedScores.push_back(std::make_pair(scores[i], i));
        }
        std::sort(rankedScores.rbegin(), rankedScores.rend());

        int survivors = generationSize * survivalRate;
        std::vector<NeuralNetwork> newPopulation;
        for (int i = 0; i < survivors; i++) {
            newPopulation.push_back(population[rankedScores[i].second]);
        }

        while (newPopulation.size() < generationSize) {
            NeuralNetwork parent1 = newPopulation[rand() % survivors];
            NeuralNetwork parent2 = newPopulation[rand() % survivors];

            NeuralNetwork child = parent1;
            for (int i = 0; i < child.weights.size(); i++) {
                if (rand() % 2) {
                    child.weights[i] = parent2.weights[i];
                }
            }
            child.mutate(mutationRate);
            newPopulation.push_back(child);
        }

        population = newPopulation;
        saveWeights(population[0], weightFile);
    }

    endwin();
    return 0;
}
