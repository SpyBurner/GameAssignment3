#ifndef GLOBALS_HPP
#define GLOBALS_HPP

/*GameObject tags:
    0 - Default
*/

#include <SDL2/SDL.h>
#include <vector>

extern SDL_Renderer* RENDERER;
extern std::vector<SDL_Texture *> TEXTURES;

//SETTINGS
const int FPS = 60;

const int WIDTH = 1280, HEIGHT = 720;
// const int WIDTH = 1920, HEIGHT = 1080;

const bool FULLSCREEN = false;

#define VELOCITY_EPS 0.1f
#define EPS 0.0001f
#define GRAVITY_ACCELERATION 9.8f  * 3 / FPS

const int PLAYER_HP = 3;
const int MELEE_HP = 10;
const int RANGED_HP = 5;
const int MOAI_HP = 50;

//x/y: Drops by x Dmg every y pixels traveled
const double PLAYER_DMG_DROP = 1/150.0;

const int PLAYER_DAMAGE = 3;
const int MELEE_DAMAGE = 1;
const int RANGED_DAMAGE = 1;
const int MOAI_DAMAGE = 1;

#endif // GLOBALS_HPP