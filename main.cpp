#include "src/Game.hpp"
#include "src/Global.hpp"
#include <SDL2/SDL.h>

#include <iostream>
#include <cstring>

Game *game = nullptr;

int main(int argc, char *argv[]) {
    game = new Game();

    game->init("Game Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, FULLSCREEN);

    while (game->running()) {

        game->handleEvents();
        game->update();
        game->render();
        game->handleSceneChange();

        // Delay to achieve target FPS
        SDL_Delay(1000 / FPS);
    }

    game->clean();

    return 0;
}