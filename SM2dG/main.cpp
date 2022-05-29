#include <iostream>
#include "engine.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

int main() {
    Game game;
    RenderWindow window(VideoMode(0, 0), "", Style::Fullscreen);
    game.init(window);
    window.display();

    //game.draw();
    //window.display();
    while (window.isOpen()){
        Event event;
        while (window.pollEvent(event)){
            if (event.type == Event::Closed){
                window.close();
                break;
            } else {
                game.update(event);
            }
        }
        game.move();
        game.draw(window);
        window.display();
    }
    return 0;
}
