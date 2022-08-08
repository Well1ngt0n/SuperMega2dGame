//created by ООО "Разрабы Дауны"
#ifndef UNTITLED4_ENGINE_H
#define UNTITLED4_ENGINE_H
#define json std::map

#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include <bits/stdc++.h>
// #include "rapidjson/document.h"
// #include "rapidjson/writer.h"
// #include "rapidjson/stringbuffer.h"

using std::string, std::vector, std::cin, std::cout;

const string dir_path = "../";

const int
        MAX_TEXTURES_COUNT = 10,
        WORLD_SIZE = 256,
        CHUNK_SIZE = 32,
        MAX_RENDER_DISTANCE = 7,
        WORLD_PIXEL_SIZE = WORLD_SIZE * CHUNK_SIZE,
        PACK_WIDTH = 10,
        PACK_HEIGHT = 3,
        FAST_PACK_SIZE = 10,
        SLOT_PIXEL_SIZE = 32,
        ITEMS_COUNT = 1;

int window_width = 800, window_height = 600;

bool active_chunks[WORLD_SIZE][WORLD_SIZE];
sf::Texture textures[MAX_TEXTURES_COUNT]; // Жоска экономим память, не храня спрайты
sf::Texture item_textures[ITEMS_COUNT];
sf::Sprite active_slot_sprite, default_slot_sprite;
sf::Font test_font;
sf::Font cnt_font;


void init_textures() {

    for (int i = 0; i < MAX_TEXTURES_COUNT; i++) {
        string name = dir_path + "textures/" + std::to_string(i) + ".png";
        textures[i].loadFromFile(name);
    }

    for (int i = 0; i < ITEMS_COUNT; i++) {
        string name = dir_path + "textures/items/" + std::to_string(i) + ".png";
        sf::Image im;
        im.loadFromFile(name);
        im.createMaskFromColor({255, 255, 255});
        item_textures[i].loadFromImage(im);
    }

}

struct Game {
    sf::RenderWindow *window;
    std::chrono::duration<float> time_passed;
    std::chrono::time_point<std::chrono::system_clock> last_update_time;
    float last_sleep_time = 10;

    Game(sf::RenderWindow *n_window) : window{n_window} {
        std::srand(std::time(nullptr));
        chunks.resize(WORLD_SIZE, vector<Chunk>(WORLD_SIZE));
        test_font.loadFromFile(dir_path + "Hack-Regular.ttf");
        cnt_font.loadFromFile(dir_path+"fonts/"+"Vogue Bold.ttf");

        for (int i = 0; i < WORLD_SIZE; ++i)
            for (int j = 0; j < WORLD_SIZE; ++j)
                active_chunks[i][j] = false;

        last_update_time = std::chrono::system_clock::now();
    }

    struct Object : public sf::Sprite {
        Object(int n_x, int n_y, int n_id) :
                x_coord{n_x}, y_coord{n_y}, id{n_id} {}

        int x_coord, y_coord, id;
        json<string, string> parameters;
        int static_texture;
        vector<int> animated_texture;

        void draw(int player_x, int player_y) {
            return;
        }
    };


    struct MovableObject : public Object {
        int speed;
        float dx, dy, actual_x_coord, actual_y_coord;

        MovableObject(int x, int y, int id) : Object(x, y, id) {
            actual_x_coord = x;
            actual_y_coord = y;
        }

        void move(std::chrono::duration<float> &time_passed) {
            actual_x_coord = actual_x_coord + speed * dx * time_passed.count();
            actual_y_coord = actual_y_coord + speed * dy * time_passed.count();
            while (actual_x_coord < 0) actual_x_coord += WORLD_PIXEL_SIZE;
            while (actual_y_coord < 0) actual_y_coord += WORLD_PIXEL_SIZE;
            while (actual_x_coord >= WORLD_PIXEL_SIZE) actual_x_coord -= WORLD_PIXEL_SIZE;
            while (actual_y_coord >= WORLD_PIXEL_SIZE) actual_y_coord -= WORLD_PIXEL_SIZE;
            x_coord = (int) actual_x_coord;
            y_coord = (int) actual_y_coord;
        }

    };

    struct Inventory {
        struct Item {
            int id = -1;
            int max_stack_size = 256;
            json<string, string> params;
        };

        struct Slot {
            int x_pix, y_pix;
            bool is_active = 0;
            Item item;
            int cnt = 0;

            bool is_pressed(int x_mouse, int y_mouse) const {
                return (x_mouse >= x_pix && x_mouse < x_pix + SLOT_PIXEL_SIZE) &&
                       (y_mouse >= y_pix && y_mouse < y_pix + SLOT_PIXEL_SIZE);
            }

