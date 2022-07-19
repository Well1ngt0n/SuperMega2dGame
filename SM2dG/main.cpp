#include <iostream>
#include "engine.h"

int main() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "GameBebra beta", sf::Style::Default);
    Game game(&window);
    init_textures();
    // window.clear({0, 0, 100, 255});
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
        window.clear({0, 0, 100, 255});
        game.move();
        game.draw();
        window.display();
        sf::sleep(sf::Time(sf::milliseconds(10)));
    }
    return 0;
}
