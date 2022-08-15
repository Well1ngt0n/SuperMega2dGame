#ifndef SUPERMEGA2DGAME_INVENTORY_H
#define SUPERMEGA2DGAME_INVENTORY_H


#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include <bits/stdc++.h>
#include "init.h"

using std::string, std::vector, std::map;

struct Item {
    int id;
    int max_stack_size;
    map<string, int> params; //string has no spaces

    Item() : id(-1), max_stack_size(64) {}
    Item(int id, int mss) : id(id), max_stack_size(mss) {}
};

struct Slot {
    int x_pix{}, y_pix{};
    bool is_active = false;
    Item item;
    int cnt = 0;

    [[nodiscard]] bool is_pressed(int, int) const;

    void draw(sf::RenderWindow *, bool) const;

    void upload(std::ofstream &);

    void load(std::ifstream &);

    void swap_item(Slot &other);
};


struct Inventory {

    Slot invisible_mouse_slot;
    Slot left_hand, right_hand;
    Slot fast_pack[FAST_PACK_SIZE];
    Slot cock_pack[PACK_HEIGHT][PACK_WIDTH];
    Slot armor[4];
    std::pair<int, int> active_slot = {0, 0}; // default nums
    bool is_cock_pack_open = false;
    bool is_item_in_mouse = false;

    Inventory();

    void left_swap();

    void right_swap();

    void swap_hands();

    void draw(sf::RenderWindow *);

    Slot &get_active_slot();

    int left_click_in_inventory(int, int);

    int right_click_in_inventory(int, int);

    void upload();

    void load();

    void add(Slot&);
};

bool Slot::is_pressed(int x_mouse, int y_mouse) const {
    return (x_mouse >= x_pix && x_mouse < x_pix + SLOT_PIXEL_SIZE) &&
           (y_mouse >= y_pix && y_mouse < y_pix + SLOT_PIXEL_SIZE);
}

void Slot::draw(sf::RenderWindow *window, bool draw_slot_icon = true) const {
    if (is_active) {
        active_slot_sprite.setPosition((float) x_pix, (float) y_pix);
        window->draw(active_slot_sprite);
    } else if (draw_slot_icon) {
        default_slot_sprite.setPosition((float) x_pix, (float) y_pix);
        window->draw(default_slot_sprite);
    }
    if (item.id != -1) {
        sf::Sprite item_sprite;
        item_sprite.setTexture(item_textures[item.id]);
        item_sprite.setPosition((float) (x_pix - (draw_slot_icon ? 0 : SLOT_PIXEL_SIZE * 3 / 4)),
                                (float) (y_pix - (draw_slot_icon ? 0 : SLOT_PIXEL_SIZE * 3 / 4)));
        window->draw(item_sprite);
        if (item.max_stack_size != 1) {
            sf::Text text_count(std::to_string(cnt), cnt_font);
            text_count.setCharacterSize(SLOT_PIXEL_SIZE / 2);
            text_count.setFillColor(sf::Color(0, 0, 0));
            text_count.setPosition(
                    (float) (x_pix + SLOT_PIXEL_SIZE / 2. - (draw_slot_icon ? 0 : SLOT_PIXEL_SIZE * 3 / 4)),
                    (float) (y_pix + SLOT_PIXEL_SIZE * 3. / 7 -
                             (draw_slot_icon ? 0 : SLOT_PIXEL_SIZE * 3 / 4)));
            window->draw(text_count);
        }
    }
}



void Slot::swap_item(Slot &other) {
    std::swap(other.item, item);
    std::swap(other.cnt, cnt);
}

void Slot::load(std::ifstream &in) {
    in >> item.id;
    if (item.id == -1) return;
    in >> item.max_stack_size >> cnt;
    int sz_params;
    in >> sz_params;
    for (int i = 0; i < sz_params; i++) {
        string name;
        int val;
        in >> name >> val;
        item.params.insert(item.params.end(), {name, val});
    }
}

