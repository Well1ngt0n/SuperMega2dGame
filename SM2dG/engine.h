//
// Created by adamlocal on 25.05.2022.
//

#ifndef UNTITLED4_ENGINE_H
#define UNTITLED4_ENGINE_H
#define json unordered_map

#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <bits/stdc++.h>

using namespace sf;
using namespace std;

const int MAX_PACK_COCK_SIZE = 20, // Размер рюкзака
MAX_FAST_PACK_COCK_SIZE = 5, // Размер строки быстрого доступа
MAX_TEXTURES_COUNT = 200,
WORLD_SIZE = 1000;

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

    RenderWindow window;


    struct Object : public Sprite{
        int id;
        int x_coord, y_coord;
        json<string, string> parameters;
        void init(int x, int y, int n_id){
            x_coord = x, y_coord = y, id = n_id;
        }
    };

    struct MovableObject : public Object{
        int speed, dx, dy;
        void move(){
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

    Player player;

    void init(){
        window.setTitle("GameBebra beta");
        auto sizes = VideoMode::getFullscreenModes();
        auto h = sizes[0].height, w = sizes[0].width;
        window.setSize({w, h});
    }

    void update(Event& event){
        if (event.type == Event::KeyPressed){
            switch (event.key.code){
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
        } else if (event.type == Event::KeyReleased){
            if (event.key.code == Keyboard::A || event.key.code == Keyboard::D) {
                player.dx = 0;
                if (player.dy == 0) player.walk = 0;
            } else {
                player.dy = 0;
                if (player.dx == 0) player.walk = 0;
            }
        }
    }

    void move(){

    };

    void draw(){

    }



    int max_fps = 60;

};

#endif //UNTITLED4_ENGINE_H
