#include <iostream>
#include "engine.h"

int main() {
    sf::RenderWindow window(sf::VideoMode(window_width, window_height), "GameBebra beta"/*, sf::Style::Fullscreen*/);
    init_textures();
    Game game(&window);
    game.player.inventory.load();
    window.clear({0, 0, 100, 255});
    while (window.isOpen()) {
        sf::Event event{};
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                game.exit();
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
        sf::sleep(sf::Time(sf::milliseconds((int)game.calculate_sleep_time())));
    }
    return 0;
}