            void draw(sf::RenderWindow *window, bool draw_slot_icon=true) const {
                if (is_active) {
                    active_slot_sprite.setPosition((float) x_pix, (float) y_pix);
                    window->draw(active_slot_sprite);
                } else if(draw_slot_icon){
                    default_slot_sprite.setPosition((float) x_pix, (float) y_pix);
                    window->draw(default_slot_sprite);
                }
                if (item.id != -1) {
                    sf::Sprite item_sprite;
                    item_sprite.setTexture(item_textures[item.id]);
                    item_sprite.setPosition((float)x_pix, (float)y_pix);
                    window->draw(item_sprite);
                    if(item.max_stack_size != 1) {
                        sf::Text text_count(std::to_string(cnt), cnt_font);
                        text_count.setCharacterSize(SLOT_PIXEL_SIZE/2);
                        text_count.setFillColor(sf::Color(0, 0, 0));
                        text_count.setPosition((float) x_pix + SLOT_PIXEL_SIZE / 2 ,
                                               (float) y_pix + SLOT_PIXEL_SIZE * 3 / 7);
                        window->draw(text_count);
                    }
                }
            }

            void swap_item(Slot& other){
                std::swap(other.item, item);
                std::swap(other.cnt, cnt);
            }
        };

        Slot invisible_mouse_slot;
        Slot left_hand, right_hand;
        Slot fast_pack[FAST_PACK_SIZE];
        Slot cock_pack[PACK_HEIGHT][PACK_WIDTH];
        Slot armor[4];
        std::pair<int, int> active_slot = {0, 0}; // default nums
        bool is_cock_pack_open = false;

        Inventory() {
            active_slot_sprite.setTexture(textures[9]);
            default_slot_sprite.setTexture(textures[8]);
            left_hand.item.id = 0; // test
            left_hand.cnt = 10;
            left_hand.x_pix = SLOT_PIXEL_SIZE / 2;
            left_hand.y_pix = 2 * SLOT_PIXEL_SIZE;
            right_hand.x_pix = SLOT_PIXEL_SIZE / 2 + SLOT_PIXEL_SIZE + 2 * SLOT_PIXEL_SIZE;
            right_hand.y_pix = 2 * SLOT_PIXEL_SIZE;
            for (int i = 0; i < 4; i++) {
                armor[i].x_pix = SLOT_PIXEL_SIZE / 2 + SLOT_PIXEL_SIZE + SLOT_PIXEL_SIZE / 2;
                armor[i].y_pix = SLOT_PIXEL_SIZE / 4 + i * SLOT_PIXEL_SIZE + SLOT_PIXEL_SIZE / 4 * i;
            }
            fast_pack[0].is_active = true;
            for (int i = 0; i < FAST_PACK_SIZE; i++) {
                fast_pack[i].x_pix =
                        4 * SLOT_PIXEL_SIZE + SLOT_PIXEL_SIZE / 2 + 2 * SLOT_PIXEL_SIZE + i * SLOT_PIXEL_SIZE;
                fast_pack[i].y_pix = SLOT_PIXEL_SIZE;
            }
            for (int i = 0; i < PACK_HEIGHT; i++) {
                for (int j = 0; j < PACK_WIDTH; j++) {
                    cock_pack[i][j].x_pix =
                            4 * SLOT_PIXEL_SIZE + SLOT_PIXEL_SIZE / 2 + 2 * SLOT_PIXEL_SIZE + j * SLOT_PIXEL_SIZE;
                    cock_pack[i][j].y_pix = 2 * SLOT_PIXEL_SIZE + SLOT_PIXEL_SIZE / 4 + i * SLOT_PIXEL_SIZE;
                }
            }
        }

        void left_swap() {
            if (active_slot.first == 0) {
                left_hand.swap_item(fast_pack[active_slot.second]);
            } else if (active_slot.first == 1) {
                left_hand.swap_item(cock_pack[active_slot.second / PACK_WIDTH][active_slot.second % PACK_WIDTH]);
            } else if (active_slot.first == 2) {
                left_hand.swap_item(armor[active_slot.second]);
            }
        }

        void right_swap() {
            if (active_slot.first == 0) {
                right_hand.swap_item(fast_pack[active_slot.second]);
            } else if (active_slot.first == 1) {
                right_hand.swap_item(cock_pack[active_slot.second / PACK_WIDTH][active_slot.second % PACK_WIDTH]);
            } else if (active_slot.first == 2) {
                std::swap(right_hand.item, armor[active_slot.second].item);
                std::swap(right_hand.cnt, armor[active_slot.second].cnt);
            }
        }