void Slot::upload(std::ofstream &out) {
    if (item.id == -1) {
        out << -1 << '\n';
        return;
    } else {
        out << item.id << ' ' << item.max_stack_size << ' ' << cnt << ' ' << item.params.size() << ' ';
        for (auto &i: item.params) out << i.first << ' ' << i.second << ' ';
        out << '\n';
    }
}

int Inventory::right_click_in_inventory(int x, int y) {
    if (left_hand.is_pressed(x, y)) {
        if (is_item_in_mouse) {
            if (left_hand.item.id == invisible_mouse_slot.item.id &&
                left_hand.item.max_stack_size != 1) {
                int &cnt1 = left_hand.cnt, &cnt2 = invisible_mouse_slot.cnt;
                int mx = left_hand.item.max_stack_size;
                if (cnt1 + 1 <= mx) {
                    cnt1 += 1;
                    cnt2 -= 1;
                    if (cnt2 == 0) {
                        invisible_mouse_slot.item = Item();
                        is_item_in_mouse = false;
                    }
                } // else do nothing
            } else if (left_hand.item.id == -1) {
                left_hand.item = invisible_mouse_slot.item;
                left_hand.cnt = 1;
                invisible_mouse_slot.cnt -= 1;
                if (invisible_mouse_slot.cnt == 0) {
                    invisible_mouse_slot.item = Item();
                    is_item_in_mouse = false;
                }
            }
        } else if (left_hand.item.id != -1) {
            int &cnt = left_hand.cnt;
            int to_mouse = (cnt + 1) / 2;
            cnt -= to_mouse;
            invisible_mouse_slot.item = left_hand.item;
            invisible_mouse_slot.cnt = to_mouse;
            is_item_in_mouse = true;
            if (cnt == 0) {
                left_hand.item = Item();
            }
        }
        return 1;
    }
    if (right_hand.is_pressed(x, y)) {
        if (is_item_in_mouse) {
            if (right_hand.item.id == invisible_mouse_slot.item.id &&
                right_hand.item.max_stack_size != 1) {
                int &cnt1 = right_hand.cnt, &cnt2 = invisible_mouse_slot.cnt;
                int mx = right_hand.item.max_stack_size;
                if (cnt1 + 1 <= mx) {
                    cnt1 += 1;
                    cnt2 -= 1;
                    if (cnt2 == 0) {
                        invisible_mouse_slot.item = Item();
                        is_item_in_mouse = false;
                    }
                } // else do nothing
            } else if (right_hand.item.id == -1) {
                right_hand.item = invisible_mouse_slot.item;
                right_hand.cnt = 1;
                invisible_mouse_slot.cnt -= 1;
                if (invisible_mouse_slot.cnt == 0) {
                    invisible_mouse_slot.item = Item();
                    is_item_in_mouse = false;
                }
            }
        } else if (right_hand.item.id != -1) {
            int &cnt = right_hand.cnt;
            int to_mouse = (cnt + 1) / 2;
            cnt -= to_mouse;
            invisible_mouse_slot.item = right_hand.item;
            invisible_mouse_slot.cnt = to_mouse;
            is_item_in_mouse = true;
            if (cnt == 0) {
                right_hand.item = Item();
            }
        }
        return 2;
    }

    bool flag = false;
    for (int i = 0; i < 4; i++) {
        if (armor[i].is_pressed(x, y)) {
            flag = true;
            if (is_item_in_mouse) {
                if (armor[i].item.id == invisible_mouse_slot.item.id &&
                    armor[i].item.max_stack_size != 1) {
                    int &cnt1 = armor[i].cnt, &cnt2 = invisible_mouse_slot.cnt;
                    int mx = armor[i].item.max_stack_size;
                    if (cnt1 + 1 <= mx) {
                        cnt1 += 1;
                        cnt2 -= 1;
                        if (cnt2 == 0) {
                            invisible_mouse_slot.item = Item();
                            is_item_in_mouse = false;
                        }
                    } // else do nothing
                } else if (armor[i].item.id == -1) {
                    armor[i].item = invisible_mouse_slot.item;
                    armor[i].cnt = 1;
                    invisible_mouse_slot.cnt -= 1;
                    if (invisible_mouse_slot.cnt == 0) {
                        invisible_mouse_slot.item = Item();
                        is_item_in_mouse = false;
                    }
                }
            } else if (armor[i].item.id != -1 && armor[i].is_active) {
                int &cnt = armor[i].cnt;
                int to_mouse = (cnt + 1) / 2;
                cnt -= to_mouse;
                invisible_mouse_slot.item = armor[i].item;
                invisible_mouse_slot.cnt = to_mouse;
                is_item_in_mouse = true;
                if (cnt == 0) {
                    armor[i].item = Item();
                }
            }
            get_active_slot().is_active = false;
            armor[i].is_active = true;
            active_slot = {2, i};
            break;
        }
    }
    if (flag) return 3;
    for (int i = 0; i < FAST_PACK_SIZE; i++) {
        if (fast_pack[i].is_pressed(x, y)) {
            flag = true;
            if (is_item_in_mouse) {
                if (fast_pack[i].item.id == invisible_mouse_slot.item.id &&
                    fast_pack[i].item.max_stack_size != 1) {
                    int &cnt1 = fast_pack[i].cnt, &cnt2 = invisible_mouse_slot.cnt;
                    int mx = fast_pack[i].item.max_stack_size;
                    if (cnt1 + 1 <= mx) {
                        cnt1 += 1;
                        cnt2 -= 1;
                        if (cnt2 == 0) {
                            invisible_mouse_slot.item = Item();
                            is_item_in_mouse = false;
                        }
                    } // else do nothing
                } else if (fast_pack[i].item.id == -1) {
                    fast_pack[i].item = invisible_mouse_slot.item;
                    fast_pack[i].cnt = 1;
                    invisible_mouse_slot.cnt -= 1;
                    if (invisible_mouse_slot.cnt == 0) {
                        invisible_mouse_slot.item = Item();
                        is_item_in_mouse = false;
                    }
                }
            } else if (fast_pack[i].item.id != -1 && fast_pack[i].is_active) {
                int &cnt = fast_pack[i].cnt;
                int to_mouse = (cnt + 1) / 2;
                cnt -= to_mouse;
                invisible_mouse_slot.item = fast_pack[i].item;
                invisible_mouse_slot.cnt = to_mouse;
                is_item_in_mouse = true;
                if (cnt == 0) {
                    fast_pack[i].item = Item();
                }
            }
            get_active_slot().is_active = false;
            fast_pack[i].is_active = true;
            active_slot = {0, i};
            break;
        }
    }
    if (flag) return 4;
    if (is_cock_pack_open) {
        for (int i = 0; i < PACK_HEIGHT; i++) {
            for (int j = 0; j < PACK_WIDTH; j++) {
                if (cock_pack[i][j].is_pressed(x, y)) {
                    flag = true;
                    if (is_item_in_mouse) {
                        if (cock_pack[i][j].item.id == invisible_mouse_slot.item.id &&
                            cock_pack[i][j].item.max_stack_size != 1) {
                            int &cnt1 = cock_pack[i][j].cnt, &cnt2 = invisible_mouse_slot.cnt;
                            int mx = cock_pack[i][j].item.max_stack_size;
                            if (cnt1 + 1 <= mx) {
                                cnt1 += 1;
                                cnt2 -= 1;
                                if (cnt2 == 0) {
                                    invisible_mouse_slot.item = Item();
                                    is_item_in_mouse = false;
                                }
                            } // else do nothing
                        } else if (cock_pack[i][j].item.id == -1) {
                            cock_pack[i][j].item = invisible_mouse_slot.item;
                            cock_pack[i][j].cnt = 1;
                            invisible_mouse_slot.cnt -= 1;
                            if (invisible_mouse_slot.cnt == 0) {
                                invisible_mouse_slot.item = Item();
                                is_item_in_mouse = false;
                            }
                        }
                    } else if (cock_pack[i][j].item.id != -1 && cock_pack[i][j].is_active) {
                        int &cnt = cock_pack[i][j].cnt;
                        int to_mouse = (cnt + 1) / 2;
                        cnt -= to_mouse;
                        invisible_mouse_slot.item = cock_pack[i][j].item;
                        invisible_mouse_slot.cnt = to_mouse;
                        is_item_in_mouse = true;
                        if (cnt == 0) {
                            cock_pack[i][j].item = Item();
                        }
                    }
                    get_active_slot().is_active = false;
                    cock_pack[i][j].is_active = true;
                    active_slot = {1, i * PACK_WIDTH + j};
                    break;
                }
            }
        }
    }
    if (flag) return 5;
    return 0;
}

