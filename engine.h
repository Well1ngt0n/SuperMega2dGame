//created by ООО "Разрабы Дауны"
#ifndef UNTITLED4_ENGINE_H
#define UNTITLED4_ENGINE_H

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include <bits/stdc++.h>
#include "inventory.h"

bool active_chunks[WORLD_SIZE][WORLD_SIZE];

struct Game {
    sf::RenderWindow *window;
    std::chrono::duration<float> time_passed;
    std::chrono::time_point<std::chrono::system_clock> last_update_time;
    float last_sleep_time = 10;

    Game(sf::RenderWindow *n_window) : window{n_window} {
        std::srand(std::time(nullptr));
        chunks.resize(WORLD_SIZE, vector<Chunk>(WORLD_SIZE));
        test_font.loadFromFile("../Hack-Regular.ttf");
        cnt_font.loadFromFile("../fonts/Vogue Bold.ttf");

        for (int i = 0; i < WORLD_SIZE; ++i)
            for (int j = 0; j < WORLD_SIZE; ++j)
                active_chunks[i][j] = false;

        last_update_time = std::chrono::system_clock::now();
    }

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

    struct Object : public sf::Sprite {
        Object(int n_x, int n_y, int n_id) : x_coord{n_x}, y_coord{n_y}, id{n_id} {
            std::ifstream f(("../objects/" + std::to_string(id) + ".object").c_str());
            if (!f.good()) return;
            f >> static_texture;
            int k;
            f >> k;
            animated_texture.resize(k);
            for (int i = 0; i < k; i++) f >> animated_texture[i];
            if (k != 0) {
                f >> time_animation;
                animation = true;
            }
            f.close();
        }

        int x_coord, y_coord, id, cur_texture = 0, time_animation;
        float time_from_last_animation;
        map<string, int> parameters;
        int static_texture;
        bool animation = false;
        vector<int> animated_texture;

        void draw(int player_x, int player_y, sf::RenderWindow *window) {
            auto[x_window, y_window] = get_window_coords(x_coord, y_coord, player_x, player_y);
            //std::cout << x_window << ' ' << y_window << std::endl;
            this->setPosition(x_window, y_window);
            window->draw(*this);
        }

        void update(std::chrono::duration<float> &time_passed) {
            time_from_last_animation += time_passed.count();
            if (!animation) this->setTexture(textures[static_texture]);
            else if (((float) time_animation / (float) animated_texture.size() / 1000) <= time_from_last_animation) {

                cur_texture = (cur_texture + 1) % animated_texture.size();
                this->setTexture(textures[animated_texture[cur_texture]]);
                time_from_last_animation = 0;
            }
        }
    };


    struct MovableObject : public Object {
        int speed;
        float dx, dy, actual_x_coord, actual_y_coord;

        std::map<std::pair<int, int>, int> index_from_direction = { // mp[{dx, dy}] = index
                //{{0, 0}, 0}, - may be?
                {{0,  1},  0}, // sorted by angle
                {{1,  1},  1},
                {{1,  0},  2},
                {{1,  -1}, 3},
                {{0,  -1}, 4},
                {{-1, -1}, 5}, //animations to this directions can be maked with reverse of other directions
                {{-1, 0},  6},
                {{-1, 1},  7},
        };

        vector<int> move_animation[8];