        void swap_hands() {
            left_hand.swap_item(right_hand);
        }

        void draw(sf::RenderWindow *window) {
            left_hand.draw(window);
            right_hand.draw(window);
            for (const auto & i : armor) {
                i.draw(window);
            }
            for (const auto & i : fast_pack) {
                i.draw(window);
            }
            if (is_cock_pack_open) {
                for (auto & i : cock_pack) {
                    for (const auto & j : i) {
                        j.draw(window);
                    }
                }
            }
            invisible_mouse_slot.draw(window);
        }

        Slot &get_active_slot() {
            if (active_slot.first == 0) {
                return fast_pack[active_slot.second];
            } else if (active_slot.first == 1) {
                return cock_pack[active_slot.second / PACK_WIDTH][active_slot.second % PACK_WIDTH];
            } else {
                return armor[active_slot.second];
            }
        }
    };

    struct Player : public MovableObject {
        Player() : MovableObject(0, 0, 0) {
            dx = 0;
            dy = 0;
            speed = 300;
            setTexture(textures[0]);
        };

        int click_in_inventory(int x, int y) {
            if (inventory.left_hand.is_pressed(x, y)) return 1;
            if (inventory.right_hand.is_pressed(x, y)) return 2;
            bool flag = false;
            for (int i = 0; i < 4; i++) {
                if (inventory.armor[i].is_pressed(x, y)) {
                    flag = true;
                    inventory.get_active_slot().is_active = false;
                    inventory.armor[i].is_active = true;
                    inventory.active_slot = {2, i};
                    break;
                }
            }
            if (flag) return 3;
            for (int i = 0; i < FAST_PACK_SIZE; i++) {
                if (inventory.fast_pack[i].is_pressed(x, y)) {
                    flag = true;
                    inventory.get_active_slot().is_active = false;
                    inventory.fast_pack[i].is_active = true;
                    inventory.active_slot = {0, i};
                    break;
                }
            }
            if (flag) return 4;
            if (inventory.is_cock_pack_open) {
                for (int i = 0; i < PACK_HEIGHT; i++) {
                    for (int j = 0; j < PACK_WIDTH; j++) {
                        if (inventory.cock_pack[i][j].is_pressed(x, y)) {
                            flag = true;
                            inventory.get_active_slot().is_active = false;
                            inventory.cock_pack[i][j].is_active = true;
                            inventory.active_slot = {1, i * PACK_WIDTH + j};
                            break;
                        }
                    }
                }
            }
            if (flag) return 5;
            return 0;

        }

        void update() {
            walk = 1;
            if (d_up) {
                if (d_right) {
                    setTexture(textures[4]);
                    dx = 0.7;
                    dy = -0.7;
                } else if (d_left) {
                    setTexture(textures[5]);
                    dx = -0.7;
                    dy = -0.7;
                } else {
                    setTexture(textures[0]);
                    dx = 0;
                    dy = -1;
                }
            } else if (d_down) {
                if (d_right) {
                    setTexture(textures[7]);
                    dx = 0.7;
                    dy = 0.7;
                } else if (d_left) {
                    setTexture(textures[6]);
                    dx = -0.7;
                    dy = 0.7;
                } else {
                    setTexture(textures[2]);
                    dx = 0;
                    dy = 1;
                }
            } else if (d_right) {
                setTexture(textures[3]);
                dx = 1;
                dy = 0;
            } else if (d_left) {
                setTexture(textures[1]);
                dx = -1;
                dy = 0;
            } else {
                walk = 0;
                dx = 0;
                dy = 0;
            }
            setPosition(400, 300);
        }

        void draw(sf::RenderWindow *window) {
            window->draw(*this);
            inventory.draw(window);
        }

        Inventory inventory;

        bool d_right = false, d_left = false, d_up = false, d_down = false;

        int walk = 0;
        int attack = 0;
    };

    Player player{};

