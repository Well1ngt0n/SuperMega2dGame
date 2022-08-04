//created by ООО "Разрабы Дауны"
#ifndef UNTITLED4_ENGINE_H
#define UNTITLED4_ENGINE_H
#define json std::unordered_map

#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include <bits/stdc++.h>
// #include "rapidjson/document.h"
// #include "rapidjson/writer.h"
// #include "rapidjson/stringbuffer.h"

using std::string, std::vector, std::cin, std::cout;

const int
        MAX_COCK_PACK_SIZE = 20, // Размер рюкзака
MAX_FAST_COCK_PACK_SIZE = 5, // Размер строки быстрого доступа
MAX_TEXTURES_COUNT = 4,
        WORLD_SIZE = 256,
        CHUNK_SIZE = 256,
        MAX_RENDER_DISTANCE = 7,
        WORLD_PIXEL_SIZE = WORLD_SIZE * CHUNK_SIZE;

int window_width = 800, window_height = 600;

bool active_chunks[WORLD_SIZE][WORLD_SIZE];
vector<sf::Texture> textures(MAX_TEXTURES_COUNT); // Жоска экономим память, не храня спрайты

void init_textures() {
    for (int i = 0; i < MAX_TEXTURES_COUNT; i++) {
        string name = "../textures/" + std::to_string(i) + ".png";
        textures[i].loadFromFile(name);
    }
}

struct Game {
    sf::RenderWindow *window;

    Game(sf::RenderWindow *n_window) : window{n_window} {
        std::srand(std::time(nullptr));
        chunks.resize(WORLD_SIZE, vector<Chunk>(WORLD_SIZE));
    }

    struct Object : public sf::Sprite {
        Object(int n_x, int n_y, int n_id) :
                x_coord{n_x}, y_coord{n_y}, id{n_id} {}

        int id;
        int x_coord, y_coord;
        json<string, string> parameters;
        int static_texture;
        vector<int> animated_texture;

        void draw(int player_x, int player_y) {

        }
    };


    struct MovableObject : public Object {
        int speed, dx, dy;

        MovableObject(int x, int y, int id) : Object(x, y, id) {}

        void move() {
            x_coord = (x_coord + speed * dx + WORLD_PIXEL_SIZE) % WORLD_PIXEL_SIZE;
            y_coord = (y_coord + speed * dy + WORLD_PIXEL_SIZE) % WORLD_PIXEL_SIZE;
        }

    };

    struct Item {
        int id;
        char type;
        json<string, string> parameters;
    };

    struct CockPack {
        Item inventory[MAX_COCK_PACK_SIZE];
        bool is_open = false;
    };

    struct Player : public MovableObject {
        Player() : MovableObject(0, 0, 0) {
            dx = 0;
            dy = 0;
            speed = 5;
        };

        void update() {
            if (direction == "up") setTexture(textures[0]);
            else if (direction == "left") setTexture(textures[1]);
            else if (direction == "down") setTexture(textures[2]);
            else if (direction == "right") setTexture(textures[3]);
            setPosition(400, 300);
        }

        void draw(sf::RenderWindow *window) {
            window->draw(*this);
            sf::Font test_font;
            //test_font.loadFromFile("Hack-Regular.ttf");
            sf::Text coords_text(std::to_string(x_coord) + ' ' + std::to_string(y_coord), test_font);
            coords_text.setPosition(10, 10);
            coords_text.setCharacterSize(18);
            window->draw(coords_text);
        }

        CockPack pack;
        Item left_hand_item, right_hand_item;
        string direction = "down";

        int walk = 0;
        int attack = 0;
    };

    Player player;

    static std::pair<int, int> get_window_coords(int x, int y, int player_x, int player_y){
        int nx = window_width / 2 - player_x + x, ny = window_height / 2 - player_y + y;
        if (x > player_x && WORLD_PIXEL_SIZE + player_x - x < x - player_x){
            nx = window_width / 2 - (WORLD_PIXEL_SIZE + player_x - x);
        } else if (x <= player_x && WORLD_PIXEL_SIZE - player_x + x < player_x - x) {
            nx = window_width / 2 + (WORLD_PIXEL_SIZE - player_x + x);
        }
        if (y > player_y && WORLD_PIXEL_SIZE + player_y - y < y - player_y){
            ny = window_height / 2 - (WORLD_PIXEL_SIZE + player_y - y);
        } else if (y <= player_y && WORLD_PIXEL_SIZE - player_y + y < player_y - y) {
            ny = window_height / 2 + (WORLD_PIXEL_SIZE - player_y + y);
        }
        return {nx, ny};
    }


    struct Mob : public MovableObject {
        vector<int> animation_textures;
        int stop_texture;

        Mob(int x, int y, int id) : MovableObject(x, y, id) {}

        void init2() {

        }

        void logic() {

        }
    };

