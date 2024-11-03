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
GameObject *Game::CAMERA = nullptr;

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

    CollisionMatrix::setCollisionMatrix(CollisionMatrix::WALL, CollisionMatrix::PARTICLE, true);

    #pragma endregion

    Scene *gameScene = new Scene("Game");
    gameScene->AssignLogic([gameScene, this]() {
        Game::state = GAME;
        // SoundManager::GetInstance()->PlayMusic("GameBgm");

#pragma region Wall Setup
        GameObject *wall = new GameObject("Wall");
        wall->layer = CollisionMatrix::WALL;
        wall->transform.rotation = 0;
        wall->transform.position = Vector2(1000, 700);
        wall->transform.scale = Vector2(500, 2);
        wall->AddComponent(new SpriteRenderer(wall, Vector2(15, 30), 0, LoadSpriteSheet("Assets/wall.png")));
        wall->AddComponent(new BoxCollider2D(wall, Vector2(0, 0), Vector2(7500, 60), false));

        GameObjectManager::GetInstance()->AddGameObject(wall);

        //Test culling
        // // Wall 1
        // GameObject *wall1 = new GameObject("Wall1");
        // wall1->layer = CollisionMatrix::WALL;
        // wall1->transform.rotation = 0;
        // wall1->transform.position = Vector2(0, 600);
        // wall1->transform.scale = Vector2(100, 2);
        // wall1->AddComponent(new SpriteRenderer(wall1, Vector2(100, 10), 0, LoadSpriteSheet("Assets/wall.png")));
        // wall1->AddComponent(new BoxCollider2D(wall1, Vector2(0, 0), Vector2(100, 10), false));
        // GameObjectManager::GetInstance()->AddGameObject(wall1);

        // // Wall 2
        // GameObject *wall2 = new GameObject("Wall2");
        // wall2->layer = CollisionMatrix::WALL;
        // wall2->transform.rotation = 0;
        // wall2->transform.position = Vector2(0, 550);
        // wall2->transform.scale = Vector2(100, 2);
        // wall2->AddComponent(new SpriteRenderer(wall2, Vector2(100, 20), 0, LoadSpriteSheet("Assets/wall.png")));
        // wall2->AddComponent(new BoxCollider2D(wall2, Vector2(0, 0), Vector2(100, 20), false));
        // GameObjectManager::GetInstance()->AddGameObject(wall2);

        // // Wall 3
        // GameObject *wall3 = new GameObject("Wall3");
        // wall3->layer = CollisionMatrix::WALL;
        // wall3->transform.rotation = 0;
        // wall3->transform.position = Vector2(0, 400);
        // wall3->transform.scale = Vector2(100, 2);
        // wall3->AddComponent(new SpriteRenderer(wall3, Vector2(100, 30), 0, LoadSpriteSheet("Assets/wall.png")));
        // wall3->AddComponent(new BoxCollider2D(wall3, Vector2(0, 0), Vector2(100, 30), false));
        // GameObjectManager::GetInstance()->AddGameObject(wall3);

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
        rightWall->transform.position = Vector2(2000, 400); // Adjust position as needed
        rightWall->transform.scale = Vector2(2, 100); // Adjust scale as needed
        rightWall->AddComponent(new SpriteRenderer(rightWall, Vector2(15, 30), 0, LoadSpriteSheet("Assets/wall.png")));
        rightWall->AddComponent(new BoxCollider2D(rightWall, Vector2(0, 0), Vector2(30, 1500), false));

        GameObjectManager::GetInstance()->AddGameObject(rightWall);
#pragma endregion

#pragma region Shell Setup
    GameObject *dropShellParticle = new GameObject("DropShellParticle");
    dropShellParticle->layer = CollisionMatrix::PARTICLE;
    dropShellParticle->transform.scale = Vector2(2, 2);

    dropShellParticle->AddComponent(new SpriteRenderer(dropShellParticle, Vector2(5, 5), 1, nullptr));
    dropShellParticle->AddComponent(new Rigidbody2D(dropShellParticle, 1, 0.025, 0, 1.0));
    dropShellParticle->AddComponent(new CircleCollider2D(dropShellParticle, Vector2(0, 0), 3, true));
    dropShellParticle->AddComponent(new Animator(dropShellParticle, {
        AnimationClip("Default", "Assets/Sprites/Player/shell_particle.png", Vector2(5, 5), 100, true, 1.0, 0, 3),
    }));
    dropShellParticle->AddComponent(new VelocityToAnimSpeedController(dropShellParticle, "Default", 1.0, false));

    GameObject *shellParticle = new GameObject("ShellParticle");
    shellParticle->layer = CollisionMatrix::PARTICLE;
    shellParticle->transform.scale = Vector2(2, 2);

    shellParticle->AddComponent(new SpriteRenderer(shellParticle, Vector2(5, 5), 1, nullptr));
    shellParticle->AddComponent(new Rigidbody2D(shellParticle, 1, 0.025, 0, 0.0));
    shellParticle->AddComponent(new CircleCollider2D(shellParticle, Vector2(0, 0), 3, true));

    shellParticle->AddComponent(new Animator(shellParticle, {
        AnimationClip("Default", "Assets/Sprites/Player/shell_trail.png", Vector2(3, 3), 100, true, 1.0, 0, 1),
    }));

    auto CreateShell = [shellParticle](float speed, Vector2 direction, float lifeTime, Vector2 position) {
        GameObject *shell = new GameObject("Shell" + std::to_string(rand() + rand()));
        shell->transform.scale = Vector2(2, 2);
        shell->layer = CollisionMatrix::PROJECTILE;
        shell->transform.position = position;

        shell->AddComponent(new SpriteRenderer(shell, Vector2(5, 4), 10, LoadSpriteSheet("Assets/Sprites/Player/shell.png")));

        shell->AddComponent(new ParticleSystem(shell, shellParticle, 50, 500, -1 * direction, 0, 0));

        shell->AddComponent(new Rigidbody2D(shell, 1, 0.025, 0, 0.0));
        shell->AddComponent(new ShellBehavior(shell, lifeTime, speed, direction));
        shell->AddComponent(new RotateTowardVelocity(shell, Vector2(1, 0)));

        shell->AddComponent(new CircleCollider2D(shell, Vector2(0, 0), 
            5 * shell->transform.scale.x / 2
        , true));
        shell->GetComponent<CircleCollider2D>()->OnCollisionEnter.addHandler([shell](Collider2D *collider) {
            GameObjectManager::GetInstance()->RemoveGameObject(shell->GetName());
        });

        return shell;
    };
        
#pragma endregion

#pragma region Player Setup

        GameObject *player = new GameObject("Player");
        player->layer = CollisionMatrix::PLAYER;
        player->transform.position = Vector2(640, 100);
        player->transform.scale = Vector2(2, 2);

        player->AddComponent(new SpriteRenderer(player, Vector2(35, 37), 10, nullptr));

        player->AddComponent(new Animator(player, 
            {AnimationClip("Idle", "Assets/Sprites/Player/player_idle.png", Vector2(15, 27), 1000, true, 1.0, 0, 2),
            AnimationClip("Walk", "Assets/Sprites/Player/player_walking.png", Vector2(15, 27), 1000, true, 1.0, 0, 4),
        }));

        player->GetComponent<Animator>()->Play("Idle");

        player->AddComponent(new Rigidbody2D(player, 1, 0.025, 0, 1.0));

        player->AddComponent(new VelocityToAnimSpeedController(player, "Walk"));
        player->AddComponent(new FLipToVelocity(player, Vector2(1, 0)));

        Joystick *movementStick = dynamic_cast<Joystick *>(
            player->AddComponent(new Joystick(player, SDLK_w, SDLK_s, SDLK_a, SDLK_d))
        );

        player->AddComponent(new MovementController(player, 18, movementStick));

        player->AddComponent(new PlayerAnimController(player));
        
        player->AddComponent(new BoxCollider2D(player, Vector2(0, 0), 
            Vector2(15 * player->transform.scale.x, 27 * player->transform.scale.y) 
            , false));

        player->AddComponent(new ParticleSystem(player, dropShellParticle, 1, 5000, Vector2(0, -1), 10, 10));
        player->GetComponent<ParticleSystem>()->Stop();

        Joystick *aimStick = dynamic_cast<Joystick *>(
            player->AddComponent(new Joystick(player, SDLK_UP, SDLK_DOWN, SDLK_LEFT, SDLK_RIGHT))
        );

        player->AddComponent(new PlayerShoot(player, 50, 1000, 1000, 5, 5, aimStick));
        player->GetComponent<PlayerShoot>()->setSpawnFunction(CreateShell);

        player->AddComponent(new JumpController(player, SDLK_SPACE, 20, 450, CollisionMatrix::WALL));
        player->GetComponent<JumpController>()->BindCollider(player->GetComponent<BoxCollider2D>());

        GameObjectManager::GetInstance()->AddGameObject(player);

#pragma endregion

#pragma region Gun Setup
    GameObject *gun = new GameObject("Gun");
    gun->transform.position = Vector2(640, 100);
    gun->transform.scale = Vector2(1.5, 1.5);

    gun->AddComponent(new SpriteRenderer(gun, Vector2(16, 16), 11, LoadSpriteSheet("Assets/Sprites/Player/gun.png")));

    gun->AddComponent(new Orbit(gun, player, 25, Vector2(1, 0), aimStick));

    GameObjectManager::GetInstance()->AddGameObject(gun);
#pragma endregion

#pragma region Camera Setup
        GameObject *camera = new GameObject("Camera");
        camera->layer = CollisionMatrix::CAMERA;
        camera->transform.position = Vector2(640, 360);

        camera->AddComponent(new BoxCollider2D(camera, Vector2(0, 0), Vector2(1280, 720), true));
        camera->AddComponent(new Rigidbody2D(camera, 1, 0.5, 0, 0.0));
        camera->AddComponent(new Camera(camera, player, Vector2(1280, 720), Vector2(0, -70), 10, Vector2(50, 70)));
        
        Game::CAMERA = camera;

        GameObjectManager::GetInstance()->AddGameObject(camera);
#pragma endregion

    });

    SceneManager::GetInstance()->AddScene(gameScene);
    SceneManager::GetInstance()->LoadScene("Game");

    TileMap::GetInstance()->InitTileSet("Assets/tileset.png", "/tilemap_sheet_structure.json");

    TileMap::GetInstance()->LoadTileMap("Assets/tile_map_0.json");
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