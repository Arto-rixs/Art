#include <SFML/Graphics.hpp>

int main()
{
 sf::RenderWindow okno(sf::VideoMode(1920, 1080), L"Терминал");

 while(okno.isOpen())
 {
  sf::Event событие;
    while(okno.pollEvent(событие))
    {  
     if(событие.type == sf::Event::Closed)
     {
      okno.close();

     }

    }

 okno.clear();
 okno.display();
 }
   

 return 0;
}