    struct Chunk {
        vector<Mob> mobs;
        vector<Object> objects;
        sf::Color color;
        int x_coord, y_coord;

        void load(int x, int y) {
            x_coord = x;
            y_coord = y;
            color = {std::rand() % 256, std::rand() % 256, std::rand() % 256, 255};
            freopen((".//chunks//" + std::to_string(x) + "-" + std::to_string(y) + ".chunk").c_str(), "r", stdin);
            bool bool_mobs_reading = false;
            string cur;
            if (!(cin >> cur)) return;
            if (cur == "mobs")
                bool_mobs_reading = true;

            while (cin >> cur) {
                if (cur == "mobs")
                    bool_mobs_reading = true;
                else if (cur == "objects")
                    bool_mobs_reading = false;
                else {
                    int type = atoi(cur.c_str());
                    int content_x_coord, content_y_coord;
                    cin >> content_x_coord >> content_y_coord;
                    if (bool_mobs_reading)
                        mobs.emplace_back(Mob(content_x_coord, content_y_coord, type));
                    else
                        objects.emplace_back(Object(content_x_coord, content_y_coord, type));
                }
            }
        }

        void draw(int player_x, int player_y, sf::RenderWindow *window) {
            sf::RectangleShape shape({CHUNK_SIZE, CHUNK_SIZE});
            shape.setFillColor(color);
            int x = x_coord * CHUNK_SIZE % WORLD_PIXEL_SIZE;
            int y = y_coord * CHUNK_SIZE % WORLD_PIXEL_SIZE;
            auto xy = get_window_coords(x, y, player_x, player_y);
            x = xy.first, y = xy.second;
            shape.setPosition({x, y});
            window->draw(shape);
        }

        void del() {
            // TODO: вот тут надо файлик поправить
            mobs.clear();
            mobs.shrink_to_fit();
            objects.clear();
            objects.shrink_to_fit();
        }
    };



    int max_fps = 60;
    vector<vector<Chunk>> chunks;

    void update(sf::Event &event) {
        if (event.type == sf::Event::KeyPressed) {
            switch (event.key.code) {
                case sf::Keyboard::A:
                    player.walk = 1;
                    player.direction = "left";
                    player.dx = -1;
                    break;
                case sf::Keyboard::D:
                    player.walk = 1;
                    player.direction = "right";
                    player.dx = 1;
                    break;
                case sf::Keyboard::W:
                    player.walk = 1;
                    player.direction = "up";
                    player.dy = -1;
                    break;
                case sf::Keyboard::S:
                    player.walk = 1;
                    player.direction = "down";
                    player.dy = 1;
                    break;
                default:
                    break;
            }
        } else if (event.type == sf::Event::KeyReleased) {
            if (event.key.code == sf::Keyboard::A || event.key.code == sf::Keyboard::D) {
                player.dx = 0;
                if (player.dy == 0) player.walk = 0;
            } else {
                player.dy = 0;
                if (player.dx == 0) player.walk = 0;
            }
        }

        player.update();
    }

    void move() {
        for (auto &row_chunk: chunks) {
            for (auto &chunk: row_chunk) {
                for (auto &i: chunk.mobs) {
                    i.logic();
                    i.move();
                    //if (){ // TODO: проверка на то что моб клоун и ливнул из чанка
                    //
                    //}
                }
            }
        }
        player.move();
    }


    void draw() {
        int dx[4] = {-1, -1, 1, 1};
        int dy[4] = {1, -1, -1, 1};
        for (int d = 0; d <= MAX_RENDER_DISTANCE + 1; d++) {
            for (int i = 0; i <= d; ++i) {
                int j = d - i;
                for (int k = 0; k < 4; k++) {
                    int x = player.x_coord / CHUNK_SIZE, y = player.y_coord / CHUNK_SIZE;
                    int nx = (x + dx[k] * i + WORLD_SIZE) % WORLD_SIZE, ny = (y + dy[k] * j + WORLD_SIZE) % WORLD_SIZE;
                    if (d == MAX_RENDER_DISTANCE + 1 && active_chunks[nx][ny]) {
                        active_chunks[nx][ny] = false;
                        chunks[nx][ny].del();
                    } else if (d <= MAX_RENDER_DISTANCE && !active_chunks[nx][ny]) {
                        active_chunks[nx][ny] = true;
                        chunks[nx][ny].load(nx, ny);
                    }

                    if (active_chunks[nx][ny]) {
                        chunks[nx][ny].draw(player.x_coord, player.y_coord, window);
                        for (auto &i: chunks[i][j].mobs)
                            i.draw(player.x_coord, player.y_coord);
                        for (auto &i: chunks[i][j].objects)
                            i.draw(player.x_coord, player.y_coord);
                    }
                }
            }
        }
        player.draw(window);
    }

};

#endif //UNTITLED4_ENGINE_H