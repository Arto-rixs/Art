#include <ncurses.h>
#include <stdlib.h>
#include <time.h>

const int w = 20, g = 20;
int x = 10, y = 10;
int ch;
int fx, fy;
int хвост;
int тх[100], ту[100];

void move_tail() {
    // Перемещаем каждый элемент хвоста на одну позицию вперед
    for (int i = хвост; i > 0; i--) {
        тх[i] = тх[i-1];
        ту[i] = ту[i-1];
    }
    тх[0] = x; // Новая позиция головы хвоста
    ту[0] = y;
}

void draw_board() {
    // Очистка экрана
    clear();

    // Рисуем границы поля
    for (int i = 0; i <= w; ++i) {
        mvprintw(0, i, "#");
        mvprintw(g, i, "#");
    }
    for (int j = 0; j <= g; ++j) {
        mvprintw(j, 0, "#");
        mvprintw(j, w, "#");
    }

    // Рисуем объекты на поле
    mvprintw(x, y, "0");
    mvprintw(fx, fy, "F");

    // Рисуем хвост
    for (int i = 1; i <= хвост; i++) {
        mvprintw(тх[i], ту[i], "o");
    }

    // Обновляем экран
    refresh();
}

int main() {
    // Инициализация ncurses
    initscr();
    keypad(stdscr, TRUE);
    curs_set(0);
    srand(time(NULL));

    // Инициализация начальных значений
    fx = 1 + (rand() % (w - 2));
    fy = 1 + (rand() % (g - 2));
    хвост = 0;

    // Основной цикл программы
    halfdelay(1);
    while ((ch = getch()) != 'q') {
        // Обработка пользовательского ввода
        switch(ch) {
            case KEY_UP:
                if (x > 1) {
                    mvprintw(x, y, " ");
                    x--;
                    move_tail();
                    draw_board();
                }
                break;
            case KEY_DOWN:
                if (x < w - 1) {
                    mvprintw(x, y, " ");
                    x++;
                    move_tail();
                    draw_board();
                }
                break;
            case KEY_LEFT:
                if (y > 1) {
                    mvprintw(x, y, " ");
                    y--;
                    move_tail();
                    draw_board();
                }
                break;
            case KEY_RIGHT:
                if (y < g - 1) {
                    mvprintw(x, y, " ");
                    y++;
                    move_tail();
                    draw_board();
                }
                break;
            default:
                break;
        }

        // Проверка столкновения с объектом F
        if (x == fx && y == fy) {
            хвост++;
            if (хвост > 100) хвост = 100;
            fx = 1 + (rand() % (w - 2));
            fy = 1 + (rand() % (g - 2));
            mvprintw(fx, fy, "F");
            draw_board();
        }
    }

    // Завершение работы ncurses
    endwin();
    return 0;
}
