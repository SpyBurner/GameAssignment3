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

const bool FULLSCREEN = true;

#define VELOCITY_EPS 0.1f
#define EPS 0.0001f
#define GRAVITY_ACCELERATION 9.8f  * 3 / FPS

const int PLAYER_SPEED = 19;
const int MELEE_SPEED = 2;
const int RANGED_SPEED = 1;

const int PLAYER_HP = 3;
const int MELEE_HP = 12;
const int RANGED_HP = 20;
const int MOAI_HP = 50;

const float PLAYER_INVINCIBILITY_TIME = 1500;

//x/y: Drops by x Dmg every y pixels traveled
const double PLAYER_DMG_DROP = 1/120.0;
const int PLAYER_SHOTGUN_PELLET = 5;

const int PLAYER_DAMAGE = 3;
const int MELEE_DAMAGE = 1;
const int RANGED_DAMAGE = 1;
const int MOAI_DAMAGE = 1;

const int PLAYER_HOOK_FORCE = 15;

const int TILE_DAMAGE = 1;

const float MELEE_ATTACK_COOLDOWN = 2000;
const float RANGED_ATTACK_COOLDOWN = 2000;

const float MELEE_ATTACK_RANGE = 100;

const float RANGED_PROJECTILE_LIFETIME = 3000;
const float RANGED_PROJECTILE_SPEED = 10;
const float RANGED_DETECT_RANGE = 700;
const float RANGED_ATTACK_RANGE = 400;

const float BOX_MAX_SPEED = 10;

const float POWER_UP_POP_UP_FORCE = 10;
#endif // GLOBALS_HPP