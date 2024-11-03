#ifndef GLOBALS_HPP
#define GLOBALS_HPP

/*GameObject tags:
    0 - Default
*/
#define TAG_MAP 0

#include <SDL2/SDL.h>
#include <vector>
#include <string>

extern SDL_Renderer* RENDERER;
extern std::vector<SDL_Texture *> TEXTURES;

//SETTINGS
const int FPS = 60;

const int WIDTH = 1280, HEIGHT = 720;
// const int WIDTH = 1920, HEIGHT = 1080;

const bool FULLSCREEN = false;
const int TILE_SIZE = 16;

#define VELOCITY_EPS 0.1f
#define EPS 0.0001f
#define GRAVITY_ACCELERATION 9.8f  * 3 / FPS

struct Tile {
    std::string type;
    struct Position {
        int posX;
        int posY;
    } position;
    int size;
    std::string direction;
    int index;

    // Tile(std::string type, struct Position position, int size, std::string direction = "", int index = 0) : type(type), position(position), size(size), direction(direction), index(index) {}
};

const std::vector<Tile> TILEMAP_SHEET_STRUCTURE = {
    {"block", {1, 1}, 1},
    {"cross", {2, 0}, 1},
    {"plus", {2, 1}, 1},
    {"question", {3, 1}, 1},
    {"gate", {4, 0}, 2},
    {"bush", {6, 2}, 1},
    {"conner-land", {5, 2}, 1, "up-right"},
    {"land", {4, 2}, 1, "up", 0},
    {"land", {3, 2}, 1, "up", 1},
    {"land", {2, 2}, 1, "up", 2},
    {"conner-land", {1, 2}, 1, "up-left"},
    {"conner-land", {0, 3}, 1, "left-right"},
    {"inner-conner-land", {1, 3}, 1, "up-left"},
    {"land", {0, 4}, 1, "left", 0},
    {"land", {0, 5}, 1, "left", 1},
    {"land", {0, 6}, 1, "left", 2},
    {"conner-land", {0, 7}, 1, "left-left"},
    {"inner-conner-land", {1, 7}, 1, "down-left"},
    {"conner-land", {1, 8}, 1, "down-left"},
    {"land", {2, 8}, 1, "down", 0},
    {"land", {3, 8}, 1, "down", 1},
    {"land", {4, 8}, 1, "down", 2},
    {"conner-land", {5, 8}, 1, "down-right"},
    {"inner-conner-land", {5, 7}, 1, "down-right"},
    {"conner-land", {6, 7}, 1, "right-right"},
    {"land", {6, 6}, 1, "right", 0},
    {"land", {6, 5}, 1, "right", 1},
    {"land", {6, 4}, 1, "right", 2},
    {"conner-land", {6, 3}, 1, "right-left"},
    {"inner-conner-land", {5, 3}, 1, "up-right"},
    {"space", {3, 4}, 1}
};

const std::vector<std::vector<std::string>> TILEMAP_0 = {
    {""},
    {""},
    {""},
    {""},
    {""},
    {""},
    {"", "block", "", "", "gate", "", "block"},
    {""},
    {
        "land_up_0",
        "land_up_1",
        "land_up_1"
    }
};

#endif // GLOBALS_HPP