    static std::pair<int, int> get_window_coords(int x, int y, int player_x, int player_y) {
        int nx = window_width / 2 - player_x + x, ny = window_height / 2 - player_y + y;
        if (x > player_x && WORLD_PIXEL_SIZE + player_x - x < x - player_x) {
            nx = window_width / 2 - (WORLD_PIXEL_SIZE + player_x - x);
        } else if (x <= player_x && WORLD_PIXEL_SIZE - player_x + x < player_x - x) {
            nx = window_width / 2 + (WORLD_PIXEL_SIZE - player_x + x);
        }
        if (y > player_y && WORLD_PIXEL_SIZE + player_y - y < y - player_y) {
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
            x_coord = x, y_coord = y;
            std::ifstream f((dir_path + "chunks/" + std::to_string(x_coord) + "-" + std::to_string(y_coord) +
                             ".chunk").c_str());
            color = {uint8_t(std::rand() % 256), uint8_t(std::rand() % 256), uint8_t(std::rand() % 256), 255};
            if (!f.good()) return;
            int mobs_cnt;
            f >> mobs_cnt;
            for (int i = 0; i < mobs_cnt; ++i) {
                int mob_type, mob_x, mob_y;
                f >> mob_type >> mob_x >> mob_y;
                mobs.emplace_back(Mob(mob_x, mob_y, mob_type));
            }

            int obj_cnt;
            f >> obj_cnt;
            for (int i = 0; i < obj_cnt; ++i) {
                int obj_type, obj_x, obj_y;
                f >> obj_type >> obj_x >> obj_y;
                objects.emplace_back(Object(obj_x, obj_y, obj_type));
            }
        }

        void upload() {
            std::ofstream test(
                    (dir_path+"chunks/" + std::to_string(x_coord) + "-" + std::to_string(y_coord) + ".chunk").c_str(),
                    std::ios::in);
            if (!test.good()) return;
            std::ofstream f(
                    (dir_path+"chunks/" + std::to_string(x_coord) + "-" + std::to_string(y_coord) + ".chunk").c_str(),
                    std::ios::out | std::ios::trunc);
            if (!f.good()) return;
            int mobs_cnt = mobs.size();
            f << mobs_cnt << '\n';
            for (int i = 0; i < mobs_cnt; ++i) {
                f << mobs[i].id << ' ' << mobs[i].x_coord << ' ' << mobs[i].y_coord << '\n';
            }
            int obj_cnt = objects.size();
            f << obj_cnt << '\n';
            for (int i = 0; i < obj_cnt; ++i) {
                f << objects[i].id << ' ' << objects[i].x_coord << ' ' << objects[i].y_coord << '\n';
            }
        }

        void draw(int player_x, int player_y, sf::RenderWindow *window) {
            sf::RectangleShape shape({CHUNK_SIZE, CHUNK_SIZE});
            shape.setFillColor(color);
            int x = x_coord * CHUNK_SIZE % WORLD_PIXEL_SIZE;
            int y = y_coord * CHUNK_SIZE % WORLD_PIXEL_SIZE;
            auto xy = get_window_coords(x, y, player_x, player_y);
            x = xy.first, y = xy.second;
            shape.setPosition((float) x, (float) y);
            window->draw(shape);
        }

        void del() {
            upload();
            mobs.clear();
            mobs.shrink_to_fit();
            objects.clear();
            objects.shrink_to_fit();
        }
    };

    struct Debug {
        int frames_current_count = 0, frames_latest_count = 0;
        std::time_t frames_time;

        void draw_player_coords(sf::RenderWindow *window, Player *player) {
            sf::Text coords_text(std::to_string(player->x_coord) + ' ' + std::to_string(player->y_coord), test_font);
            coords_text.setPosition(10, 10);
            coords_text.setCharacterSize(18);
            window->draw(coords_text);
        }

        void draw_fps(sf::RenderWindow *window) {
            if (frames_time == std::time(nullptr)) ++frames_current_count;
            else {
                frames_time = std::time(nullptr);
                frames_latest_count = frames_current_count;
                frames_current_count = 0;
            }
            sf::Text fps_text(std::to_string(frames_latest_count), test_font);
            fps_text.setPosition(200, 10);
            fps_text.setCharacterSize(18);
            window->draw(fps_text);
        }
    };

    const int max_fps = 60;
    vector<vector<Chunk>> chunks;
    Debug debug{};

