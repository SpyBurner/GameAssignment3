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

    #pragma region Collision Matrix
    CollisionMatrix::init();

    CollisionMatrix::setCollisionMatrix(CollisionMatrix::PLAYER, CollisionMatrix::WALL, true);
    CollisionMatrix::setCollisionMatrix(CollisionMatrix::PROJECTILE, CollisionMatrix::WALL, true);
    CollisionMatrix::setCollisionMatrix(CollisionMatrix::PLAYER, CollisionMatrix::PROJECTILE, false);

    #pragma endregion

    Scene *gameScene = new Scene("Game");
    gameScene->AssignLogic([gameScene, this]() {
        Game::state = GAME;
        // SoundManager::GetInstance()->PlayMusic("GameBgm");

#pragma region Ball Setup
        GameObject *ball = new GameObject("Ball");
        ball->layer = CollisionMatrix::PROJECTILE;
        ball->transform.position = Vector2(100, 600);
        ball->transform.scale = Vector2(2, 2);

        ball->AddComponent(new SpriteRenderer(ball, Vector2(15, 15), 10, LoadSpriteSheet("Assets/default.png")));

        ball->AddComponent(new Animator(ball, {AnimationClip("Roll", "Assets/soccer_ball.png", Vector2(15, 15), 1000, true, 1.0, 0, 1)}));
        ball->GetComponent<Animator>()->Play("Roll");

        ball->AddComponent(new Rigidbody2D(ball, 1, 0.025, .9, 1.0));
        ball->GetComponent<Rigidbody2D>()->AddForce(Vector2(1, 1) * 100);

        ball->AddComponent(new VelocityToAnimSpeedController(ball, "Roll"));
        ball->AddComponent(new StayInBounds(ball, false));

        ball->AddComponent(new CircleCollider2D(ball, Vector2(0, 0), 7.5, true));

        ball->GetComponent<CircleCollider2D>()->OnCollisionEnter.addHandler([ball](Collider2D *collider) {
            Rigidbody2D *rb = ball->GetComponent<Rigidbody2D>();
            rb->BounceOff(collider->GetNormal(ball->transform.position));
        });

        // GameObjectManager::GetInstance()->AddGameObject(ball);
#pragma endregion

#pragma region Wall Setup
        GameObject *wall = new GameObject("Wall");
        wall->layer = CollisionMatrix::WALL;
        wall->transform.rotation = 0;
        wall->transform.position = Vector2(640, 700);
        wall->transform.scale = Vector2(100, 2);
        wall->AddComponent(new SpriteRenderer(wall, Vector2(15, 30), 0, LoadSpriteSheet("Assets/wall.png")));
        wall->AddComponent(new BoxCollider2D(wall, Vector2(0, 0), Vector2(1500, 60), false));

        GameObjectManager::GetInstance()->AddGameObject(wall);
        // Left Wall
        GameObject *leftWall = new GameObject("LeftWall");
        leftWall->layer = CollisionMatrix::WALL;
        leftWall->transform.rotation = 0;
        leftWall->transform.position = Vector2(50, 400); // Adjust position as needed
        leftWall->transform.scale = Vector2(2, 100); // Adjust scale as needed
        leftWall->AddComponent(new SpriteRenderer(leftWall, Vector2(15, 30), 0, LoadSpriteSheet("Assets/wall.png")));
        leftWall->AddComponent(new BoxCollider2D(leftWall, Vector2(0, 0), Vector2(30, 1500), false));

        GameObjectManager::GetInstance()->AddGameObject(leftWall);

        // Right Wall
        GameObject *rightWall = new GameObject("RightWall");
        rightWall->layer = CollisionMatrix::WALL;
        rightWall->transform.rotation = 0;
        rightWall->transform.position = Vector2(1230, 400); // Adjust position as needed
        rightWall->transform.scale = Vector2(2, 100); // Adjust scale as needed
        rightWall->AddComponent(new SpriteRenderer(rightWall, Vector2(15, 30), 0, LoadSpriteSheet("Assets/wall.png")));
        rightWall->AddComponent(new BoxCollider2D(rightWall, Vector2(0, 0), Vector2(30, 1500), false));

        GameObjectManager::GetInstance()->AddGameObject(rightWall);
#pragma endregion

#pragma region Player Setup

        GameObject *player = new GameObject("Player");
        player->layer = CollisionMatrix::PLAYER;
        player->transform.position = Vector2(640, 100);
        player->transform.scale = Vector2(2, 2);

        player->AddComponent(new SpriteRenderer(player, Vector2(35, 37), 10, LoadSpriteSheet("Assets/default.png")));

        player->AddComponent(new Animator(player, 
            {AnimationClip("Idle", "Assets/Sprites/player_idle.png", Vector2(16, 32), 1000, true, 1.0, 0, 2),
            AnimationClip("Walk", "Assets/Sprites/player_walking.png", Vector2(16, 32), 1000, true, 1.0, 0, 4),
        }));

        
        player->GetComponent<Animator>()->Play("Idle");

        player->AddComponent(new Rigidbody2D(player, 1, 0.025, 0, 1.0));

        player->AddComponent(new VelocityToAnimSpeedController(player, "Walk"));
        player->AddComponent(new StayInBounds(player, false));
        player->AddComponent(new FLipToVelocity(player, Vector2(1, 0)));

        player->AddComponent(new MovementController(player, 18, .5, true));

        player->AddComponent(new BoxCollider2D(player, Vector2(0, 0), 
            Vector2(16 * player->transform.scale.x, 32 * player->transform.scale.y) 
            , false));

        // player->GetComponent<CircleCollider2D>()->OnCollisionEnter.addHandler([player](Collider2D *collider) {
        //     Rigidbody2D *rb = player->GetComponent<Rigidbody2D>();
        //     // player->transform.position += rb->velocity * -1;
        //     rb->BounceOff(collider->GetNormal(player->transform.position));
        // });
        player->AddComponent(new PlayerAnimController(player));
        player->AddComponent(new SpawnBall(player, ball, SDLK_w));


        GameObjectManager::GetInstance()->AddGameObject(player);

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
    //TEST

    //TEST

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