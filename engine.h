//created by ООО "Разрабы Дауны"
#ifndef UNTITLED4_ENGINE_H
#define UNTITLED4_ENGINE_H
#define json std::map

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include <bits/stdc++.h>
#include "inventory.h"

// #include "rapidjson/document.h"
// #include "rapidjson/writer.h"
// #include "rapidjson/stringbuffer.h"


bool active_chunks[WORLD_SIZE][WORLD_SIZE];

struct Game {
    sf::RenderWindow *window;
    std::chrono::duration<float> time_passed;
    std::chrono::time_point<std::chrono::system_clock> last_update_time;
    float last_sleep_time = 10;

    Game(sf::RenderWindow *n_window) : window{n_window} {
        std::srand(std::time(nullptr));
        chunks.resize(WORLD_SIZE, vector<Chunk>(WORLD_SIZE));
        test_font.loadFromFile(dir_path + "Hack-Regular.ttf");
        cnt_font.loadFromFile(dir_path + "fonts/" + "Vogue Bold.ttf");

        for (auto & active_chunk : active_chunks)
            for (bool & j : active_chunk)
                j = false;

        last_update_time = std::chrono::system_clock::now();
    }

    struct Object : public sf::Sprite {
        Object(int n_x, int n_y, int n_id) :
                x_coord{n_x}, y_coord{n_y}, id{n_id} {}

        int x_coord, y_coord, id;
        json<string, string> parameters;
        int static_texture{};
        vector<int> animated_texture;

        void draw(int player_x, int player_y) {
            return;
        }
    };


    struct MovableObject : public Object {
        int speed{};
        float dx{}, dy{}, actual_x_coord, actual_y_coord;

        MovableObject(int x, int y, int id) : Object(x, y, id) {
            actual_x_coord = (float)x;
            actual_y_coord = (float)y;
        }

        void move(std::chrono::duration<float> &time_passed_) {
            actual_x_coord = actual_x_coord + (float)speed * dx * time_passed_.count();
            actual_y_coord = actual_y_coord + (float)speed * dy * time_passed_.count();
            while (actual_x_coord < 0) actual_x_coord += WORLD_PIXEL_SIZE;
            while (actual_y_coord < 0) actual_y_coord += WORLD_PIXEL_SIZE;
            while (actual_x_coord >= WORLD_PIXEL_SIZE) actual_x_coord -= WORLD_PIXEL_SIZE;
            while (actual_y_coord >= WORLD_PIXEL_SIZE) actual_y_coord -= WORLD_PIXEL_SIZE;
            x_coord = (int) actual_x_coord;
            y_coord = (int) actual_y_coord;
        }

    };


    struct Player : public MovableObject {
        Player() : MovableObject(0, 0, 0) {
            dx = 0;
            dy = 0;
            speed = 300;
            setTexture(textures[0]);
        };

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

        void draw(sf::RenderWindow *window_) {
            window_->draw(*this);
            inventory.draw(window_);
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
                    (dir_path + "chunks/" + std::to_string(x_coord) + "-" + std::to_string(y_coord) + ".chunk").c_str(),
                    std::ios::in);
            if (!test.good()) return;
            std::ofstream f(
                    (dir_path + "chunks/" + std::to_string(x_coord) + "-" + std::to_string(y_coord) + ".chunk").c_str(),
                    std::ios::out | std::ios::trunc);
            if (!f.good()) return;
            int mobs_cnt = (int)mobs.size();
            f << mobs_cnt << '\n';
            for (int i = 0; i < mobs_cnt; ++i) {
                f << mobs[i].id << ' ' << mobs[i].x_coord << ' ' << mobs[i].y_coord << '\n';
            }
            int obj_cnt = (int)objects.size();
            f << obj_cnt << '\n';
            for (int i = 0; i < obj_cnt; ++i) {
                f << objects[i].id << ' ' << objects[i].x_coord << ' ' << objects[i].y_coord << '\n';
            }
        }

        void draw(int player_x, int player_y, sf::RenderWindow *window_) {
            sf::RectangleShape shape({CHUNK_SIZE, CHUNK_SIZE});
            shape.setFillColor(color);
            int x = x_coord * CHUNK_SIZE % WORLD_PIXEL_SIZE;
            int y = y_coord * CHUNK_SIZE % WORLD_PIXEL_SIZE;
            auto xy = get_window_coords(x, y, player_x, player_y);
            x = xy.first, y = xy.second;
            shape.setPosition((float) x, (float) y);
            window_->draw(shape);
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

        static void draw_player_coords(sf::RenderWindow *window_, Player *player_) {
            sf::Text coords_text(std::to_string(player_->x_coord) + ' ' + std::to_string(player_->y_coord), test_font);
            coords_text.setPosition(10, 10);
            coords_text.setCharacterSize(18);
            window_->draw(coords_text);
        }

        void draw_fps(sf::RenderWindow *window_) {
            if (frames_time == std::time(nullptr)) ++frames_current_count;
            else {
                frames_time = std::time(nullptr);
                frames_latest_count = frames_current_count;
                frames_current_count = 0;
            }
            sf::Text fps_text(std::to_string(frames_latest_count), test_font);
            fps_text.setPosition(200, 10);
            fps_text.setCharacterSize(18);
            window_->draw(fps_text);
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
                if (player.inventory.left_click_in_inventory(event.mouseButton.x, event.mouseButton.y));
                else {
                    // обработка лкм если клик пришелся не на слоты инвентаря
                }
            } else if (event.mouseButton.button == sf::Mouse::Right) {
                if (player.inventory.right_click_in_inventory(event.mouseButton.x, event.mouseButton.y));
                else {
                    // обработка лкм если клик пришелся не на слоты инвентаря
                }
            }
        } else if (event.type == sf::Event::MouseMoved) {
            player.inventory.invisible_mouse_slot.x_pix = event.mouseMove.x;
            player.inventory.invisible_mouse_slot.y_pix = event.mouseMove.y;
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
        Debug::draw_player_coords(window, &player);
        debug.draw_fps(window);
    }

    float calculate_sleep_time() {
        float full_sleep_time = time_passed.count() * 1000, ms_pf = 1000.f /(float)max_fps;
        last_sleep_time = ms_pf + last_sleep_time - full_sleep_time;
        return last_sleep_time;
    }
};

#endif //UNTITLED4_ENGINE_H
