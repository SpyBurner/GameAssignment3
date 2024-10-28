#ifndef GLOBALS_HPP
#define GLOBALS_HPP

/*GameObject tags:
1 - Team1
2 - Team2
3 - Ball
4 - Wall
5 - Goal1
6 - Goal2
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

const float HIGH_KICK_FORCE = 17.0f;
const float LOW_KICK_FORCE = 12.0f;

const float GoalKeeperSpeed = 15.0f;
const float DefenderSpeed = 10.0f;
const float AttackerSpeed = 11.0f;

static bool Player2Mode = false;
static bool TestMode = false;

#define EPS 0.0001

#endif // GLOBALS_HPP