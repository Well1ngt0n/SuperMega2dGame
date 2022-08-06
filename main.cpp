#include <iostream>
#include "engine.h"

int main() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "GameBebra beta", sf::Style::Default);
    init_textures();
    Game game(&window);
    window.clear({0, 0, 100, 255});
    window.display();
    int ind = 0;
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
        sf::sleep(sf::Time(sf::milliseconds(game.calculate_sleep_time())));
    }
    return 0;
}
