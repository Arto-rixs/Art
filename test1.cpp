#include <iostream>
#include <vector>
#include <cstdlib> // Для std::rand
#include <ctime>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h> // Для ioctl и FIONREAD
#include <dlib/dnn.h>

using namespace std;
using namespace dlib;

const int WIDTH = 25;
const int HEIGHT = 25;
int x, y, fruitX, fruitY, score;
enum Direction { STOP = 0, LEFT, RIGHT, UP, DOWN };
Direction dir;
std::vector<pair<int, int>> tail;

void Setup() {
    srand(time(0));
    x = WIDTH / 2;
    y = HEIGHT / 2;
    fruitX = std::rand() % WIDTH;
    fruitY = std::rand() % HEIGHT;
    score = 0;
    dir = STOP;
}

bool kbhit() {
    termios term;
    tcgetattr(0, &term);
    termios term2 = term;
    term2.c_lflag &= ~ICANON;
    tcsetattr(0, TCSANOW, &term2);
    int bytesWaiting;
    ioctl(0, FIONREAD, &bytesWaiting);
    tcsetattr(0, TCSANOW, &term);
    return bytesWaiting > 0;
}

int getch() {
    return getchar();
}

void Draw() {
    system("clear");
    for (int i = 0; i < WIDTH + 2; i++)
        cout << "#";
    cout << endl;

    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            if (j == 0)
                cout << "#";
            if (i == y && j == x)
                cout << "O";
            else if (i == fruitY && j == fruitX)
                cout << "*";
            else {
                bool print = false;
                for (int k = 0; k < tail.size(); k++) {
                    if (tail[k].first == j && tail[k].second == i) {
                        cout << "o";
                        print = true;
                    }
                }
                if (!print)
                    cout << " ";
            }
            if (j == WIDTH - 1)
                cout << "#";
        }
        cout << endl;
    }

    for (int i = 0; i < WIDTH + 2; i++)
        cout << "#";
    cout << endl;

    cout << "Score:" << score << endl;
}

void Input() {
    if (kbhit()) {
        switch (getch()) {
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
            exit(0);
        }
    }
}

void Logic() {
    int prevX = tail.empty() ? x : tail[0].first;
    int prevY = tail.empty() ? y : tail[0].second;
    int prev2X, prev2Y;
    if (!tail.empty()) {
        tail[0].first = x;
        tail[0].second = y;
    }
    for (int i = 1; i < tail.size(); i++) {
        prev2X = tail[i].first;
        prev2Y = tail[i].second;
        tail[i].first = prevX;
        tail[i].second = prevY;
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

    if (x >= WIDTH) x = 0; else if (x < 0) x = WIDTH - 1;
    if (y >= HEIGHT) y = 0; else if (y < 0) y = HEIGHT - 1;

    for (int i = 0; i < tail.size(); i++)
        if (tail[i].first == x && tail[i].second == y)
            exit(0);

    if (x == fruitX && y == fruitY) {
        score += 10;
        fruitX = std::rand() % WIDTH;
        fruitY = std::rand() % HEIGHT;
        tail.push_back(make_pair(x, y));
    }
}

using net_type = loss_multiclass_log<fc<4, relu<fc<128, input<matrix<float>>>>>>;

int main() {
    net_type net;

    matrix<float> sample;
    sample.set_size(1, WIDTH * HEIGHT);
    for (int i = 0; i < WIDTH * HEIGHT; ++i) {
        sample(0, i) = static_cast<float>(std::rand()) / RAND_MAX;
    }

    Setup();
    while (true) {
        Draw();
        Input();
        Logic();
        usleep(100000);
    }

    return 0;
}
