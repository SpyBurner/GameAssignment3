#include "Game.hpp"
#include "Components.hpp"
#include "Core.hpp"
#include "Global.hpp"
#include "Helper.hpp"
#include "Physic2D.hpp"
#include "SDLCustomEvent.hpp"

#include <cmath>
#include <iostream>
#include <SDL2/SDL_mixer.h>

SDL_Event Game::event;

Game::Game() {
    isRunning = false;
}

Game::~Game() {
}

void Game::init(const char *title, int xpos, int ypos, int width, int height, bool fullscreen) {
    reset = false;
    int flags = 0;
    
    if (fullscreen) {
        flags = SDL_WINDOW_FULLSCREEN;
    }

    if (SDL_Init(SDL_INIT_EVERYTHING) == 0) {
        std::cout << "Subsystems Initialised..." << std::endl;

        window = SDL_CreateWindow(title, xpos, ypos, width, height, flags);
        if (window) {
            std::cout << "Window created..." << std::endl;
        }

        renderer = SDL_CreateRenderer(window, -1, 0);
        if (renderer) {
            SDL_SetRenderDrawColor(renderer, 128, 239, 129, 255);
            std::cout << "Renderer created..." << std::endl;
        }

        RENDERER = renderer;

        // Initialize SDL_ttf
        if (TTF_Init() == -1) {
            std::cerr << "Failed to initialize TTF: " << TTF_GetError() << std::endl;
            isRunning = false;
            return;
        }

        if (Mix_Init(MIX_INIT_MP3) == 0){
            std::cerr << "Failed to initialize Mixer: " << Mix_GetError() << std::endl;
            isRunning = false;
            return;
        }

        isRunning = true;
    } else {
        isRunning = false;
    }

    state = MENU;
    objectInit();
}

GameObject *player = new GameObject("Player");

void Game::objectInit() {
    
    //Add sounds and music
    // SoundManager::GetInstance();
    // SoundManager::GetInstance()->AddMusic("MenuBgm", "Assets/SFX/fairyfountain.mp3", 100);
    // SoundManager::GetInstance()->AddMusic("GameBgm", "Assets/SFX/papyrus.mp3", 32);

    // SoundManager::GetInstance()->AddSound("ball_bounce", "Assets/SFX/ball_bounce.mp3", 128);
    // SoundManager::GetInstance()->AddSound("ball_kick", "Assets/SFX/ball_kick.mp3", 128);

    // SoundManager::GetInstance()->AddSound("Game_Over", "Assets/SFX/gameover.mp3", 128);
    // SoundManager::GetInstance()->AddSound("Goal", "Assets/SFX/score.mp3", 64);

    Scene *gameScene = new Scene("Game");
    gameScene->AssignLogic([gameScene, this]() {
        Game::state = GAME;
        // SoundManager::GetInstance()->PlayMusic("GameBgm");

#pragma region Ball Setup
        GameObject *ball = new GameObject("Ball");
        ball->tag = 3;
        ball->transform.position = Vector2(100, 600);
        ball->transform.scale = Vector2(2, 2);

        ball->AddComponent(new SpriteRenderer(ball, Vector2(15, 15), 10, LoadSpriteSheet("Assets/default.png")));

        ball->AddComponent(new Animator(ball, {AnimationClip("Roll", "Assets/soccer_ball.png", Vector2(15, 15), 1000, true, 1.0, 0, 1)}));
        ball->GetComponent<Animator>()->Play("Roll");

        ball->AddComponent(new Rigidbody2D(ball, 1, 0.025, .9, 1.0));
        ball->GetComponent<Rigidbody2D>()->AddForce(Vector2(1, 1) * 100);

        ball->AddComponent(new VelocityToAnimSpeedController(ball, "Roll"));
        ball->AddComponent(new StayInBounds(ball, false));

        ball->AddComponent(new CircleCollider2D(ball, Vector2(0, 0), 7.5));

        ball->GetComponent<CircleCollider2D>()->OnCollisionEnter.addHandler([ball](Collider2D *collider) {
            Rigidbody2D *rb = ball->GetComponent<Rigidbody2D>();
            ball->transform.position += rb->velocity * -1;
            // rb->BounceOff(collider->GetNormal(ball->transform.position));
        });

        GameObjectManager::GetInstance()->AddGameObject(ball);
#pragma endregion

#pragma region Wall Setup
        GameObject *wall = new GameObject("Wall");
        wall->tag = 2;
        wall->transform.rotation = 0;
        wall->transform.position = Vector2(640, 700);
        wall->transform.scale = Vector2(100, 1);

        wall->AddComponent(new SpriteRenderer(wall, Vector2(15, 30), 0, LoadSpriteSheet("Assets/wall.png")));

        wall->AddComponent(new BoxCollider2D(wall, Vector2(0, 0), Vector2(1500, 32)));

        wall->GetComponent<BoxCollider2D>()->OnCollisionEnter.addHandler([wall](Collider2D *collider) {
            Rigidbody2D *rb = collider->gameObject->GetComponent<Rigidbody2D>();
            rb->BounceOff(wall->GetComponent<BoxCollider2D>()->GetNormal(collider->gameObject->transform.position));
        });

        GameObjectManager::GetInstance()->AddGameObject(wall);
#pragma endregion
    
    });
    SceneManager::GetInstance()->AddScene(gameScene);
    SceneManager::GetInstance()->LoadScene("Game");
}

