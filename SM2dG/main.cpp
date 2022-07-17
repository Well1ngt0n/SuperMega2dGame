#include <iostream>
#include "engine.h"

int main() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "GameBebra beta", sf::Style::Default);
    Game game;
    window.clear();
    window.display();

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
                break;
            }
            else {
                game.update(event);
            }
        }
        game.move();
        game.draw(window);
        window.display();
        sf::sleep(sf::Time(sf::milliseconds(10)));
    }
    return 0;
}