int Inventory::left_click_in_inventory(int x, int y) {
    if (left_hand.is_pressed(x, y)) {
        if (is_item_in_mouse) {
            if (left_hand.item.id == invisible_mouse_slot.item.id &&
                left_hand.item.max_stack_size != 1) {
                int &cnt1 = left_hand.cnt, &cnt2 = invisible_mouse_slot.cnt;
                int mx = left_hand.item.max_stack_size;
                if (cnt1 + cnt2 <= mx) {
                    cnt1 += cnt2;
                    invisible_mouse_slot.item = Item();
                    is_item_in_mouse = false;
                } else {
                    cnt2 = cnt1 + cnt2 - mx;
                    cnt1 = mx;
                }
            } else {
                is_item_in_mouse = (left_hand.item.id != -1);
                invisible_mouse_slot.swap_item(left_hand);
            }
        } else {
            is_item_in_mouse = (left_hand.item.id != -1);
            invisible_mouse_slot.swap_item(left_hand);
        }
        return 1;
    }
    if (right_hand.is_pressed(x, y)) {
        if (is_item_in_mouse) {
            if (right_hand.item.id == invisible_mouse_slot.item.id &&
                right_hand.item.max_stack_size != 1) {
                int &cnt1 = right_hand.cnt, &cnt2 = invisible_mouse_slot.cnt;
                int mx = right_hand.item.max_stack_size;
                if (cnt1 + cnt2 <= mx) {
                    cnt1 += cnt2;
                    invisible_mouse_slot.item = Item();
                    is_item_in_mouse = false;
                } else {
                    cnt2 = cnt1 + cnt2 - mx;
                    cnt1 = mx;
                }
            } else {
                is_item_in_mouse = (right_hand.item.id != -1);
                invisible_mouse_slot.swap_item(right_hand);
            }
        } else {
            is_item_in_mouse = (right_hand.item.id != -1);
            invisible_mouse_slot.swap_item(right_hand);
        }
        return 2;
    }
    bool flag = false;
    for (int i = 0; i < 4; i++) {
        if (armor[i].is_pressed(x, y)) {
            flag = true;
            if (!is_item_in_mouse) {
                if (armor[i].is_active) {
                    is_item_in_mouse = true;
                    invisible_mouse_slot.swap_item(armor[i]);
                }
            } else {
                if (armor[i].item.id == invisible_mouse_slot.item.id &&
                    armor[i].item.max_stack_size != 1) {
                    int &cnt1 = armor[i].cnt, &cnt2 = invisible_mouse_slot.cnt;
                    int mx = armor[i].item.max_stack_size;
                    if (cnt1 + cnt2 <= mx) {
                        cnt1 += cnt2;
                        invisible_mouse_slot.item = Item();
                        is_item_in_mouse = false;
                    } else {
                        cnt2 = cnt1 + cnt2 - mx;
                        cnt1 = mx;
                    }
                } else {
                    is_item_in_mouse = (armor[i].item.id != -1);
                    invisible_mouse_slot.swap_item(armor[i]);
                }
            }
            get_active_slot().is_active = false;
            armor[i].is_active = true;
            active_slot = {2, i};
            break;
        }
    }
    if (flag) return 3;
    for (int i = 0; i < FAST_PACK_SIZE; i++) {
        if (fast_pack[i].is_pressed(x, y)) {
            flag = true;
            if (!is_item_in_mouse) {
                if (fast_pack[i].is_active) {
                    is_item_in_mouse = true;
                    invisible_mouse_slot.swap_item(fast_pack[i]);
                }
            } else {
                if (fast_pack[i].item.id == invisible_mouse_slot.item.id &&
                    fast_pack[i].item.max_stack_size != 1) {
                    int &cnt1 = fast_pack[i].cnt, &cnt2 = invisible_mouse_slot.cnt;
                    int mx = fast_pack[i].item.max_stack_size;
                    if (cnt1 + cnt2 <= mx) {
                        cnt1 += cnt2;
                        invisible_mouse_slot.item = Item();
                        is_item_in_mouse = false;
                    } else {
                        cnt2 = cnt1 + cnt2 - mx;
                        cnt1 = mx;
                    }
                } else {
                    is_item_in_mouse = (fast_pack[i].item.id != -1);
                    invisible_mouse_slot.swap_item(fast_pack[i]);
                }
            }
            get_active_slot().is_active = false;
            fast_pack[i].is_active = true;
            active_slot = {0, i};
            break;
        }
    }
    if (flag) return 4;
    if (is_cock_pack_open) {
        for (int i = 0; i < PACK_HEIGHT; i++) {
            for (int j = 0; j < PACK_WIDTH; j++) {
                if (cock_pack[i][j].is_pressed(x, y)) {
                    flag = true;
                    if (!is_item_in_mouse) {
                        if (cock_pack[i][j].is_active) {
                            is_item_in_mouse = true;
                            invisible_mouse_slot.swap_item(cock_pack[i][j]);
                        }
                    } else {
                        if (cock_pack[i][j].item.id == invisible_mouse_slot.item.id &&
                            cock_pack[i][j].item.max_stack_size != 1) {
                            int &cnt1 = cock_pack[i][j].cnt, &cnt2 = invisible_mouse_slot.cnt;
                            int mx = cock_pack[i][j].item.max_stack_size;
                            if (cnt1 + cnt2 <= mx) {
                                cnt1 += cnt2;
                                invisible_mouse_slot.item = Item();
                                is_item_in_mouse = false;
                            } else {
                                cnt2 = cnt1 + cnt2 - mx;
                                cnt1 = mx;
                            }
                        } else {
                            is_item_in_mouse = (cock_pack[i][j].item.id != -1);
                            invisible_mouse_slot.swap_item(cock_pack[i][j]);
                        }
                    }
                    get_active_slot().is_active = false;
                    cock_pack[i][j].is_active = true;
                    active_slot = {1, i * PACK_WIDTH + j};
                    break;
                }
            }
        }
    }
    if (flag) return 5;
    return 0;
}