void Game::handleEvents() {

    SDL_PollEvent(&Game::event);

    if (event.type == SDL_QUIT) {
        isRunning = false;
        return;
    }

    // if (event.type == SDL_KEYDOWN) {
    //     if (event.key.keysym.sym == SDLK_ESCAPE) {
    //         state = MENU;
    //         scoreTeam1 = scoreTeam2 = 0;
    //         return;
    //     }
    // }

    // //End condition
    // if (scoreTeam1 + scoreTeam2 >= 5) {
    //     state = GAMEOVER;
    //     return;
    // }

}

void Game::handleSceneChange() {
    // switch (state) {
    // case MENU:
    //     if (SceneManager::GetInstance()->GetCurrentScene()->GetName() != "MainMenu")
    //         SceneManager::GetInstance()->LoadScene("MainMenu");
    //     break;
    // case GAME:
    //     if (SceneManager::GetInstance()->GetCurrentScene()->GetName() != "Game")
    //         SceneManager::GetInstance()->LoadScene("Game");
    //     break;
    // case GAMEOVER:
    //     if (SceneManager::GetInstance()->GetCurrentScene()->GetName() != "GameOver")
    //         SceneManager::GetInstance()->LoadScene("GameOver");
    //     break;
    // }
}

void Game::update() {
    SceneManager::GetInstance()->Update();
}

void Game::render() {
    SDL_RenderClear(renderer);
    SceneManager::GetInstance()->Draw();

    // // Show score
    // if (state == GAME){
    //     SDL_Color textColor = {0, 0, 0, 255};
    //     std::string scoreText = std::to_string(scoreTeam1) + " - " + std::to_string(scoreTeam2);
    //     SDL_Texture* scoreTexture = LoadFontTexture(scoreText, "Assets/Fonts/arial.ttf", textColor, 50);
    //     if (scoreTexture) {
    //         RenderTexture(scoreTexture, 640, 20);
    //         SDL_DestroyTexture(scoreTexture);
    //     } else {
    //         std::cerr << "Failed to load score texture" << std::endl;
    //     }
    // }

    // if (state == GAMEOVER){
    //     // Render final scores
    //     SDL_Color textColor = {255, 255, 255, 255};
    //     std::string scoreText = "Final Score: " + std::to_string(scoreTeam1) + " - " + std::to_string(scoreTeam2);
    //     SDL_Texture* scoreTexture = LoadFontTexture(scoreText, "Assets/Fonts/arial.ttf", textColor, 75);
    //     if (scoreTexture) {
    //         RenderTexture(scoreTexture, 640, 400); // Centered below "Game Over!"
    //         SDL_DestroyTexture(scoreTexture);
    //     } else {
    //         std::cerr << "Failed to load score texture" << std::endl;
    //     }
    // }

    SDL_RenderPresent(renderer);
}

void Game::clean() {
    delete SceneManager::GetInstance();

    for (auto &texture : TEXTURES) {
        SDL_DestroyTexture(texture);
    }
    TEXTURES.clear();

    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);
    
    TTF_Quit();
    SDL_Quit();
    std::cout << "Game cleaned..." << std::endl;
}

bool Game::running() {
    return isRunning;
}

bool Game::reseting() {
    return reset;
}