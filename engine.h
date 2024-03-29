//created by ООО "Разрабы Дауны"
#ifndef UNTITLED4_ENGINE_H
#define UNTITLED4_ENGINE_H

#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include <bits/stdc++.h>
#include "inventory.h"
#include "init.h"

bool active_chunks[WORLD_SIZE][WORLD_SIZE];

struct Game {
    sf::RenderWindow *window;
    std::chrono::duration<float> time_passed;
    std::chrono::time_point<std::chrono::system_clock> last_update_time;
    float last_sleep_time = 10;

    Game(sf::RenderWindow *n_window) : window{n_window} {
        std::srand(std::time(nullptr));
        chunks.resize(WORLD_SIZE, vector<Chunk>(WORLD_SIZE));

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
            speed = default_speed = 300;
            health_regeneration = default_health_regeneration;
            stamina_regeneration = default_stamina_regeneration;
            max_health_points = max_stamina_points = 1000;
            health_points = stamina_points = 1;

            stamina_sprite.setTexture(texture_stamina_health);
            stamina_sprite.setPosition(0, 40);
            health_sprite.setTexture(texture_stamina_health);
            health_sprite.setPosition(0, 0);
            setTexture(textures[0]);
        };

        void move(std::chrono::duration<float> &time_passed){
            speed = default_speed;
            if (is_run) speed = default_speed * run_speed;
            if (is_rush) speed = default_speed * rush_speed;
            if (objects_run) speed = default_speed * slow_speed;
            MovableObject::move(time_passed);
        }

        void update(std::chrono::duration<float> &time_passed) {
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

            time_from_last_damage = std::min(time_from_last_damage + time_passed.count(), 4.0);
            time_from_last_regeneration_health += time_passed.count();
            if (time_from_last_damage == 4.0) {
                health_points = std::min(health_points + health_regeneration, max_health_points);
                //time_from_last_regeneration_health = 0;
            }

            time_from_last_stamina_waste = std::min(time_from_last_stamina_waste + time_passed.count(), 2.0);
            time_from_last_regeneration_stamina += time_passed.count();
            if (time_from_last_stamina_waste == 2.0) {
                stamina_points = std::min(stamina_points + stamina_regeneration, max_stamina_points);
                //time_from_last_regeneration_health = 0;
            }
            time_from_last_rush = std::min(time_from_last_rush + time_passed.count(), time_between_rushs);
            if (is_rush && (dx != 0 || dy != 0)){
                time_from_last_stamina_waste = 0;
                time_from_last_rush = 0;
                stamina_points = std::max(stamina_points - rush_cost, 0.0);
                rush_time += time_passed.count();
                if (rush_time >= all_rush_time) {
                    is_rush = false;
                    rush_time = 0;
                }
                if (stamina_points == 0){
                    is_rush = is_run = false;
                    rush_time = 0;
                }
            } else if (is_run && (dx != 0 || dy != 0)){
                time_from_last_stamina_waste = 0;
                stamina_points = std::max(stamina_points - run_cost, 0.0);
                if (stamina_points == 0) is_run = false;
            }
            if (is_rush && dx == 0 && dy == 0){
                is_rush = false;
            }
        }

        void draw(sf::RenderWindow *window_) {
            window_->draw(*this);

            sf::RectangleShape health_rect(sf::Vector2f(600.0 * ((float)health_points / (float)max_health_points), 32));
            sf::RectangleShape stamina_rect(sf::Vector2f(600.0 * ((float)stamina_points / (float)max_stamina_points), 32));
            health_rect.setFillColor(sf::Color(255, 0, 0));
            health_rect.setPosition(0, 0);
            stamina_rect.setPosition(0, 40);
            window_->draw(health_rect);
            window_->draw(stamina_rect);

            window_->draw(health_sprite);
            window_->draw(stamina_sprite);
            inventory.draw(window_);
        }

        void damage(int damage_points) {
            health_points = std::max(0.0, health_points - damage_points);
            time_from_last_damage = 0;
            if (health_points == 0) {
                //TODO: сделать интерфейс
            }
        }

        Inventory inventory;

        bool d_right = false, d_left = false, d_up = false, d_down = false;

