#include "Game.hpp"
#include "Global.hpp"
#include <SDL2/SDL.h>

#include <iostream>
#include <cstring>

Game *game = nullptr;

int main(int argc, char *argv[]) {
    game = new Game();

    game->init("Game Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, FULLSCREEN);

    while (game->running()) {
        if (game->reseting()){
            game->clean();
            game->init("Game Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, FULLSCREEN);
        }
        game->handleEvents();
        game->update();
        game->render();
        
        game->handleSceneChange();
        SDL_Delay(1000 / FPS);
    }

    game->clean();

    return 0;
}