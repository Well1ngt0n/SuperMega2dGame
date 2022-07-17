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
MAX_TEXTURES_COUNT = 200,
WORLD_SIZE = 256,
CHUNK_SIZE = 512,
MAX_RENDER_DISTANCE = 7;

bool active_chunks[WORLD_SIZE][WORLD_SIZE];
json<int, sf::Texture> textures; // Жоска экономим память, не храня спрайты

void init_textures() {
    for (int i = 0; i < MAX_TEXTURES_COUNT; i++) {
        string num = std::to_string(i);
        string name = "texture" + num;
        textures[i].loadFromFile(name);
    }
}

struct Game {
    int screen_x_size, screen_y_size;

    struct Object : public sf::Sprite {
        int id;
        int x_coord, y_coord;
        json<string, string> parameters;
        int static_texture;
        vector<int> animated_texture;
        Object(int n_x, int n_y, int n_id) :
            x_coord{ n_x }, y_coord{ n_y }, id{ n_id } {}


        void draw(int player_x, int player_y) {

        }
    };


    struct MovableObject : public Object {
        int speed, dx, dy;
        MovableObject(int x, int y, int id) : Object(x, y, id) {}
        void move() {
            x_coord += speed * dx;
            y_coord += speed * dy;
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
        Player() : MovableObject(0, 0, 0) {}; //TODO: fix player coords
        CockPack pack;
        Item left_hand_item, right_hand_item;
        string direction = "down";
        int dx = 0, dy = 0;
        int speed = 5;
        int walk = 0;
        int attack = 0;
    };

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
        void load(int x, int y) {
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
                    int xcoord, ycoord;
                    cin >> xcoord >> ycoord;
                    if (bool_mobs_reading)
                        Mob new_mob(1, 2, 3);

                    else
                        objects.emplace_back(Object(xcoord, ycoord, type));

                }
            }
        }

        void del() {
            // TODO: вот тут надо файлик поправить
            mobs.clear();
            mobs.shrink_to_fit();
            objects.clear();
            objects.shrink_to_fit();
        }
    };

    Player player;
    int max_fps = 60;
    vector<vector<Chunk>> chunks;

    Game() {
        chunks.resize(WORLD_SIZE, vector<Chunk>(WORLD_SIZE));
    }


    void update(sf::Event& event) {
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
        }
        else if (event.type == sf::Event::KeyReleased) {
            if (event.key.code == sf::Keyboard::A || event.key.code == sf::Keyboard::D) {
                player.dx = 0;
                if (player.dy == 0) player.walk = 0;
            }
            else {
                player.dy = 0;
                if (player.dx == 0) player.walk = 0;
            }
        }

        int chunk_x_coord = player.x_coord / CHUNK_SIZE;
        int chunk_y_coord = player.y_coord / CHUNK_SIZE;
        for (int i = 0; i < MAX_RENDER_DISTANCE + 2; i++) {
            int j = MAX_RENDER_DISTANCE + 1 - i;
            int dx[4] = { -1, -1, 1, 1 };
            int dy[4] = { 1, -1, -1, 1 };
            for (int d = 0; d < 4; d++) {
                int nx = chunk_x_coord + i * dx[d];
                int ny = chunk_y_coord + j * dy[d];
                if (nx < 0 || ny < 0) continue;
                nx %= WORLD_SIZE;
                ny %= WORLD_SIZE;

                if (active_chunks[nx][ny]) {
                    chunks[nx][ny].del();
                    active_chunks[nx][ny] = false;
                }
            }
        }

        for (int i = 0; i < MAX_RENDER_DISTANCE + 1; i++) {
            int j = MAX_RENDER_DISTANCE + 1 - i;
            int dx[4] = { -1, -1, 1, 1 };
            int dy[4] = { 1, -1, -1, 1 };
            for (int d = 0; d < 4; d++) {
                int nx = chunk_x_coord + i * dx[d];
                int ny = chunk_y_coord + j * dy[d];
                if (nx < 0 || ny < 0) continue;
                nx %= WORLD_SIZE;
                ny %= WORLD_SIZE;

                if (!active_chunks[nx][ny]) {
                    chunks[nx][ny].load(nx, ny);
                    active_chunks[nx][ny] = true;
                }
            }
        }
    }

    void move() {
        for (auto& row_chunk : chunks) {
            for (auto& chunk : row_chunk) {
                for (auto& i : chunk.mobs) {
                    i.logic();
                    i.move();
                    //if (){ // проверка на то что моб клоун и ливнул из чанка
                    //
                    //}
                }
            }
        }
    }


    void draw(sf::RenderWindow& window) {
        for (auto& row_chunk : chunks) {
            for (auto& chunk : row_chunk) {
                for (auto& i : chunk.mobs) {
                    i.draw(player.x_coord, player.y_coord);
                }
                for (auto& i : chunk.objects) {
                    i.draw(player.x_coord, player.y_coord);
                }
            }
        }
    }

};

#endif //UNTITLED4_ENGINE_H