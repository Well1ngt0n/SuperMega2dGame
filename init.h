#ifndef SUPERMEGA2DGAME_INIT_H
#define SUPERMEGA2DGAME_INIT_H

using std::string;

const int
        MAX_TEXTURES_COUNT = 14,
        WORLD_SIZE = 256,
        CHUNK_SIZE = 300,
        MAX_RENDER_DISTANCE = 7,
        WORLD_PIXEL_SIZE = WORLD_SIZE * CHUNK_SIZE,
        ITEMS_COUNT = 2,
        PACK_WIDTH = 10,
        PACK_HEIGHT = 3,
        FAST_PACK_SIZE = 10,
        SLOT_PIXEL_SIZE = 32,
        max_fps = 60;

const string dir_path = "../";
sf::Sprite active_slot_sprite, default_slot_sprite;
sf::Texture textures[MAX_TEXTURES_COUNT]; // Жоска экономим память, не храня спрайты
sf::Texture item_textures[ITEMS_COUNT];
sf::Font test_font;
sf::Font cnt_font;
int window_width = 800, window_height = 600;

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

#endif //SUPERMEGA2DGAME_INIT_H