        int objects_run = 0;
        int walk = 0;
        int attack = 0;
        bool is_run = false;
        bool is_rush = false;
        int default_speed;
        int run_speed = 2;
        int rush_speed = 11;
        double slow_speed = 0.5;
        double max_health_points, max_stamina_points;
        double health_points, stamina_points;
        double default_health_regeneration = 1, default_stamina_regeneration = 3;
        double health_regeneration, stamina_regeneration;
        double time_from_last_damage = 4.0, time_from_last_stamina_waste = 2.0,
        time_from_last_regeneration_health = 0, time_from_last_regeneration_stamina = 0,
        rush_time = 0, all_rush_time = 0.1, time_between_rushs = 0.2, time_from_last_rush = 0;
        double rush_cost = 7, run_cost = 1;
        sf::Sprite stamina_sprite, health_sprite;
    };

    Player player;

    void drop_slot(Slot &slot, int x, int y, bool cnt = 0) {
        if (slot.item.id == -1) return;
        int cx = x / CHUNK_SIZE, cy = y / CHUNK_SIZE;
        bool f = 0;
        if (slot.item.max_stack_size != 1) {
            for (int i = cx - 1; i <= cx + 1 && !f; i++) {
                for (int j = cy - 1; j <= cy + 1 && !f; j++) {
                    int nx = (i < 0 ? i + WORLD_SIZE : i >= WORLD_SIZE ? i - WORLD_SIZE : i);
                    int ny = (j < 0 ? j + WORLD_SIZE : j >= WORLD_SIZE ? j - WORLD_SIZE : j);
                    for (auto &item: chunks[nx][ny].drop_items) {
                        if (item.item.id != slot.item.id) continue;
                        int dx1 = x - item.x_coord;
                        int dx2 = x - WORLD_PIXEL_SIZE - item.x_coord;
                        int dx3 = -item.x_coord + WORLD_PIXEL_SIZE + x;

                        int dy1 = y - item.y_coord;
                        int dy2 = y - WORLD_PIXEL_SIZE - item.y_coord;
                        int dy3 = -item.y_coord + y + WORLD_PIXEL_SIZE;

                        int dx = (abs(dx1) < abs(dx2) ? abs(dx3) < abs(dx1) ? dx3 : dx1 : abs(dx3) <
                                                                                          abs(dx2) ? dx3
                                                                                                   : dx2), dy = (
                                abs(dy1) < abs(dy2) ? abs(dy3) < abs(dy1) ? dy3 : dy1 : abs(dy3) < abs(dy2)
                                                                                        ? dy3 : dy2);

                        if (hypot(dx, dy) <= 48) {
                            f = 1;
                            if (cnt) {
                                item.cnt += 1;
                                slot.cnt--;
                                if (slot.cnt == 0) {
                                    slot.item = Item();
                                }
                            } else {
                                item.cnt += slot.cnt;
                                slot.item = Item();
                                slot.cnt = 0;
                            }
                            break;
                        }
                    }
                }
            }
        }
        if (!f) {
            auto item = DropItem(slot);
            if (cnt) {
                item.cnt = 1;
                slot.cnt--;
                if (slot.cnt == 0) {
                    slot.item = Item();
                }
            } else {
                slot.item = Item();
                slot.cnt = 0;
            }
            item.x_coord = x;
            item.y_coord = y;
            chunks[x / CHUNK_SIZE][y / CHUNK_SIZE].drop_items.push_back(item);
        }
    }

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
        int x_coord{}, y_coord{};
        float time_from_last_animation = 0;
        char delta = 0;
        bool dir = 1;
        bool is_run = false;

        DropItem(Item &item, int cnt, int x, int y) : item(item), cnt(cnt), x_coord(x), y_coord(y) {
            this->setTexture(item_textures[item.id]);
        }

        explicit DropItem(Slot &slot) {
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
            this->setPosition(x_window - SLOT_PIXEL_SIZE / 2, y_window - delta);
            window->draw(*this);
            if (cnt > 1) {
                this->setPosition(x_window + SLOT_PIXEL_SIZE / 8 - SLOT_PIXEL_SIZE / 2,
                                  y_window - delta + SLOT_PIXEL_SIZE / 8);
                window->draw(*this);
            }
        }

        int update(std::chrono::duration<float> &time_passed, Player &player) {
            time_from_last_animation += time_passed.count();
            if (time_from_last_animation * 1000 >= 30) {
                time_from_last_animation = 0;
                if (delta == 16 && dir) dir = 0;
                else if (delta == 0 && !dir) dir = 1;
                else {
                    delta += 2 * dir - 1;
                }
            }
            if (is_run) {
                int dx1 = player.x_coord - x_coord;
                int dx2 = +player.x_coord - WORLD_PIXEL_SIZE - x_coord;
                int dx3 = -x_coord + WORLD_PIXEL_SIZE + player.x_coord;

                int dy1 = player.y_coord - y_coord;
                int dy2 = player.y_coord - WORLD_PIXEL_SIZE - y_coord;
                int dy3 = -y_coord + player.y_coord + WORLD_PIXEL_SIZE;

                int dx = (abs(dx1) < abs(dx2) ? abs(dx3) < abs(dx1) ? dx3 : dx1 : abs(dx3) < abs(dx2) ? dx3
                                                                                                      : dx2), dy = (
                        abs(dy1) < abs(dy2) ? abs(dy3) < abs(dy1) ? dy3 : dy1 : abs(dy3) < abs(dy2) ? dy3 : dy2);

                if (hypot(dx, dy) < 48) {
                    return 1;
                }

                int prx = x_coord, pry = y_coord;

                x_coord = x_coord + 10 * dx * std::min(0.1f, time_passed.count());
                y_coord = y_coord + 10 * dy * std::min(0.1f, time_passed.count());
                while (x_coord < 0) x_coord += WORLD_PIXEL_SIZE;
                while (y_coord < 0) y_coord += WORLD_PIXEL_SIZE;
                while (x_coord >= WORLD_PIXEL_SIZE) x_coord -= WORLD_PIXEL_SIZE;
                while (y_coord >= WORLD_PIXEL_SIZE) y_coord -= WORLD_PIXEL_SIZE;

                if (prx / CHUNK_SIZE != x_coord / CHUNK_SIZE || pry / CHUNK_SIZE != y_coord / CHUNK_SIZE) {
                    return 2; //выписать из старого чанка, азгрузить в новый
                }
            }
            return 0;
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
            if (mobs.size() + objects.size() + drop_items.size() == 0) {
                remove((dir_path + "chunks/" + std::to_string(x_coord) + "-" + std::to_string(y_coord) +
                        ".chunk").c_str());
                return;
            }
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
                case sf::Keyboard::C: {
                    int x = player.x_coord / CHUNK_SIZE, y = player.y_coord / CHUNK_SIZE;
                    for (int i = x - 1; i <= x + 1; i++) {
                        for (int j = y - 1; j <= y + 1; j++) {
                            int nx = (i < 0 ? i + WORLD_SIZE : i >= WORLD_SIZE ? i - WORLD_SIZE : i);
                            int ny = (j < 0 ? j + WORLD_SIZE : j >= WORLD_SIZE ? j - WORLD_SIZE : j);
                            for (auto &item: chunks[nx][ny].drop_items) {
                                int dx1 = player.x_coord - item.x_coord;
                                int dx2 = +player.x_coord - WORLD_PIXEL_SIZE - item.x_coord;
                                int dx3 = -item.x_coord + WORLD_PIXEL_SIZE + player.x_coord;

                                int dy1 = player.y_coord - item.y_coord;
                                int dy2 = player.y_coord - WORLD_PIXEL_SIZE - item.y_coord;
                                int dy3 = -item.y_coord + player.y_coord + WORLD_PIXEL_SIZE;

                                int dx = (abs(dx1) < abs(dx2) ? abs(dx3) < abs(dx1) ? dx3 : dx1 : abs(dx3) < abs(dx2) ? dx3 : dx2);
                                int dy = (abs(dy1) < abs(dy2) ? abs(dy3) < abs(dy1) ? dy3 : dy1 : abs(dy3) < abs(dy2) ? dy3 : dy2);

                                if (hypot(dx, dy) <= 128) {
                                    player.objects_run += 1 ^ item.is_run;
                                    item.is_run = true;
                                    player.speed = player.default_speed * player.slow_speed;
                                    if (player.is_run)
                                        player.is_run = false;
                                }
                            }
                        }
                    }
                }
                    break;
                case sf::Keyboard::LShift:
                    if (!player.objects_run && !player.is_rush) {
                        player.is_run = true;
                    }
                    break;
                case sf::Keyboard::Space:
                    if (!player.objects_run && player.time_from_last_rush == player.time_between_rushs){
                        player.is_rush = true;
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
                case sf::Keyboard::LShift:
                    player.is_run = false;
                    break;
                case sf::Keyboard::Space:
                    player.is_rush = false;
                default:
                    break;
            }
        } else if (event.type == sf::Event::MouseButtonPressed) {
            if (event.mouseButton.button == sf::Mouse::Left) {
                if (player.inventory.left_click_in_inventory(event.mouseButton.x, event.mouseButton.y));
                else if (player.inventory.invisible_mouse_slot.item.id != -1) {
                    int dx = event.mouseButton.x - window_width / 2, dy = event.mouseButton.y - window_height / 2;
                    double x, y;
                    if (hypot(dx, dy) < radius) {
                        x = dx, y = dy;
                    } else {
                        double alpha = (double) dy / dx;
                        x = radius / sqrt(1 + alpha * alpha);
                        if (dx < 0) x = -x;
                        y = x * alpha;
                    }
                    drop_slot(player.inventory.invisible_mouse_slot,
                              (player.x_coord + (int) x + WORLD_PIXEL_SIZE) % WORLD_PIXEL_SIZE,
                              (player.y_coord + (int) y + WORLD_PIXEL_SIZE) % WORLD_PIXEL_SIZE);
                }
            } else if (event.mouseButton.button == sf::Mouse::Right) {
                if (player.inventory.right_click_in_inventory(event.mouseButton.x, event.mouseButton.y));
                else if (player.inventory.invisible_mouse_slot.item.id != -1) {
                    int dx = event.mouseButton.x - window_width / 2, dy = event.mouseButton.y - window_height / 2;
                    double x, y;
                    if (hypot(dx, dy) < radius) {
                        x = dx, y = dy;
                    } else {
                        double alpha = (double) dy / dx;
                        x = radius / sqrt(1 + alpha * alpha);
                        if (dx < 0) x = -x;
                        y = x * alpha;
                    }
                    drop_slot(player.inventory.invisible_mouse_slot,
                              (player.x_coord + (int) x + WORLD_PIXEL_SIZE) % WORLD_PIXEL_SIZE,
                              (player.y_coord + (int) y + WORLD_PIXEL_SIZE) % WORLD_PIXEL_SIZE, 1);
                }
            }
        } else if (event.type == sf::Event::MouseMoved) {
            player.inventory.invisible_mouse_slot.x_pix = event.mouseMove.x;
            player.inventory.invisible_mouse_slot.y_pix = event.mouseMove.y;
        }
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
        player.update(time_passed);
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
            for (int i_ = 0; i_ < (d == 0 ? 1 : d); ++i_) {
                for (int k = 0; k < (d == 0 ? 1 : 4); k++) {
                    int i = i_ + (k & 1);
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
                        for (auto it = chunks[nx][ny].drop_items.begin(); it != chunks[nx][ny].drop_items.end();) {
                            int kok = it->update(time_passed, player);
                            pool_items.push_back(*it);
                            if (kok == 2) {
                                chunks[it->x_coord / CHUNK_SIZE][it->y_coord / CHUNK_SIZE].drop_items.push_back(*it);
                                std::cout << "kok" << std::endl;
                                it = chunks[nx][ny].drop_items.erase(it);
                                continue;
                            } else if (kok == 1) {
                                auto slot = it->get_slot();
                                player.inventory.add(slot);
                                if (slot.item.id == -1) {
                                    it = chunks[nx][ny].drop_items.erase(it);
                                    //std::cout << player.objects_run - 1 << std::endl;
                                    player.objects_run--;
                                    /*if (!(--player.objects_run)) {
                                        player.speed = player.default_speed;
                                    }*/
                                    continue;
                                } else {
                                    it->is_run = false;
                                }
                                player.objects_run--;
                                //std::cout << player.objects_run - 1 << std::endl;
                                /*if (!(--player.objects_run)) {
                                    player.speed = player.default_speed;
                                }*/
                            }
                            it++;
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
        for (int i = 0; i < WORLD_SIZE; i++) {
            for (int j = 0; j < WORLD_SIZE; j++) {
                if (active_chunks[i][j]) {
                    chunks[i][j].del();
                }
            }
        }
        // выгрузка всех чанков при выходе
    }
};

#endif //UNTITLED4_ENGINE_H
