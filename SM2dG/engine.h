//created by ООО "Разрабы Дауны"
#ifndef UNTITLED4_ENGINE_H
#define UNTITLED4_ENGINE_H
#define json unordered_map

#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include <bits/stdc++.h>
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

using namespace sf;
using namespace std;

const int MAX_PACK_COCK_SIZE = 20, // Размер рюкзака
MAX_FAST_PACK_COCK_SIZE = 5, // Размер строки быстрого доступа
MAX_TEXTURES_COUNT = 200,
        WORLD_SIZE = 256,
        CHUNK_SIZE = 512,
        MAX_RENDER_DISTANCE = 7;

bool active_chunks[WORLD_SIZE][WORLD_SIZE];
json<int, Texture> textures; // Жоска экономим память, не храня спрайты

void init_textures() {
    for (int i = 0; i < MAX_TEXTURES_COUNT; i++) {
        string num = to_string(i);
        string name = "texture" + num;
        textures[i].loadFromFile(name);
    }
}

struct Game {

    int screen_x_size, screen_y_size;

    struct Object : public Sprite {
        int id;
        int x_coord, y_coord;
        json<string, string> parameters;
        int static_texture;
        vector<int> animated_texture;

        void init(int x, int y, int n_id) {
            x_coord = x, y_coord = y, id = n_id;
        }

        void draw(int player_x, int player_y) {

        }
    };

    struct MovableObject : public Object {
        int speed, dx, dy;

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
        Item inventory[MAX_PACK_COCK_SIZE];
        bool is_open = false;
    };

    struct Player : public MovableObject {
        CockPack pack;
        Item in_left_hand, in_right_hand;
        string left_right_up_down = "down";
        int dx = 0, dy = 0;
        int speed = 5;
        int walk = 0;
        int attack = 0;
    };

    struct Mob : public MovableObject {
        vector<int> animation_textures;
        int stop_texture;
        void init2(){

        }
        void logic() {

        }
    };

    struct Chunk {
        vector<Mob> mobs;
        vector<Object> objects;
        void load(int x, int y){
            freopen((to_string(x) + "-" + to_string(y) + ".chunk").c_str(), "r", stdin);
            string cur;
            cin >> cur >> cur;
            while (cur != "objects"){
                int type = atoi(cur.c_str());
                int mx, my;
                cin >> mx >> my;
                Mob new_mob;
                new_mob.init(mx, my, type);
                mobs.emplace_back(new_mob);
                cin >> cur;
            }
            int type, ox, oy;
            while (cin >> type){
                cin >> ox >> oy;
                Object new_object;
                new_object.init(ox, oy, type);
                objects.emplace_back(new_object);
            }
        }
        void del(){
            // вот тут надо файлик поправить
            mobs.clear();
            mobs.shrink_to_fit();
            objects.clear();
            objects.shrink_to_fit();
        }
    };

    Player player;
    int max_fps = 60;
    vector<vector<Chunk>> chunks;

    void init(RenderWindow &window) {
        window.setTitle("GameBebra beta");
        chunks.resize(WORLD_SIZE, vector<Chunk>(WORLD_SIZE));
    }

    void update(Event &event) {
        if (event.type == Event::KeyPressed) {
            switch (event.key.code) {
                case Keyboard::A:
                    player.walk = 1;
                    player.left_right_up_down = "left";
                    player.dx = -1;
                    break;
                case Keyboard::D:
                    player.walk = 1;
                    player.left_right_up_down = "right";
                    player.dx = 1;
                    break;
                case Keyboard::W:
                    player.walk = 1;
                    player.left_right_up_down = "up";
                    player.dy = -1;
                    break;
                case Keyboard::S:
                    player.walk = 1;
                    player.left_right_up_down = "down";
                    player.dy = 1;
                    break;
                default:
                    break;
            }
        } else if (event.type == Event::KeyReleased) {
            if (event.key.code == Keyboard::A || event.key.code == Keyboard::D) {
                player.dx = 0;
                if (player.dy == 0) player.walk = 0;
            } else {
                player.dy = 0;
                if (player.dx == 0) player.walk = 0;
            }
        }
        int chunk_x_coord = player.x_coord / CHUNK_SIZE;
        int chunk_y_coord = player.y_coord / CHUNK_SIZE;
        for (int i = 0; i < MAX_RENDER_DISTANCE + 2; i++) {
            int j = MAX_RENDER_DISTANCE + 1 - i;
            int dx[4] = {-1, -1, 1, 1};
            int dy[4] = {1, -1, -1, 1};
            for (int d = 0; d < 4; d++) {
                int nx = chunk_x_coord + i * dx[d];
                int ny = chunk_y_coord + j * dy[d];
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
            int dx[4] = {-1, -1, 1, 1};
            int dy[4] = {1, -1, -1, 1};
            for (int d = 0; d < 4; d++) {
                int nx = chunk_x_coord + i * dx[d];
                int ny = chunk_y_coord + j * dy[d];
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
        for (auto &row_chunk: chunks){
            for (auto &chunk : row_chunk) {
                for (auto &i: chunk.mobs) {
                    i.logic();
                    i.move();
                    //if (){ // проверка на то что моб клоун и ливнул из чанка
                    //
                    //}
                }
            }
        }
    }


    void draw(RenderWindow &window) {
        for (auto &row_chunk: chunks){
            for (auto &chunk : row_chunk) {
                for (auto &i: chunk.mobs) {
                    i.draw(player.x_coord, player.y_coord);
                }
                for (auto &i : chunk.objects){
                    i.draw(player.x_coord, player.y_coord);
                }
            }
        }
    }

};

#endif //UNTITLED4_ENGINE_H