    void update(sf::Event &event) {
        if (event.type == sf::Event::KeyPressed) {
            switch (event.key.code) {
                case sf::Keyboard::A:
                    player.d_left = true;
                    player.d_right = false;
                    break;
                case sf::Keyboard::D:
                    player.d_right = true;
                    player.d_left = false;
                    break;
                case sf::Keyboard::W:
                    player.d_up = true;
                    player.d_down = false;
                    break;
                case sf::Keyboard::S:
                    player.d_down = true;
                    player.d_up = false;
                    break;
                case sf::Keyboard::E:
                    player.inventory.is_cock_pack_open ^= 1;
                    if (player.inventory.is_cock_pack_open == 0) {
                        if (player.inventory.active_slot.first == 1) {
                            player.inventory.get_active_slot().is_active = false;
                            player.inventory.fast_pack[0].is_active = true;
                            player.inventory.active_slot = {0, 0};
                        }
                    }
                    break;
                case sf::Keyboard::F:
                    player.inventory.swap_hands();
                    break;
                case sf::Keyboard::Z:
                    player.inventory.left_swap();
                    break;
                case sf::Keyboard::X:
                    player.inventory.right_swap();
                    break;
                case sf::Keyboard::Num1:
                    player.inventory.get_active_slot().is_active = false;
                    player.inventory.fast_pack[0].is_active = true;
                    player.inventory.active_slot = {0, 0};
                    break;
                case sf::Keyboard::Num2:
                    player.inventory.get_active_slot().is_active = false;
                    player.inventory.fast_pack[1].is_active = true;
                    player.inventory.active_slot = {0, 1};
                    break;
                case sf::Keyboard::Num3:
                    player.inventory.get_active_slot().is_active = false;
                    player.inventory.fast_pack[2].is_active = true;
                    player.inventory.active_slot = {0, 2};
                    break;
                case sf::Keyboard::Num4:
                    player.inventory.get_active_slot().is_active = false;
                    player.inventory.fast_pack[3].is_active = true;
                    player.inventory.active_slot = {0, 3};
                    break;
                case sf::Keyboard::Num5:
                    player.inventory.get_active_slot().is_active = false;
                    player.inventory.fast_pack[4].is_active = true;
                    player.inventory.active_slot = {0, 4};
                    break;
                case sf::Keyboard::Num6:
                    player.inventory.get_active_slot().is_active = false;
                    player.inventory.fast_pack[5].is_active = true;
                    player.inventory.active_slot = {0, 5};
                    break;
                case sf::Keyboard::Num7:
                    player.inventory.get_active_slot().is_active = false;
                    player.inventory.fast_pack[6].is_active = true;
                    player.inventory.active_slot = {0, 6};
                    break;
                case sf::Keyboard::Num8:
                    player.inventory.get_active_slot().is_active = false;
                    player.inventory.fast_pack[7].is_active = true;
                    player.inventory.active_slot = {0, 7};
                    break;
                case sf::Keyboard::Num9:
                    player.inventory.get_active_slot().is_active = false;
                    player.inventory.fast_pack[8].is_active = true;
                    player.inventory.active_slot = {0, 8};
                    break;
                case sf::Keyboard::Num0:
                    player.inventory.get_active_slot().is_active = false;
                    player.inventory.fast_pack[9].is_active = true;
                    player.inventory.active_slot = {0, 9};
                    break;
                default:
                    break;
            }

        } else if (event.type == sf::Event::KeyReleased) {
            switch (event.key.code) {
                case sf::Keyboard::A:
                    player.d_left = false;
                    break;
                case sf::Keyboard::D:
                    player.d_right = false;
                    break;
                case sf::Keyboard::W:
                    player.d_up = false;
                    break;
                case sf::Keyboard::S:
                    player.d_down = false;
                    break;
                default:
                    break;
            }
        } else if (event.type == sf::Event::MouseButtonPressed) {
            if (event.mouseButton.button == sf::Mouse::Left) {
                if (player.click_in_inventory(event.mouseButton.x, event.mouseButton.y));
                else {

                }
            }
        }

        player.update();
    }

    void move() {
        auto current_time = std::chrono::system_clock::now();
        time_passed = current_time - last_update_time;
        last_update_time = std::chrono::system_clock::now();

        for (auto &row_chunk: chunks) {
            for (auto &chunk: row_chunk) {
                for (auto &i: chunk.mobs) {
                    i.logic();
                    i.move(time_passed);
                    //if (){ // TODO: проверка на то что моб клоун и ливнул из чанка
                    //
                    //}
                }
            }
        }
        player.move(time_passed);
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
                        for (auto &r: chunks[nx][ny].mobs)
                            r.draw(player.x_coord, player.y_coord);
                        for (auto &r: chunks[nx][ny].objects)
                            r.draw(player.x_coord, player.y_coord);
                    }
                }
            }
        }
        player.draw(window);
        debug.draw_player_coords(window, &player);
        debug.draw_fps(window);
    }

    float calculate_sleep_time() {
        float full_sleep_time = time_passed.count() * 1000, mspf = 1000 / max_fps;
        last_sleep_time = mspf + last_sleep_time - full_sleep_time;
        return last_sleep_time;
    }
};

#endif //UNTITLED4_ENGINE_H