        MovableObject(int x, int y, int id) : Object(x, y, id) {
            actual_x_coord = x;
            actual_y_coord = y;
            std::ifstream f(("../movable_objects/" + std::to_string(id) + ".movable_object").c_str());
            if (!f.good()) return;
            for (auto &direction_animation: move_animation) {
                int k;
                f >> k;
                direction_animation.resize(k);
                for (auto &texture_index: direction_animation)
                    f >> texture_index;
            }
            f.close();
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
            auto[width, height] = this->getTexture()->getSize();
            setPosition(window_width / 2 - width / 2, window_height / 2 - height / 2);
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

    Player player;


    struct Mob : public MovableObject {
        vector<int> attack_animation[8];
        int stop_texture;

        Mob(int x, int y, int id) : MovableObject(x, y, id) {
            std::ifstream f(("../movable_objects/" + std::to_string(id) + ".movable_object").c_str());
            if (!f.good()) return;
            for (auto &direction_animation: attack_animation) {
                int k;
                f >> k;
                direction_animation.resize(k);
                for (auto &texture_index: direction_animation) f >> texture_index;
            }
        }


        void logic() {

        }
    };


    struct DropItem : public sf::Sprite {
        Item item;
        int cnt = 0;
        float x_coord{}, y_coord{};
        float time_from_last_animation = 0;
        char delta = 0;
        bool dir = 1;
        bool is_run = false;

        DropItem(Item& item, int cnt, int x, int y) : item(item), cnt(cnt), x_coord(x), y_coord(y) {
            this->setTexture(item_textures[item.id]);
        }

        explicit DropItem(Slot& slot) {
            this->setTexture(item_textures[slot.item.id]);
            item = slot.item;
            cnt = slot.cnt;
        }

        Slot get_slot() const {
            Slot x;
            x.item = item;
            x.cnt = cnt;
            return x;
        }

        void draw(int player_x, int player_y, sf::RenderWindow *window) {
            auto[x_window, y_window] = get_window_coords(x_coord, y_coord, player_x, player_y);
            this->setPosition(x_window, y_window-delta);
            window->draw(*this);
            if (cnt > 1) {
                this->setPosition(x_window + SLOT_PIXEL_SIZE / 8, y_window-delta+ SLOT_PIXEL_SIZE / 8);
                window->draw(*this);
            }
        }
        void update(std::chrono::duration<float> &time_passed, Player& player) {
            time_from_last_animation += time_passed.count();
            if(time_from_last_animation * 1000 >= 30){
                time_from_last_animation = 0;
                if(delta == 16 && dir) dir = 0;
                else if(delta == 0 && !dir) dir = 1;
                else{
                    delta += 2*dir-1;
                }
            }
            if(is_run){
                float dx1 = player.x_coord - x_coord;
                float dx2 = +player.x_coord - WORLD_PIXEL_SIZE - x_coord;
                float dx3 = -x_coord + WORLD_PIXEL_SIZE + player.x_coord;

                float dy1 = player.y_coord - y_coord;
                float dy2 = player.y_coord - WORLD_PIXEL_SIZE - y_coord;
                float dy3 = -y_coord + player.y_coord + WORLD_PIXEL_SIZE;

                float dx = (fabs(dx1) < fabs(dx2) ? fabs(dx3) < fabs(dx1) ? dx3 : dx1 : fabs(dx3) < fabs(dx2) ? dx3 : dx2), dy = (fabs(dy1) < fabs(dy2) ? fabs(dy3) < fabs(dy1) ? dy3 : dy1 : fabs(dy3) < fabs(dy2) ? dy3 : dy2);

                if(hypot(dx, dy) < 10){

                }

                x_coord = x_coord + 10 * dx * time_passed.count();
                y_coord = y_coord + 10 * dy * time_passed.count();
                while (x_coord < 0) x_coord += WORLD_PIXEL_SIZE;
                while (y_coord < 0) y_coord += WORLD_PIXEL_SIZE;
                while (x_coord >= WORLD_PIXEL_SIZE) x_coord -= WORLD_PIXEL_SIZE;
                while (y_coord >= WORLD_PIXEL_SIZE) y_coord -= WORLD_PIXEL_SIZE;
            }
        }
    };

    struct Chunk {
        vector<Mob> mobs;
        vector<DropItem> drop_items;//= {DropItem(Item(1, 64), 10, 200, 200)};
        vector<Object> objects;
        sf::Color color;
        int x_coord, y_coord;

        void load(int x, int y) {
            x_coord = x, y_coord = y;
            std::ifstream f(
                    (dir_path + "chunks/" + std::to_string(x_coord) + "-" + std::to_string(y_coord) +
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

            int items_cnt;
            f >> items_cnt;
            for (int i = 0; i < items_cnt; i++) {
                Slot x;
                x.load(f);
                drop_items.emplace_back(x);
                int item_x, item_y;
                f >> item_x >> item_y;
                drop_items.back().x_coord = item_x, drop_items.back().y_coord = item_y;
            }
            f.close();
        }

        void upload() {
            std::ofstream f(
                    (dir_path + "chunks/" + std::to_string(x_coord) + "-" + std::to_string(y_coord) + ".chunk").c_str(),
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

            int items_cnt = drop_items.size();
            f << items_cnt << '\n';
            for (int i = 0; i < items_cnt; i++) {
                Slot x = drop_items[i].get_slot();
                x.upload(f);
                f << drop_items[i].x_coord << ' ' << drop_items[i].y_coord << '\n';
            }
            f.close();
        }

        void draw(int player_x, int player_y, sf::RenderWindow *window) {
            sf::RectangleShape shape({CHUNK_SIZE, CHUNK_SIZE});
            shape.setFillColor(color);
            int x = x_coord * CHUNK_SIZE % WORLD_PIXEL_SIZE;
            int y = y_coord * CHUNK_SIZE % WORLD_PIXEL_SIZE;
            auto xy = get_window_coords(x, y, player_x, player_y);
            x = xy.first, y = xy.second;
            shape.setPosition(x, y);
            window->draw(shape);
        }

        void del() {
            upload();
            mobs.clear();
            mobs.shrink_to_fit();
            drop_items.clear();
            drop_items.shrink_to_fit();
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
                case sf::Keyboard::Space:
                    {
                        int x = player.x_coord / CHUNK_SIZE, y = player.y_coord / CHUNK_SIZE;
                        for (int i = x - 1; i <= x + 1; i++) {
                            for (int j = y - 1; j <= y + 1; j++) {
                                int nx = (i < 0 ? i + WORLD_SIZE : i >= WORLD_SIZE ? i - WORLD_SIZE:i);
                                int ny = (j < 0 ? j + WORLD_SIZE : j >= WORLD_SIZE ? j - WORLD_SIZE:j);
                                for (auto &item: chunks[nx][ny].drop_items) {
                                    if (hypot(item.x_coord - player.x_coord, item.y_coord - player.y_coord) <= 128) {
                                        item.is_run = true;
                                    }
                                }
                            }
                        }
                    }
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

    template<class T>
    static bool cmp_objects_for_rendering(const T &a, const T &b) {
        return a.y_coord > b.y_coord;
    }

    void draw() {
        vector<Object> pool_objects;
        vector<Mob> pool_mobs;
        vector<DropItem> pool_items;
        int dx[4] = {-1, 1, 1, -1};
        int dy[4] = {1, 1, -1, -1};
        int x = player.x_coord / CHUNK_SIZE, y = player.y_coord / CHUNK_SIZE;

        for (int d = 0; d <= MAX_RENDER_DISTANCE + 1; d++) {
            for (int i_ = 0; i_ < (d==0?1:d); ++i_) {
                for (int k = 0; k < (d==0?1:4); k++) {
                    int i = i_ + (k&1);
                    int j = d - i;
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
                        for (auto &mob: chunks[nx][ny].mobs) {
                            //mob.logic();
                            //mob.move();
                            pool_mobs.emplace_back(mob);
                        }
                       for (auto &item: chunks[nx][ny].drop_items) {
                            item.update(time_passed, player);
                            pool_items.push_back(item);
                        }
                        for (auto &object: chunks[nx][ny].objects) {
                            object.update(time_passed);
                            pool_objects.emplace_back(object);
                        }

                    }
                }
            }
        }
        sort(pool_objects.begin(), pool_objects.end(), cmp_objects_for_rendering<Object>);
        sort(pool_mobs.begin(), pool_mobs.end(), cmp_objects_for_rendering<Object>);
        sort(pool_items.begin(), pool_items.end(), cmp_objects_for_rendering<DropItem>);
        bool player_drawed = false;
        while (!pool_objects.empty() || !pool_mobs.empty() || !pool_items.empty() || !player_drawed) {
            int y_cur_object = (pool_objects.empty() ? WORLD_PIXEL_SIZE : pool_objects.back().y_coord);
            int y_cur_mob = (pool_mobs.empty() ? WORLD_PIXEL_SIZE : pool_mobs.back().y_coord);
            int y_cur_item = (pool_items.empty() ? WORLD_PIXEL_SIZE : pool_items.back().y_coord);
            int min_y_coord = std::min({y_cur_object, y_cur_item, y_cur_mob});
            if (!player_drawed && player.y_coord < min_y_coord) {
                player_drawed = true;
                player.draw(window);
            } else if (y_cur_object == min_y_coord) {
                pool_objects.back().draw(player.x_coord, player.y_coord, window);
                pool_objects.pop_back();
            } else if (y_cur_mob == min_y_coord) {
                pool_mobs.back().draw(player.x_coord, player.y_coord, window);
                pool_mobs.pop_back();
            } else {
                pool_items.back().draw(player.x_coord, player.y_coord, window);
                pool_items.pop_back();
            }
        }

        debug.draw_player_coords(window, &player);
        debug.draw_fps(window);
    }

    float calculate_sleep_time() {
        float full_sleep_time = time_passed.count() * 1000, ms_pf = 1000.f / (float) max_fps;
        last_sleep_time = ms_pf + last_sleep_time - full_sleep_time;
        return last_sleep_time;
    }

    void exit() {
        player.inventory.upload();
    }
};

#endif //UNTITLED4_ENGINE_H
