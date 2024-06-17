#include <iostream>
#include <ncurses.h>
#include <unistd.h>
#include <cstdlib>
#include <ctime>

using namespace std;

bool gameOver;
const int width = 20, height = 20;
int x, y, fruitX, fruitY, score;
int tailX[100], tailY[100];
int nTail;
enum eDirection { STOP = 0, LEFT, RIGHT, UP, DOWN };
eDirection dir;

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
    srand(time(NULL)); // Инициализация генератора случайных чисел
    fruitX = (rand() % (width - 2)) + 1;
    fruitY = (rand() % (height - 2)) + 1;
    score = 0;
}

void Draw() {
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
    mvprintw(height + 4, 0, "Score: %d", score);
    refresh();
}

void Input() {
    keypad(stdscr, TRUE);
    halfdelay(1);
    int c = getch();
    switch (c) {
        case KEY_LEFT:
            dir = LEFT;
            break;
        case KEY_RIGHT:
            dir = RIGHT;
            break;
        case KEY_UP:
            dir = UP;
            break;
        case KEY_DOWN:
            dir = DOWN;
            break;
        case 'x':
            gameOver = true;
            break;
    }
}

void Logic() {
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
    if (x >= width) x = 0; else if (x < 0) x = width - 1;
    if (y >= height) y = 0; else if (y < 0) y = height - 1;

    for (int i = 0; i < nTail; i++)
        if (tailX[i] == x && tailY[i] == y)
            gameOver = true;

    if (x == fruitX && y == fruitY) {
        score += 10;
        fruitX = (rand() % (width - 2)) + 1;
        fruitY = (rand() % (height - 2)) + 1;
        nTail++;
    }
}

int main() {
    Setup();
    while (!gameOver) {
        Draw();
        Input();
        Logic();
        usleep(100000); //sleep(10);
    }
    endwin();
    return 0;
}
