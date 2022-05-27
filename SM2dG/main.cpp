#include <iostream>
#include "engine.h"

int main() {
    Game game;
    game.init();
    game.draw();
    while (game.window.isOpen()){
        Event event;
        while (game.window.pollEvent(event)){
            if (event.type == Event::Closed){
                game.window.close();
                break;
            } else {
                game.update(event);
            }
        }
        game.move();
        game.draw();
    }
    return 0;
}
