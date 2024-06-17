#include <iostream>
#include <vector> 
#include <string>

std::vector<std::string> gw(){
    std::vector<std::string> строки;
    строки.push_back ("ggvdtb");
    return строки;
}

std::vector<int> dsw() {
    std::vector<int> числа;
    числа.push_back(20);
    числа.push_back(50);
    числа.push_back(90);
    числа.push_back(10);
    числа.push_back(40);
    числа.push_back(250);
    числа.push_back(300);
    числа.push_back(45);
    числа.push_back(34);
    числа.push_back(280);
    числа.push_back(14);
    числа.push_back(20);
    return числа;
}

 /*int main() {
    std::vector<int> числа = dsw();
    bool found = false;
    int index = -1; // Индекс ячейки, где найдено число 300
    for (int i = 0; i < числа.size(); ++i) {
        if (числа[i] == 300) {
            found = true;
            index = i; // Сохраняем индекс ячейки
            break; // Найдено число 300, завершаем цикл
        }
    }
    if (found) {
        std::cout << "Число 300 найдено в ячейке с индексом: " << index << std::endl;
    } else {
        std::cout << "Число 300 не найдено" << std::endl;
    }

    std::vector<std::string> строки = gw();
   int indexx = -1;
    for (int d = 0; d < строки.size(); ++d){

        std::cout << строки[d] << std::endl;
        indexx = d;
        std::cout << "ячейка: " << indexx << std::endl;
    }
    return 0;
}
*/