Slot &Inventory::get_active_slot() {
    if (active_slot.first == 0) {
        return fast_pack[active_slot.second];
    } else if (active_slot.first == 1) {
        return cock_pack[active_slot.second / PACK_WIDTH][active_slot.second % PACK_WIDTH];
    } else {
        return armor[active_slot.second];
    }
}

void Inventory::draw(sf::RenderWindow *window) {
    left_hand.draw(window);
    right_hand.draw(window);
    for (const auto &i: armor) {
        i.draw(window);
    }
    for (const auto &i: fast_pack) {
        i.draw(window);
    }
    if (is_cock_pack_open) {
        for (auto &i: cock_pack) {
            for (const auto &j: i) {
                j.draw(window);
            }
        }
    }
    if (is_item_in_mouse) {
        invisible_mouse_slot.draw(window, false);
    }
}

void Inventory::swap_hands() {
    left_hand.swap_item(right_hand);
}

void Inventory::right_swap() {
    if (active_slot.first == 0) {
        right_hand.swap_item(fast_pack[active_slot.second]);
    } else if (active_slot.first == 1) {
        right_hand.swap_item(cock_pack[active_slot.second / PACK_WIDTH][active_slot.second % PACK_WIDTH]);
    } else if (active_slot.first == 2) {
        std::swap(right_hand.item, armor[active_slot.second].item);
        std::swap(right_hand.cnt, armor[active_slot.second].cnt);
    }
}

