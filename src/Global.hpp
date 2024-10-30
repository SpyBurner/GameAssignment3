#ifndef GLOBALS_HPP
#define GLOBALS_HPP

/*GameObject tags:
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

#define VELOCITY_EPS 0.01f
#define EPS 0.0001f
#define GRAVITY_ACCELERATION 9.8f  * 1.5 / FPS

#endif // GLOBALS_HPP