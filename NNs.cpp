#include <iostream>

struct флаг {
    bool переключение : 1;
    bool переключение2 : 1;

};

int main (){
флаг ф;
ф.переключение = false;
ф.переключение2 = true;
std::cout << ф.переключение << std::endl;
std::cout << ф.переключение2 << std::endl;


    return 0;
}