void Inventory::left_swap() {
    if (active_slot.first == 0) {
        left_hand.swap_item(fast_pack[active_slot.second]);
    } else if (active_slot.first == 1) {
        left_hand.swap_item(cock_pack[active_slot.second / PACK_WIDTH][active_slot.second % PACK_WIDTH]);
    } else if (active_slot.first == 2) {
        left_hand.swap_item(armor[active_slot.second]);
    }
}

Inventory::Inventory() {
    active_slot_sprite.setTexture(textures[9]);
    default_slot_sprite.setTexture(textures[8]);
    right_hand.item.id = 1; // test
    right_hand.cnt = 10;
    right_hand.item.params["cock"] = 10;
    left_hand.item.id = 0; // test
    left_hand.cnt = 56;
    is_item_in_mouse = false;
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

void Inventory::load() {
    std::ifstream in((dir_path + "inventory.inv").c_str());
    if (!in.good()) return;
    left_hand.load(in);
    right_hand.load(in);
    for (auto &i: armor) i.load(in);
    for (auto &i: fast_pack) i.load(in);
    for (auto &i: cock_pack) {
        for (auto &j: i) j.load(in);
    }
}

void Inventory::upload() {
    std::ofstream out((dir_path + "inventory.inv").c_str());
    if (!out.good()) return;
    left_hand.upload(out);
    right_hand.upload(out);
    for (auto &i: armor) i.upload(out);
    for (auto &i: fast_pack) i.upload(out);
    for (auto &i: cock_pack) {
        for (auto &j: i) j.upload(out);
    }
    out.close();
}

void put_all(Slot& a, Slot& b) {
    if (a.item.id != -1) {
        if (b.item.id == a.item.id) {
            int &cnt1 = b.cnt, &cnt2 = a.cnt;
            int mx = b.item.max_stack_size;
            if (cnt1 + cnt2 <= mx) {
                cnt1 += cnt2;
                a.item = Item();
            } else {
                cnt2 = cnt1 + cnt2 - mx;
                cnt1 = mx;
            }
        } else if(b.item.id == -1){
            a.swap_item(b);
        }
    }
}


void Inventory::add(Slot & slot) {
    vector<Slot*> v;
    v.push_back(&left_hand);
    v.push_back(&right_hand);
    for(int i = 0; i < FAST_PACK_SIZE; i++){
        v.push_back(fast_pack+i);
    }
    for(int i = 0; i < PACK_HEIGHT; i++){
        for(int j = 0; j < PACK_WIDTH; j++){
            v.push_back(cock_pack[i]+j);
        }
    }
    for(int i = 0; i < v.size() && slot.item.id != -1; i++){
        if(v[i]->item.id == slot.item.id){
            auto& kok = *v[i];
            put_all(slot, kok);
        }
    }
    if(slot.item.id != -1){
        for(int i = 0; i < v.size() && slot.item.id != -1; i++){
            auto& kok = *v[i];
            put_all(slot, kok);
        }
    }
}

#endif //SUPERMEGA2DGAME_INVENTORY_H
