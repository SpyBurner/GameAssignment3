#include "Game.hpp"
#include "AI.hpp"
#include "Components.hpp"
#include "Core.hpp"
#include "Global.hpp"
#include "Helper.hpp"
#include "Physic2D.hpp"
#include "Tilemap.hpp"
#include "SDLCustomEvent.hpp"


#include <SDL2/SDL_mixer.h>
#include <iostream>
#include <math.h>
#include <time.h>
SDL_Event Game::event;
GameObject *Game::CAMERA = nullptr;
int Game::coin = 0;

// #define MENU_DEBUG 1

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
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            std::cout << "Renderer created..." << std::endl;
        }

        RENDERER = renderer;

        // Initialize SDL_ttf
        if (TTF_Init() == -1) {
            std::cerr << "Failed to initialize TTF: " << TTF_GetError() << std::endl;
            isRunning = false;
            return;
        }

        if (Mix_Init(MIX_INIT_MP3) == 0) {
            std::cerr << "Failed to initialize Mixer: " << Mix_GetError() << std::endl;
            isRunning = false;
            return;
        }

        isRunning = true;
    } else {
        isRunning = false;
    }

    state = MENU;

    try {
        objectInit();
    }
    catch (const char *msg) {
        std::cerr << msg << std::endl;
        isRunning = false;
    }
}

GameObject *player = new GameObject("Player");

void Game::objectInit() {
    srand(time(NULL));

    // Add sounds and music
    SoundManager::GetInstance();
    SoundManager::GetInstance()->AddSound("Jump", "Assets/SFX/jump.wav", 128);
    SoundManager::GetInstance()->AddSound("Shotgun", "Assets/SFX/shotgun.wav", 128);
    SoundManager::GetInstance()->AddSound("Pickup", "Assets/SFX/pickup.wav", 128);
    SoundManager::GetInstance()->AddSound("Hurt", "Assets/SFX/hurt.wav", 128);
    SoundManager::GetInstance()->AddSound("HookShoot", "Assets/SFX/hookshoot.wav", 128);
    SoundManager::GetInstance()->AddSound("GameOver", "Assets/SFX/gameover.mp3", 128);

#pragma region Collision Matrix
    CollisionMatrix::init();

    // Physics
    CollisionMatrix::setCollisionMatrix(CollisionMatrix::PLAYER, CollisionMatrix::WALL, true);
    CollisionMatrix::setCollisionMatrix(CollisionMatrix::ENEMY, CollisionMatrix::WALL, true);
    CollisionMatrix::setCollisionMatrix(CollisionMatrix::PROJECTILE, CollisionMatrix::WALL, true);
    CollisionMatrix::setCollisionMatrix(CollisionMatrix::E_PROJECTILE, CollisionMatrix::WALL, true);
    CollisionMatrix::setCollisionMatrix(CollisionMatrix::PARTICLE, CollisionMatrix::WALL, true);

    // Damage
    // CollisionMatrix::setCollisionMatrix(CollisionMatrix::ENEMY, CollisionMatrix::PLAYER, true);
    // CollisionMatrix::setCollisionMatrix(CollisionMatrix::PLAYER, CollisionMatrix::E_HURT_BOX, true);

    CollisionMatrix::setCollisionMatrix(CollisionMatrix::E_PROJECTILE, CollisionMatrix::PLAYER, true);
    CollisionMatrix::setCollisionMatrix(CollisionMatrix::PROJECTILE, CollisionMatrix::ENEMY, true);

    CollisionMatrix::setCollisionMatrix(CollisionMatrix::DETECTION, CollisionMatrix::WALL, true);

    CollisionMatrix::setCollisionMatrix(CollisionMatrix::SPIKE, CollisionMatrix::PLAYER, true);
    CollisionMatrix::setCollisionMatrix(CollisionMatrix::SPIKE, CollisionMatrix::ENEMY, true);

    CollisionMatrix::setCollisionMatrix(CollisionMatrix::POWERUP, CollisionMatrix::PLAYER, true);
    CollisionMatrix::setCollisionMatrix(CollisionMatrix::POWERUP, CollisionMatrix::WALL, true);
    CollisionMatrix::setCollisionMatrix(CollisionMatrix::POWERUP, CollisionMatrix::PROJECTILE, true);

    CollisionMatrix::setCollisionMatrix(CollisionMatrix::PUSHABLE, CollisionMatrix::PLAYER, true);
    CollisionMatrix::setCollisionMatrix(CollisionMatrix::PUSHABLE, CollisionMatrix::ENEMY, true);

    CollisionMatrix::setCollisionMatrix(CollisionMatrix::PUSHABLE, CollisionMatrix::WALL, true);
    CollisionMatrix::setCollisionMatrix(CollisionMatrix::PUSHABLE, CollisionMatrix::PROJECTILE, true);
    CollisionMatrix::setCollisionMatrix(CollisionMatrix::PUSHABLE, CollisionMatrix::E_PROJECTILE, true);

    // CollisionMatrix::setCollisionMatrix(CollisionMatrix::PUSHABLE, CollisionMatrix::PARTICLE, true);
    CollisionMatrix::setCollisionMatrix(CollisionMatrix::PUSHABLE, CollisionMatrix::DETECTION, true);

    CollisionMatrix::setCollisionMatrix(CollisionMatrix::GATE, CollisionMatrix::PLAYER, true);
    CollisionMatrix::setCollisionMatrix(CollisionMatrix::GATE, CollisionMatrix::WALL, true);
#pragma endregion

    Scene *menuScene = new Scene("Menu");

    menuScene->AssignLogic([menuScene, this](){
        Game::state = MENU;
        GameObject *title = new GameObject("Title");
        title->transform.position = Vector2(640, 200);
        title->transform.scale = Vector2(3, 3);

        title->AddComponent(new SpriteRenderer(title, Vector2(128, 128), 10, LoadSpriteSheet("Assets/Sprites/Menu/alien_evil_name.png")));
        
        GameObjectManager::GetInstance()->AddGameObject(title);

#pragma region button
        GameObject *newGameButton = new GameObject("NewGameButton");
        newGameButton->transform.position = Vector2(640, 400);
        newGameButton->transform.scale = Vector2(4, 4);

        newGameButton->AddComponent(new SpriteRenderer(newGameButton, Vector2(32, 16), 10, LoadSpriteSheet("Assets/Sprites/Menu/NewGameButton.png")));

        newGameButton->AddComponent(new BoxCollider2D(newGameButton, Vector2(0, 0), 
            Vector2(32 * newGameButton->transform.scale.x, 16 * newGameButton->transform.scale.y), 
        true));
        
        newGameButton->AddComponent(new Button(newGameButton));

        newGameButton->GetComponent<Button>()->AddOnClickHandler([menuScene, this]() {
            Game::state = GAME;
        });
        GameObjectManager::GetInstance()->AddGameObject(newGameButton);

        GameObject *optionButton = new GameObject("OptionButton");
        optionButton->transform.position = Vector2(640, 500);
        optionButton->transform.scale = Vector2(4, 4);

        optionButton->AddComponent(new SpriteRenderer(optionButton, Vector2(32, 16), 10, LoadSpriteSheet("Assets/Sprites/Menu/OptionButton.png")));

        optionButton->AddComponent(new BoxCollider2D(optionButton, Vector2(0, 0), 
            Vector2(32 * optionButton->transform.scale.x, 16 * optionButton->transform.scale.y),
        true));

        optionButton->AddComponent(new Button(optionButton));

        optionButton->GetComponent<Button>()->AddOnClickHandler([menuScene, this]() {
            Game::state = OPTION;
        });
        GameObjectManager::GetInstance()->AddGameObject(optionButton);

        GameObject *exitButton = new GameObject("ExitButton");
        exitButton->transform.position = Vector2(640, 600);
        exitButton->transform.scale = Vector2(4, 4);

        exitButton->AddComponent(new SpriteRenderer(exitButton, Vector2(32, 16), 10, LoadSpriteSheet("Assets/Sprites/Menu/ExitButton.png")));

        exitButton->AddComponent(new BoxCollider2D(exitButton, Vector2(0, 0), 
            Vector2(32 * exitButton->transform.scale.x, 16 * exitButton->transform.scale.y),
        true));

        exitButton->AddComponent(new Button(exitButton));
        exitButton->GetComponent<Button>()->AddOnClickHandler([menuScene, this]() {
            isRunning = false;
        });
        GameObjectManager::GetInstance()->AddGameObject(exitButton);

#pragma endregion

    });
    SceneManager::GetInstance()->AddScene(menuScene);

    Scene *optionScene = new Scene("Option");
    optionScene->AssignLogic([optionScene, this]() {
        Game::state = OPTION;
#pragma region Music Volume
        GameObject *musicLabel = new GameObject("MusicLabel");
        musicLabel->transform.position = Vector2(640, 100);
        musicLabel->transform.scale = Vector2(5, 5);

        musicLabel->AddComponent(new TextRenderer(musicLabel, "Music Volume", SDL_Color{255, 255, 255, 255}, 10, "Assets/Fonts/arial.ttf"));

        GameObject *musicText = new GameObject("MusicText");
        musicText->transform.position = Vector2(650, 250);
        musicText->transform.scale = Vector2(5, 5);

        musicText->AddComponent(new TextRenderer(musicText, std::to_string(musicVolume), SDL_Color{255, 255, 255, 255}, 10, "Assets/Fonts/arial.ttf"));

        GameObject *musicUpButton = new GameObject("MusicUpButton");
        musicUpButton->transform.position = Vector2(800, 250);
        musicUpButton->transform.scale = Vector2(5, 5);

        musicUpButton->AddComponent(new SpriteRenderer(musicUpButton, Vector2(16, 16), 10, LoadSpriteSheet("Assets/Sprites/Menu/right_button.png")));

        musicUpButton->AddComponent(new BoxCollider2D(musicUpButton, Vector2(0, 0), 
            Vector2(16 * musicUpButton->transform.scale.x, 16 * musicUpButton->transform.scale.y),
        true));

        musicUpButton->AddComponent(new Button(musicUpButton));

        musicUpButton->GetComponent<Button>()->AddOnClickHandler([musicText]() {
            musicVolume = std::min(128, musicVolume + 10);
            Mix_VolumeMusic(musicVolume);
            musicText->GetComponent<TextRenderer>()->SetText(std::to_string(musicVolume));
        });

        GameObject *musicDownButton = new GameObject("MusicDownButton");

        musicDownButton->transform.position = Vector2(500, 250);
        musicDownButton->transform.scale = Vector2(5, 5);

        musicDownButton->AddComponent(new SpriteRenderer(musicDownButton, Vector2(16, 16), 10, LoadSpriteSheet("Assets/Sprites/Menu/left_button.png")));

        musicDownButton->AddComponent(new BoxCollider2D(musicDownButton, Vector2(0, 0), 
            Vector2(16 * musicDownButton->transform.scale.x, 16 * musicDownButton->transform.scale.y),
        true));

        musicDownButton->AddComponent(new Button(musicDownButton));

        musicDownButton->GetComponent<Button>()->AddOnClickHandler([musicText]() {
            musicVolume = std::max(0, musicVolume - 10);
            Mix_VolumeMusic(musicVolume);
            musicText->GetComponent<TextRenderer>()->SetText(std::to_string(musicVolume));
        });

        GameObjectManager::GetInstance()->AddGameObject(musicLabel);
        GameObjectManager::GetInstance()->AddGameObject(musicText);
        GameObjectManager::GetInstance()->AddGameObject(musicUpButton);
        GameObjectManager::GetInstance()->AddGameObject(musicDownButton);
#pragma endregion

#pragma region SFX volume
        GameObject *sfxLabel = new GameObject("SFXLabel");
        sfxLabel->transform.position = Vector2(640, 500);
        sfxLabel->transform.scale = Vector2(5, 5);

        sfxLabel->AddComponent(new TextRenderer(sfxLabel, "SFX Volume", SDL_Color{255, 255, 255, 255}, 10, "Assets/Fonts/arial.ttf"));

        GameObject *sfxText = new GameObject("SFXText");
        sfxText->transform.position = Vector2(650, 650);
        sfxText->transform.scale = Vector2(5, 5);

        sfxText->AddComponent(new TextRenderer(sfxText, std::to_string(sfxVolume), SDL_Color{255, 255, 255, 255}, 10, "Assets/Fonts/arial.ttf"));

        GameObject *sfxUpButton = new GameObject("SFXUpButton");
        sfxUpButton->transform.position = Vector2(800, 650);
        sfxUpButton->transform.scale = Vector2(5, 5);

        sfxUpButton->AddComponent(new SpriteRenderer(sfxUpButton, Vector2(16, 16), 10, LoadSpriteSheet("Assets/Sprites/Menu/right_button.png")));

        sfxUpButton->AddComponent(new BoxCollider2D(sfxUpButton, Vector2(0, 0), 
            Vector2(16 * sfxUpButton->transform.scale.x, 16 * sfxUpButton->transform.scale.y),
        true));

        sfxUpButton->AddComponent(new Button(sfxUpButton));

        sfxUpButton->GetComponent<Button>()->AddOnClickHandler([sfxText]() {
            sfxVolume = std::min(128, sfxVolume + 10);
            Mix_Volume(-1, sfxVolume);
            sfxText->GetComponent<TextRenderer>()->SetText(std::to_string(sfxVolume));
        });

        GameObject *sfxDownButton = new GameObject("SFXDownButton");

        sfxDownButton->transform.position = Vector2(500, 650);
        sfxDownButton->transform.scale = Vector2(5, 5);

        sfxDownButton->AddComponent(new SpriteRenderer(sfxDownButton, Vector2(16, 16), 10, LoadSpriteSheet("Assets/Sprites/Menu/left_button.png")));

        sfxDownButton->AddComponent(new BoxCollider2D(sfxDownButton, Vector2(0, 0), 
            Vector2(16 * sfxDownButton->transform.scale.x, 16 * sfxDownButton->transform.scale.y),
        true));

        sfxDownButton->AddComponent(new Button(sfxDownButton));

        sfxDownButton->GetComponent<Button>()->AddOnClickHandler([sfxText]() {
            sfxVolume = std::max(0, sfxVolume - 10);
            Mix_Volume(-1, sfxVolume);
            sfxText->GetComponent<TextRenderer>()->SetText(std::to_string(sfxVolume));
        });

        GameObjectManager::GetInstance()->AddGameObject(sfxLabel);
        GameObjectManager::GetInstance()->AddGameObject(sfxText);
        GameObjectManager::GetInstance()->AddGameObject(sfxUpButton);
        GameObjectManager::GetInstance()->AddGameObject(sfxDownButton);
#pragma endregion
        
        GameObject *quitButton = new GameObject("QuitButton");
        quitButton->transform.scale = Vector2(2, 2);

        quitButton->transform.position = Vector2(1280 - 32 * 2 / 2, 32 * 2 / 2);

        quitButton->AddComponent(new SpriteRenderer(quitButton, Vector2(32, 32), 0, LoadSpriteSheet("Assets/Sprites/Menu/Quit_button.png")));

        quitButton->AddComponent(new BoxCollider2D(quitButton, Vector2(0, 0), 
            Vector2(32 * quitButton->transform.scale.x, 32 * quitButton->transform.scale.y)
        ,true));

        quitButton->AddComponent(new Button(quitButton));
        quitButton->GetComponent<Button>()->AddOnClickHandler([this](){
                Game::state = MENU;
        });

        GameObjectManager::GetInstance()->AddGameObject(quitButton);

    });
    SceneManager::GetInstance()->AddScene(optionScene);

#ifndef MENU_DEBUG
    Scene *gameScene = new Scene("Game");
    gameScene->AssignLogic([gameScene, this]() {
        Game::state = GAME;
        // SoundManager::GetInstance()->PlayMusic("GameBgm");
#pragma region Tilemap
        GameObject *tilemap = new GameObject("Tilemap");

        tilemap->layer = CollisionMatrix::PROJECTILE;
        tilemap->transform.position = Vector2(0, 0);
        tilemap->transform.scale = Vector2(4, 4);
        tilemap->AddComponent(new Tilemap(tilemap, Vector2(16, 16), Vector2(112, 144),  false, 0, LoadSpriteSheet("Assets/Sprites/Tileset/tileset.png"), "Assets/tilemap_rebuilt.txt"));

        GameObjectManager::GetInstance()->AddGameObject(tilemap);
#pragma endregion

#pragma region Shell Setup
        GameObject *dropShellParticle = new GameObject("DropShellParticle");
        dropShellParticle->layer = CollisionMatrix::PARTICLE;
        dropShellParticle->transform.scale = Vector2(2, 2);

        dropShellParticle->AddComponent(new SpriteRenderer(dropShellParticle, Vector2(5, 5), 1, nullptr));
        dropShellParticle->AddComponent(new Rigidbody2D(dropShellParticle, 1, 0.025, 0, 1.0));
        dropShellParticle->AddComponent(new CircleCollider2D(dropShellParticle, Vector2(0, 0), 3, false));
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
            GameObject *shell = new GameObject("Shell" + std::to_string(spawnID++));
            shell->transform.scale = Vector2(2, 2);
            shell->layer = CollisionMatrix::PROJECTILE;
            shell->transform.position = position;

            shell->AddComponent(new SpriteRenderer(shell, Vector2(5, 4), 10, LoadSpriteSheet("Assets/Sprites/Player/shell.png")));

            shell->AddComponent(new ParticleSystem(shell, shellParticle, 10, 500, -1 * direction, 0, 0));

            shell->AddComponent(new Rigidbody2D(shell, 1, 0.025, 0, 0.0));
            shell->AddComponent(new ShellBehavior(shell, lifeTime, speed, direction));
            shell->AddComponent(new RotateTowardVelocity(shell, Vector2(1, 0)));

            shell->AddComponent(new CircleCollider2D(shell, Vector2(0, 0),
                                                     5 * shell->transform.scale.x / 2, true));
            shell->GetComponent<CircleCollider2D>()->OnCollisionEnter.addHandler([shell](Collider2D *collider) {
                if (collider->layer == CollisionMatrix::ENEMY || collider->layer == CollisionMatrix::WALL) {
                    HPController *hpController = collider->gameObject->GetComponent<HPController>();
                    if (hpController) {
                        ShellBehavior *shellBehavior = shell->GetComponent<ShellBehavior>();
                        float distance = (shellBehavior->GetSender()->transform.position - shell->transform.position).Magnitude();

                        // Deals max dmg of 3, min dmg of 1, quadratic falloff
                        // 4 ranges of dmg, 3dmg in the first 1 / PLAYER_DMG_DROP units
                        int dmg = (int)(PLAYER_DAMAGE - std::min((distance - 1 / PLAYER_DMG_DROP) * PLAYER_DMG_DROP, (double)PLAYER_DAMAGE - 1));

                        std::cout << "Shell hit " << collider->gameObject->GetName() <<" for " << dmg << " damage" << std::endl;

                        hpController->TakeDamage(dmg);
                    }
                    GameObjectManager::GetInstance()->RemoveGameObject(shell->GetName());
                }
                if (collider->layer == CollisionMatrix::PUSHABLE){
                    Rigidbody2D *rb = collider->gameObject->GetComponent<Rigidbody2D>();
                    if (rb) {
                        rb->AddForce(shell->GetComponent<Rigidbody2D>()->velocity.Normalize() * PLAYER_HOOK_FORCE / PLAYER_SHOTGUN_PELLET);
                    }
                }

            });

            return shell;
        };

        GameObject *hookParticle = new GameObject("hookParticle");
        hookParticle->layer = CollisionMatrix::PARTICLE;
        hookParticle->transform.scale = Vector2(2, 2);

        hookParticle->AddComponent(new SpriteRenderer(hookParticle, Vector2(10, 10), 1, LoadSpriteSheet("Assets/Sprites/Player/hook_particle.png")));
        hookParticle->AddComponent(new Rigidbody2D(hookParticle, 1, 0.025, 0, 0.0));
        hookParticle->AddComponent(new CircleCollider2D(hookParticle, Vector2(0, 0), 3, true));

        auto CreateHook = [hookParticle](float speed, Vector2 direction, float lifeTime, Vector2 position){
            GameObject *hook = new GameObject("hook" + std::to_string(spawnID++));
            hook->transform.scale = Vector2(2, 2);
            hook->layer = CollisionMatrix::PROJECTILE;
            hook->transform.position = position;

            hook->AddComponent(new SpriteRenderer(hook, Vector2(16, 10), 10, LoadSpriteSheet("Assets/Sprites/Player/hook.png")));

            hook->AddComponent(new ParticleSystem(hook, hookParticle, 30, 500, -1 * direction, 0, 0));

            hook->AddComponent(new Rigidbody2D(hook, 1, 0.025, 0, 0.0));
            hook->AddComponent(new ShellBehavior(hook, lifeTime, speed, direction));
            hook->AddComponent(new RotateTowardVelocity(hook, Vector2(1, 0)));

            hook->AddComponent(new CircleCollider2D(hook, Vector2(0, 0),
                                                     5 * hook->transform.scale.x / 2, true));
            hook->GetComponent<CircleCollider2D>()->OnCollisionEnter.addHandler([hook](Collider2D *collider) {
                ShellBehavior *shellBehavior = hook->GetComponent<ShellBehavior>();
                
                HPController *hpController = collider->gameObject->GetComponent<HPController>();
                if (hpController){
                    if (hpController->IsStunned()){
                        return;
                    }
                    hpController->Stun(1000);
                }

                Rigidbody2D *rb = collider->gameObject->GetComponent<Rigidbody2D>();
                if (rb) {
                    collider->gameObject->transform.position = collider->gameObject->transform.position + Vector2(0, -1);

                    Vector2 direction = shellBehavior->GetSender()->transform.position - collider->gameObject->transform.position;
                    direction = Vector2(direction.x, direction.y - 4).Normalize();
                    rb->velocity = Vector2(0, 0);
                    rb->AddForce(direction * PLAYER_HOOK_FORCE);
                }

                GameObjectManager::GetInstance()->RemoveGameObject(hook->GetName());
            });

            return hook;
        };

#pragma endregion

#pragma region Player Setup
        GameObject *player = new GameObject("Player");
        player->layer = CollisionMatrix::PLAYER;
        //tile 9 3 for start
        //tile 88 13 for boss room
        player->transform.position = tilemap->GetComponent<Tilemap>()->GetPositionFromTile(9, 3);
        // player->transform.position = Vector2(640, 360);
        player->transform.scale = Vector2(2, 2);

        player->AddComponent(new SpriteRenderer(player, Vector2(35, 37), 10, nullptr));

        player->AddComponent(new Animator(player,
            {
                AnimationClip("Idle", "Assets/Sprites/Player/player_idle.png", Vector2(15, 27), 1000, true, 1.0, 0, 2),
                AnimationClip("Walk", "Assets/Sprites/Player/player_walking.png", Vector2(15, 27), 1000, true, 1.0, 0, 4),
            }));

        player->GetComponent<Animator>()->Play("Idle");

        player->AddComponent(new Rigidbody2D(player, 1, 0.025, 0, 1.0));

        player->AddComponent(new VelocityToAnimSpeedController(player, "Walk"));
        player->AddComponent(new FLipToVelocity(player, Vector2(1, 0)));

        Joystick *movementStick = dynamic_cast<Joystick *>(
            player->AddComponent(new Joystick(player, SDLK_w, SDLK_s, SDLK_a, SDLK_d)));

        player->AddComponent(new MovementController(player, PLAYER_SPEED, movementStick));

        player->AddComponent(new PlayerAnimController(player));

        player->AddComponent(new BoxCollider2D(player, Vector2(0, 0),
                                               Vector2(15 * player->transform.scale.x, 27 * player->transform.scale.y), false));

        ParticleSystem *shellDropPS = dynamic_cast<ParticleSystem *>(
            player->AddComponent(new ParticleSystem(player, dropShellParticle, 1, 5000, Vector2(0, -1), 10, 10))
        );
        shellDropPS->Stop();

        Joystick *aimStick = dynamic_cast<Joystick *>(
            player->AddComponent(new Joystick(player, SDLK_UP, SDLK_DOWN, SDLK_LEFT, SDLK_RIGHT)));

        PlayerWeapon *shotgun = dynamic_cast<PlayerWeapon *>(
            player->AddComponent(new PlayerWeapon(player, 40, 600, 1000, PLAYER_SHOTGUN_PELLET, 10, aimStick, shellDropPS, "Shotgun"))
        );
        shotgun->setSpawnFunction(CreateShell);

        ArsenalManager *arsenalManager = dynamic_cast<ArsenalManager *>(
            player->AddComponent(new ArsenalManager(player))
        );

        arsenalManager->AddWeapon(shotgun, SDLK_1);
    
        player->AddComponent(new JumpController(player, SDLK_SPACE, 14, 450, CollisionMatrix::WALL));
        player->GetComponent<JumpController>()->BindCollider(player->GetComponent<BoxCollider2D>());

        player->AddComponent(new HPController(player, PLAYER_HP, PLAYER_INVINCIBILITY_TIME));

        GameObjectManager::GetInstance()->AddGameObject(player);

        // Hurt particle
        GameObject *playerHurtParticle = new GameObject("PlayerHurtParticle");
        playerHurtParticle->layer = CollisionMatrix::PARTICLE;
        playerHurtParticle->transform.scale = Vector2(2, 2);

        playerHurtParticle->AddComponent(new SpriteRenderer(playerHurtParticle, Vector2(5, 5), 20, LoadSpriteSheet("Assets/Sprites/Player/player_dmg_particle.png")));
        playerHurtParticle->AddComponent(new Rigidbody2D(playerHurtParticle, 1, 0.025, 0, 1.0));
        playerHurtParticle->AddComponent(new CircleCollider2D(playerHurtParticle, Vector2(0, 0), 3, false));

        ParticleSystem *playerHurtParticleSystem = dynamic_cast<ParticleSystem *>(
            player->AddComponent(new ParticleSystem(player, playerHurtParticle, 50, 2000, Vector2(0, -1), 10, 360))
        );
        playerHurtParticleSystem->Stop();
        player->GetComponent<HPController>()->OnDamage.addHandler([playerHurtParticleSystem]() {
            playerHurtParticleSystem->Emit(5);
        });

        player->GetComponent<HPController>()->OnHPChange.addHandler([player, playerHurtParticleSystem]() {
            std::cout << "Player HP: " << player->GetComponent<HPController>()->GetCurrentHP() << std::endl;
        });

        player->GetComponent<HPController>()->OnDeath.addHandler([this]() {
            Game::state = GAMEOVER;
        });

        player->AddComponent(new CoinCollector(player));

#pragma endregion

#pragma region Gun Setup
        GameObject *gun = new GameObject("Gun");
        gun->transform.position = Vector2(640, 100);
        gun->transform.scale = Vector2(1.5, 1.5);

        gun->AddComponent(new SpriteRenderer(gun, Vector2(16, 16), 11, LoadSpriteSheet("Assets/Sprites/Player/gun.png")));

        gun->AddComponent(new Orbit(gun, player, 25, Vector2(1, 0), aimStick));

        player->GetComponent<HPController>()->OnDeath.addHandler([gun]() {
            gun->AddComponent(new AutoDestroy(gun, 5000));
            gun->AddComponent(new CircleCollider2D(gun, Vector2(0, 0), 8, true));
            gun->AddComponent(new Rigidbody2D(gun, 1, 0.025, 0, 1.0));

            gun->GetComponent<Rigidbody2D>()->AddForce(Vector2(0, -1) * POWER_UP_POP_UP_FORCE);
            gun->GetComponent<Orbit>()->enabled = false;
        });

        GameObjectManager::GetInstance()->AddGameObject(gun);
#pragma endregion

#pragma region Camera Setup
        GameObject *camera = new GameObject("Camera");
        camera->layer = CollisionMatrix::CAMERA;
        camera->transform.position = Vector2(640, 360);

        camera->AddComponent(new BoxCollider2D(camera, Vector2(0, 0), Vector2(1280, 720), true));
        camera->AddComponent(new Rigidbody2D(camera, 1, 0.5, 0, 0.0));
        camera->AddComponent(new Camera(camera, player, Vector2(1280, 720), Vector2(0, -50), 10, Vector2(50, 70)));

        Game::CAMERA = camera;

        GameObjectManager::GetInstance()->AddGameObject(camera);
#pragma endregion

#pragma region Powerup
        auto CreateHeal = [player](Vector2 position){
            GameObject *heal = new GameObject("Heal" + std::to_string(spawnID++));
            heal->layer = CollisionMatrix::POWERUP;

            heal->transform.position = position;
            heal->transform.scale = Vector2(2, 2);

            heal->AddComponent(new SpriteRenderer(heal, Vector2(0, 0), 5));
            heal->AddComponent(new Animator(heal,
            {
                AnimationClip("Idle", "Assets/Sprites/Powerup/heart.png", Vector2(19, 18), 1000, false, 1.0, 0, 0),
            }));
            heal->GetComponent<Animator>()->Play("Idle");

            heal->AddComponent(new Rigidbody2D(heal, 1, 0.025, 0, 1.0));

            // Trigger
            heal->AddComponent(new BoxCollider2D(heal, Vector2(0, 0),
                Vector2(19 * heal->transform.scale.x, 18 * heal->transform.scale.y), true));
            
            heal->AddComponent(new PowerUp(heal, CollisionMatrix::PLAYER, nullptr, 1));

            // Physic
            BoxCollider2D *physCol = dynamic_cast<BoxCollider2D *>(
                heal->AddComponent(new BoxCollider2D(heal, Vector2(0, 0),
                    Vector2(19 * heal->transform.scale.x, 18 * heal->transform.scale.y), false))
            );

            physCol->layer = CollisionMatrix::PARTICLE;

            return heal;
        };

        auto CreateHookUpgrade = [player, aimStick, CreateHook](Vector2 position){
            GameObject *hookUpgrade = new GameObject("HookUpgrade" + std::to_string(spawnID++));
            hookUpgrade->layer = CollisionMatrix::POWERUP;

            hookUpgrade->transform.position = position;
            hookUpgrade->transform.scale = Vector2(2, 2);

            hookUpgrade->AddComponent(new SpriteRenderer(hookUpgrade, Vector2(19, 18), 5, LoadSpriteSheet("Assets/Sprites/Player/hook.png")));
            hookUpgrade->AddComponent(new Rigidbody2D(hookUpgrade, 1, 0.025, 0, 1.0));

            // Trigger
            hookUpgrade->AddComponent(new BoxCollider2D(hookUpgrade, Vector2(0, 0),
                Vector2(19 * hookUpgrade->transform.scale.x, 18 * hookUpgrade->transform.scale.y), true));
            
            auto UnlockHook = [aimStick, CreateHook](GameObject *player){
                PlayerWeapon *meatHook = dynamic_cast<PlayerWeapon *>(
                    player->AddComponent(new PlayerWeapon(player, 30, 600, 3000, 1, 0, aimStick, nullptr, "HookShoot"))
                );
                meatHook->setSpawnFunction(CreateHook);
                player->GetComponent<ArsenalManager>()->AddWeapon(meatHook, SDLK_2);
                std::cout << "Hook unlocked" << std::endl;
            };

            hookUpgrade->AddComponent(new PowerUp(hookUpgrade, CollisionMatrix::PLAYER, UnlockHook, 0));

            // Physic
            BoxCollider2D *physCol = dynamic_cast<BoxCollider2D *>(
                hookUpgrade->AddComponent(new BoxCollider2D(hookUpgrade, Vector2(0, 0),
                    Vector2(19 * hookUpgrade->transform.scale.x, 18 * hookUpgrade->transform.scale.y), false))
            );

            physCol->layer = CollisionMatrix::PARTICLE;

            return hookUpgrade;
        };

        auto CreateBootUpgrade = [player](Vector2 position){
            GameObject *bootUpgrade = new GameObject("BootUpgrade" + std::to_string(spawnID++));
            bootUpgrade->layer = CollisionMatrix::POWERUP;

            bootUpgrade->transform.position = position;
            bootUpgrade->transform.scale = Vector2(3, 3);

            bootUpgrade->AddComponent(new SpriteRenderer(bootUpgrade, Vector2(13, 14), 5, LoadSpriteSheet("Assets/Sprites/Powerup/spikeboot.png")));
            bootUpgrade->AddComponent(new Rigidbody2D(bootUpgrade, 1, 0.025, 0, 1.0));

            // Trigger
            bootUpgrade->AddComponent(new BoxCollider2D(bootUpgrade, Vector2(0, 0),
                Vector2(13 * bootUpgrade->transform.scale.x, 14 * bootUpgrade->transform.scale.y), true));
            
            auto UnlockBoot = [player](GameObject *player){
                player->GetComponent<JumpController>()->SetEnableWallJump(true);
                std::cout << "Spike boot unlocked" << std::endl;
            };

            bootUpgrade->AddComponent(new PowerUp(bootUpgrade, CollisionMatrix::PLAYER, UnlockBoot, 0));

            // Physic
            BoxCollider2D *physCol = dynamic_cast<BoxCollider2D *>(
                bootUpgrade->AddComponent(new BoxCollider2D(bootUpgrade, Vector2(0, 0),
                    Vector2(13 * bootUpgrade->transform.scale.x, 14 * bootUpgrade->transform.scale.y), false))
            );

            physCol->layer = CollisionMatrix::PARTICLE;

            return bootUpgrade;
        };

        auto CreateShield = [player](Vector2 position){
            GameObject *shield = new GameObject("Shield" + std::to_string(spawnID++));
            shield->layer = CollisionMatrix::POWERUP;

            shield->transform.position = position;
            shield->transform.scale = Vector2(2, 2);

            shield->AddComponent(new SpriteRenderer(shield, Vector2(18, 18), 5, LoadSpriteSheet("Assets/Sprites/Powerup/shield.png")));
            shield->AddComponent(new Rigidbody2D(shield, 1, 0.025, 0, 1.0));

            // Trigger
            shield->AddComponent(new BoxCollider2D(shield, Vector2(0, 0),
                Vector2(18 * shield->transform.scale.x, 18 * shield->transform.scale.y), true));
            
            auto UnlockShield = [player](GameObject *player){
                player->GetComponent<HPController>()->SetInvincible(true);
                std::cout << "Shield unlocked" << std::endl;
            };

            shield->AddComponent(new PowerUp(shield, CollisionMatrix::PLAYER, UnlockShield, 0));

            // Physic
            BoxCollider2D *physCol = dynamic_cast<BoxCollider2D *>(
                shield->AddComponent(new BoxCollider2D(shield, Vector2(0, 0),
                    Vector2(18 * shield->transform.scale.x, 18 * shield->transform.scale.y), false))
            );

            physCol->layer = CollisionMatrix::PARTICLE;

            return shield;
        };

        auto CreatePowerUpBox = [](Vector2 position, std::function<GameObject *(Vector2 position)> powerUpFunction){
            GameObject *powerUpBox = new GameObject("PowerUpBox" + std::to_string(spawnID++));
            powerUpBox->layer = CollisionMatrix::WALL;

            powerUpBox->transform.position = position;
            powerUpBox->transform.scale = Vector2(2, 2);

            powerUpBox->AddComponent(new SpriteRenderer(powerUpBox, Vector2(18, 18), 5, LoadSpriteSheet("Assets/Sprites/Powerup/PowerTile.png")));
            powerUpBox->AddComponent(new Rigidbody2D(powerUpBox, 1, 0.025, 0, 1.0));

            powerUpBox->AddComponent(new PowerUpBox(powerUpBox, powerUpFunction));

            // Trigger
            powerUpBox->AddComponent(new BoxCollider2D(powerUpBox, Vector2(0, 0),
                Vector2(18 * powerUpBox->transform.scale.x, 18 * powerUpBox->transform.scale.y), true));
            powerUpBox->GetComponent<BoxCollider2D>()->layer = CollisionMatrix::POWERUP;
            
            powerUpBox->GetComponent<BoxCollider2D>()->OnCollisionEnter.addHandler([powerUpBox](Collider2D *collider) {
                if (collider->layer == CollisionMatrix::PROJECTILE){
                    GameObject *powerup = powerUpBox->GetComponent<PowerUpBox>()->GetPowerUp();

                    if (powerup){
                        powerup->GetComponent<Rigidbody2D>()->AddForce(POWER_UP_POP_UP_FORCE * Vector2(0, -1));

                        GameObjectManager::GetInstance()->AddGameObject(powerup);
                    }
                    GameObjectManager::GetInstance()->RemoveGameObject(powerUpBox->GetName());
                }
            });
            

            // Physic
            BoxCollider2D *physCol = dynamic_cast<BoxCollider2D *>(
                powerUpBox->AddComponent(new BoxCollider2D(powerUpBox, Vector2(0, 0),
                    Vector2(18 * powerUpBox->transform.scale.x, 18 * powerUpBox->transform.scale.y), false))
            );

            physCol->layer = CollisionMatrix::PARTICLE;

            return powerUpBox;
        };

        auto CreateCoin = [player](Vector2 position){
            GameObject *coin = new GameObject("Coin" + std::to_string(spawnID++));
            coin->layer = CollisionMatrix::POWERUP;

            coin->transform.position = position;
            coin->transform.scale = Vector2(2, 2);

            coin->AddComponent(new SpriteRenderer(coin, Vector2(14, 18), 5, nullptr));

            coin->AddComponent(new Animator(coin,
            {
                AnimationClip("Idle", "Assets/Sprites/Powerup/coin.png", Vector2(14, 18), 1000, true, 1.0, 0, 3),
            }));

            coin->AddComponent(new Rigidbody2D(coin, 1, 0.025, 0, 1.0));

            // Trigger
            coin->AddComponent(new BoxCollider2D(coin, Vector2(0, 0),
                Vector2(18 * coin->transform.scale.x, 18 * coin->transform.scale.y), true));
            
            auto CollectCoin = [coin](GameObject *player){
                CoinCollector *cc = player->GetComponent<CoinCollector>();
                if (!cc) return;
                cc->AddCoin();
                std::cout << "Coin collected" << std::endl;
                std::cout << player->GetComponent<CoinCollector>()->GetCoinCount() << std::endl;
                GameObjectManager::GetInstance()->RemoveGameObject(coin->GetName());
            };

            coin->AddComponent(new PowerUp(coin, CollisionMatrix::PLAYER, CollectCoin, 0));

            // Physic
            BoxCollider2D *physCol = dynamic_cast<BoxCollider2D *>(
                coin->AddComponent(new BoxCollider2D(coin, Vector2(0, 0),
                    Vector2(18 * coin->transform.scale.x, 18 * coin->transform.scale.y), false))
            );

            physCol->layer = CollisionMatrix::PARTICLE;

            return coin;
        };

        GameObjectManager::GetInstance()->AddGameObject(CreateCoin(
            tilemap->GetComponent<Tilemap>()->GetPositionFromTile(15, 4)
        ));

        GameObjectManager::GetInstance()->AddGameObject(CreateCoin(
            tilemap->GetComponent<Tilemap>()->GetPositionFromTile(22, 4)
        ));

        GameObjectManager::GetInstance()->AddGameObject(CreateCoin(
            tilemap->GetComponent<Tilemap>()->GetPositionFromTile(26, 4)
        ));


        GameObjectManager::GetInstance()->AddGameObject(CreatePowerUpBox(
            tilemap->GetComponent<Tilemap>()->GetPositionFromTile(15, 8), CreateHeal
            // tilemap->GetComponent<Tilemap>()->GetPositionFromTile(88, 13), CreateHeal
        ));

        GameObjectManager::GetInstance()->AddGameObject(CreatePowerUpBox(
            tilemap->GetComponent<Tilemap>()->GetPositionFromTile(36, 4), CreateHeal
            // tilemap->GetComponent<Tilemap>()->GetPositionFromTile(88, 13), CreateHeal
        ));


        GameObjectManager::GetInstance()->AddGameObject(CreatePowerUpBox(
            tilemap->GetComponent<Tilemap>()->GetPositionFromTile(25, 8), CreateHookUpgrade
            // tilemap->GetComponent<Tilemap>()->GetPositionFromTile(88, 13), CreateHookUpgrade
        ));

        GameObjectManager::GetInstance()->AddGameObject(CreatePowerUpBox(
            tilemap->GetComponent<Tilemap>()->GetPositionFromTile(41, 3), CreateBootUpgrade
            // tilemap->GetComponent<Tilemap>()->GetPositionFromTile(88, 13), CreateBootUpgrade
        ));

        GameObjectManager::GetInstance()->AddGameObject(CreatePowerUpBox(
            tilemap->GetComponent<Tilemap>()->GetPositionFromTile(60, 4), CreateShield
        ));
        
        GameObjectManager::GetInstance()->AddGameObject(CreatePowerUpBox(
            tilemap->GetComponent<Tilemap>()->GetPositionFromTile(88, 13), CreateShield
        ));
#pragma endregion

#pragma region Physic Box
        auto CreateBox = [](float mass, Vector2 scale, Vector2 position){
            GameObject *box = new GameObject("Box" + std::to_string(spawnID++));
            box->layer = CollisionMatrix::WALL;
            box->transform.position = position;
            box->transform.scale = scale;

            box->AddComponent(new SpriteRenderer(box, Vector2(16, 16), 5, LoadSpriteSheet("Assets/Sprites/Powerup/PhysicTile.png")));
            box->AddComponent(new Rigidbody2D(box, mass, 0.025, 0.0, 1.0));

            box->AddComponent(new BoxCollider2D(box, Vector2(0, 0),
                Vector2(16 * box->transform.scale.x, 16 * box->transform.scale.y), false));
            box->GetComponent<BoxCollider2D>()->layer = CollisionMatrix::PUSHABLE;

            return box;
        };

        GameObjectManager::GetInstance()->AddGameObject(
            CreateBox(3, Vector2(12, 12), tilemap->GetComponent<Tilemap>()->GetPositionFromTile(59, 4))
        );
#pragma endregion
    
#pragma region Gate
        auto CreateGate = [](Vector2 position, Vector2 destination){
            GameObject *gate = new GameObject("Gate" + std::to_string(spawnID++));
            gate->layer = CollisionMatrix::GATE;
            gate->transform.position = position;
            gate->transform.scale = Vector2(4, 4);

            gate->AddComponent(new SpriteRenderer(gate, Vector2(32, 32), 5, LoadSpriteSheet("Assets/Sprites/Powerup/gate.png")));
            gate->AddComponent(new Rigidbody2D(gate, 1, 0.025, 0, 1.0));

            gate->AddComponent(new BoxCollider2D(gate, Vector2(0, 0),
                Vector2(32 * gate->transform.scale.x / 2, 32 * gate->transform.scale.y), true));
            gate->GetComponent<BoxCollider2D>()->layer = CollisionMatrix::GATE;

            gate->GetComponent<BoxCollider2D>()->OnCollisionEnter.addHandler([destination](Collider2D *collider) {
                if (collider->layer == CollisionMatrix::PLAYER){
                    collider->gameObject->transform.position = destination;
                }
            });
            
            BoxCollider2D *physCol = dynamic_cast<BoxCollider2D *>(
                gate->AddComponent(new BoxCollider2D(gate, Vector2(0, 0),
                    Vector2(32 * gate->transform.scale.x / 2, 32 * gate->transform.scale.y), false))
            );
            physCol->layer = CollisionMatrix::PARTICLE;

            return gate;
        };

        GameObjectManager::GetInstance()->AddGameObject(
            CreateGate(tilemap->GetComponent<Tilemap>()->GetPositionFromTile(74, 4), tilemap->GetComponent<Tilemap>()->GetPositionFromTile(87, 14))
        );
#pragma endregion

#pragma region Melee projectile setup

    auto CreateMeleeProjectile = [](Vector2 direction, float lifeTime, Vector2 position) {
        GameObject *meleeProjectile = new GameObject("MeleeProjectile" + std::to_string(spawnID++));
        meleeProjectile->layer = CollisionMatrix::E_PROJECTILE;
        meleeProjectile->transform.scale = Vector2(3, 3);
        meleeProjectile->transform.position = position;

        meleeProjectile->AddComponent(new SpriteRenderer(meleeProjectile, Vector2(32, 16), 20, nullptr));

        meleeProjectile->AddComponent(new Animator(meleeProjectile,
            {
                AnimationClip("Default", "Assets/Sprites/Enemy/melee_slash.png", Vector2(32, 16), lifeTime, false, 1.0, 0, 2),
            }));

        meleeProjectile->AddComponent(new AutoDestroy(meleeProjectile, lifeTime));

        meleeProjectile->AddComponent(new Rigidbody2D(meleeProjectile, 1, 0.025, 0, 0.0));

        meleeProjectile->AddComponent(new FLipToVelocity(meleeProjectile, Vector2(-1, 0)));

        meleeProjectile->GetComponent<Rigidbody2D>()->AddForce(direction * MELEE_SPEED / 10);

        meleeProjectile->AddComponent(new BoxCollider2D(meleeProjectile, Vector2(0, 0), 
            Vector2(32 * meleeProjectile->transform.scale.x, 16 * meleeProjectile->transform.scale.y), 
        true));

        meleeProjectile->GetComponent<BoxCollider2D>()->OnCollisionEnter.addHandler([meleeProjectile](Collider2D *collider) {
            if (collider->layer == CollisionMatrix::PLAYER) {
                HPController *hpController = collider->gameObject->GetComponent<HPController>();
                if (hpController)
                    hpController->TakeDamage(MELEE_DAMAGE);
            }
        });

        return meleeProjectile;
    };


#pragma endregion

#pragma region Enemy hurt particle setup
        GameObject *enemyHurtParticle = new GameObject("EnemyHurtParticle");
        enemyHurtParticle->layer = CollisionMatrix::PARTICLE;
        enemyHurtParticle->transform.scale = Vector2(2, 2);

        enemyHurtParticle->AddComponent(new SpriteRenderer(enemyHurtParticle, Vector2(5, 5), 20, LoadSpriteSheet("Assets/Sprites/Enemy/enemy_dmg_particle.png")));
        enemyHurtParticle->AddComponent(new Rigidbody2D(enemyHurtParticle, 1, 0.025, 0, 1.0));
        enemyHurtParticle->AddComponent(new CircleCollider2D(enemyHurtParticle, Vector2(0, 0), 3, false));
#pragma endregion

#pragma region Melee setup
        auto CreateMelee = [player, CreateMeleeProjectile, enemyHurtParticle, CreateCoin, CreateHeal](Vector2 position){
            GameObject *melee = new GameObject("Melee" + std::to_string(spawnID++));
            melee->layer = CollisionMatrix::ENEMY;
            melee->transform.position = position;
            melee->transform.scale = Vector2(2, 2);

            melee->AddComponent(new SpriteRenderer(melee, Vector2(0, 0), 5, nullptr));
            melee->AddComponent(new Animator(melee,
                                            {
                                                AnimationClip("Idle", "Assets/Sprites/Enemy/melee_enemy_idle-sheet.png", Vector2(24, 28), 1000, true, 1.0, 0, 3),
                                                AnimationClip("Walk", "Assets/Sprites/Enemy/melee_enemy_walking-sheet.png", Vector2(24, 28), 500, true, 1.0, 0, 2),
                                                AnimationClip("Attack", "Assets/Sprites/Enemy/melee_enemy_attack-sheet.png", Vector2(29, 33), 400, false, 1.0, 0, 4),
                                            }));
            melee->GetComponent<Animator>()->Play("Walk");

            melee->AddComponent(new Rigidbody2D(melee, 1, 0.025, 0, 1.0));
            melee->AddComponent(new VelocityToAnimSpeedController(melee, "Walk"));
            melee->AddComponent(new FLipToVelocity(melee, Vector2(-1, 0)));

            // melee->GetComponent<Rigidbody2D>()->AddForce(Vector2(10, 0));

            // Physic collider
            melee->AddComponent(new BoxCollider2D(melee, Vector2(0, 0),
                                                Vector2(15 * melee->transform.scale.x, 27 * melee->transform.scale.y), false));

            // Hitbox collider
            BoxCollider2D *melee_hitbox = dynamic_cast<BoxCollider2D *>(
                melee->AddComponent(new BoxCollider2D(melee, Vector2(0, 0),
                                                    Vector2(15 * melee->transform.scale.x, 27 * melee->transform.scale.y), true)));

            melee_hitbox->OnCollisionEnter.addHandler([melee](Collider2D *collider) {
                if (collider->layer == CollisionMatrix::PLAYER) {
                    HPController *hpController = collider->gameObject->GetComponent<HPController>();
                    if (hpController)
                        hpController->TakeDamage(MELEE_DAMAGE);
                }
            });
            melee_hitbox->layer = CollisionMatrix::E_PROJECTILE;

            melee->AddComponent(new HPController(melee, MELEE_HP, 0));

            ParticleSystem *enemyHurtParticleSystem = dynamic_cast<ParticleSystem *>(
                melee->AddComponent(new ParticleSystem(melee, enemyHurtParticle, 50, 2000, Vector2(0, -1), 10, 360)));
            enemyHurtParticleSystem->Stop();
            melee->GetComponent<HPController>()->OnDamage.addHandler([enemyHurtParticleSystem]() {
                enemyHurtParticleSystem->Emit(2);
            });
            melee->GetComponent<HPController>()->AddDropFunction(CreateCoin);
            melee->GetComponent<HPController>()->AddDropFunction(CreateHeal);

        
            melee->AddComponent(new MeleeAI(melee, MELEE_SPEED, MELEE_ATTACK_RANGE, MELEE_ATTACK_COOLDOWN));
            melee->GetComponent<MeleeAI>()->SetTarget(player);

            melee->GetComponent<MeleeAI>()->SetCreateAttack(CreateMeleeProjectile);

            // melee->AddComponent(new PositionTracker(melee));

            return melee;
        };

        GameObjectManager::GetInstance()->AddGameObject(CreateMelee(
            tilemap->GetComponent<Tilemap>()->GetPositionFromTile(12, 7)
        ));

        GameObjectManager::GetInstance()->AddGameObject(CreateMelee(
            tilemap->GetComponent<Tilemap>()->GetPositionFromTile(25, 7)
        ));

        GameObjectManager::GetInstance()->AddGameObject(CreateMelee(
            tilemap->GetComponent<Tilemap>()->GetPositionFromTile(30, 7)
        ));


#pragma endregion

#pragma region Ranged projectile setup

    auto CreateRangedProjectile = [shellParticle](Vector2 direction, float speed, float lifeTime, Vector2 position){
        GameObject *rangedProjectile = new GameObject("RangedProjectile" + std::to_string(spawnID++));
        rangedProjectile->layer = CollisionMatrix::E_PROJECTILE;
        rangedProjectile->transform.scale = Vector2(4, 4);
        rangedProjectile->transform.position = position;

        rangedProjectile->AddComponent(new SpriteRenderer(rangedProjectile, Vector2(3, 3), 20, LoadSpriteSheet("Assets/Sprites/Enemy/e_shell.png")));
        rangedProjectile->AddComponent(new Rigidbody2D(rangedProjectile, 1, 0.025, 0, 0.0));
        
        rangedProjectile->AddComponent(new ShellBehavior(rangedProjectile, lifeTime, speed, direction));

        rangedProjectile->AddComponent(new ParticleSystem(rangedProjectile, shellParticle, 10, 500, -1 * direction, 0, 0));

        rangedProjectile->AddComponent(new CircleCollider2D(rangedProjectile, Vector2(0, 0), 3 * rangedProjectile->transform.scale.x / 2, true));

        rangedProjectile->GetComponent<CircleCollider2D>()->OnCollisionEnter.addHandler([rangedProjectile](Collider2D *collider) {
            if (collider->layer == CollisionMatrix::PLAYER) {
                HPController *hpController = collider->gameObject->GetComponent<HPController>();
                if (hpController)
                    hpController->TakeDamage(RANGED_DAMAGE);
            }

            GameObjectManager::GetInstance()->RemoveGameObject(rangedProjectile->GetName());
        });

        rangedProjectile->AddComponent(new RotateTowardVelocity(rangedProjectile, Vector2(1, 0)));

        return rangedProjectile;
    };
#pragma endregion

#pragma region Ranged setup
        auto CreateRanged = [player, CreateRangedProjectile, enemyHurtParticle, CreateCoin, CreateHeal](Vector2 position){
            GameObject *ranged = new GameObject("Ranged" + std::to_string(spawnID++));
            ranged->layer = CollisionMatrix::ENEMY;
            ranged->transform.position = position;
            ranged->transform.scale = Vector2(2, 2);

            ranged->AddComponent(new SpriteRenderer(ranged, Vector2(0, 0), 5, nullptr));
            ranged->AddComponent(new Animator(ranged,
            {
                AnimationClip("Walk", "Assets/Sprites/Enemy/proj_enemy.png", Vector2(48, 48), 1000, true, 1.0, 0, 3),
            }));
            ranged->GetComponent<Animator>()->Play("Walk");

            ranged->AddComponent(new Rigidbody2D(ranged, 1, 0.025, 0.8, 0));
            ranged->AddComponent(new FLipToVelocity(ranged, Vector2(-1, 0)));

            // Physic collider
            ranged->AddComponent(new BoxCollider2D(ranged, Vector2(0, 0),
                                                Vector2(48 * ranged->transform.scale.x, 48 * ranged->transform.scale.y), false));

            // Hitbox collider
            BoxCollider2D *ranged_hitbox = dynamic_cast<BoxCollider2D *>(
                ranged->AddComponent(new BoxCollider2D(ranged, Vector2(0, 0),
                                                    Vector2(48 * ranged->transform.scale.x, 48 * ranged->transform.scale.y), true)));

            ranged_hitbox->OnCollisionEnter.addHandler([ranged](Collider2D *collider) {
                if (collider->layer == CollisionMatrix::PLAYER) {
                    HPController *hpController = collider->gameObject->GetComponent<HPController>();
                    if (hpController)
                        hpController->TakeDamage(RANGED_DAMAGE);
                }
            });
            ranged_hitbox->layer = CollisionMatrix::E_PROJECTILE;

            ranged->AddComponent(new HPController(ranged, RANGED_HP, 0));

            ParticleSystem *enemyHurtParticleSystem = dynamic_cast<ParticleSystem *>(
                ranged->AddComponent(new ParticleSystem(ranged, enemyHurtParticle, 50, 2000, Vector2(0, -1), 10, 360)));
            enemyHurtParticleSystem->Stop();
            ranged->GetComponent<HPController>()->OnDamage.addHandler([enemyHurtParticleSystem]() {
                enemyHurtParticleSystem->Emit(2);
            });
            ranged->GetComponent<HPController>()->AddDropFunction(CreateCoin);
            ranged->GetComponent<HPController>()->AddDropFunction(CreateHeal);

            ranged->AddComponent(new RangedAI(ranged, RANGED_SPEED, RANGED_ATTACK_RANGE, RANGED_ATTACK_COOLDOWN));
            ranged->GetComponent<RangedAI>()->SetTarget(player);
            ranged->GetComponent<RangedAI>()->SetCreateAttack(CreateRangedProjectile);

            return ranged;
        };
            
        GameObjectManager::GetInstance()->AddGameObject(CreateRanged(
            tilemap->GetComponent<Tilemap>()->GetPositionFromTile(24, 4)
        ));

#pragma endregion

#pragma region Moai projectile
        auto CreateMoaiProjectile = [shellParticle](Vector2 direction, float speed, float lifeTime, Vector2 position){
            GameObject *moaiProjectile = new GameObject("MoaiProjectile" + std::to_string(spawnID++));
            moaiProjectile->layer = CollisionMatrix::E_PROJECTILE;
            moaiProjectile->transform.scale = Vector2(4, 4);
            moaiProjectile->transform.position = position;

            moaiProjectile->AddComponent(new SpriteRenderer(moaiProjectile, Vector2(0, 0), 20, nullptr));

            moaiProjectile->AddComponent(new Animator(moaiProjectile,
            {
                AnimationClip("Default", "Assets/Sprites/Enemy/moai_projectile.png", Vector2(32, 32), 700, true, 1.0, 0, 3),
            }));
            moaiProjectile->GetComponent<Animator>()->Play("Default");

            moaiProjectile->AddComponent(new Rigidbody2D(moaiProjectile, 1, 0.025, 0, 0.0));
            
            moaiProjectile->AddComponent(new ShellBehavior(moaiProjectile, lifeTime, speed, direction));

            moaiProjectile->AddComponent(new ParticleSystem(moaiProjectile, shellParticle, 10, 500, -1 * direction, 0, 0));

            moaiProjectile->AddComponent(new CircleCollider2D(moaiProjectile, Vector2(0, 0), 32 * moaiProjectile->transform.scale.x / 2, true));
        
            moaiProjectile->GetComponent<CircleCollider2D>()->OnCollisionEnter.addHandler([moaiProjectile](Collider2D *collider) {
                if (collider->layer == CollisionMatrix::PLAYER) {
                    HPController *hpController = collider->gameObject->GetComponent<HPController>();
                    if (hpController)
                        hpController->TakeDamage(MOAI_DAMAGE);
                }

                GameObjectManager::GetInstance()->RemoveGameObject(moaiProjectile->GetName());
            });
            moaiProjectile->AddComponent(new MoaiProjectileDelay(moaiProjectile, rand() % 200));

            return moaiProjectile;
        };
#pragma endregion

#pragma region Moai setup
        auto CreateMoai = [player, CreateMelee, CreateRanged, CreateMoaiProjectile, enemyHurtParticle](Vector2 position){
            GameObject *moai = new GameObject("Moai" + std::to_string(spawnID++));
            moai->layer = CollisionMatrix::ENEMY;
            moai->transform.position = position;
            moai->transform.scale = Vector2(2, 2);

            moai->AddComponent(new SpriteRenderer(moai, Vector2(87, 112), 5, nullptr));

            moai->AddComponent(new Animator(moai,
            {
                AnimationClip("Idle", "Assets/Sprites/Enemy/moai_idle.png", Vector2(87, 112), 1000, true, 2.0, 0, 3),
                AnimationClip("Attack", "Assets/Sprites/Enemy/moai_charge.png", Vector2(87, 112), 5000, false, 1.0, 0, 3),
            }));
            moai->GetComponent<Animator>()->Play("Idle");

            moai->AddComponent(new Rigidbody2D(moai, 1, 0.025, 0.0, 0.0));

            moai->AddComponent(new FLipToVelocity(moai, Vector2(1, 0)));
            //Physic collider
            moai->AddComponent(new BoxCollider2D(moai, Vector2(0, 0),
                Vector2(87 * moai->transform.scale.x, 112 * moai->transform.scale.y)
            , false));

            // Hitbox collider
            BoxCollider2D *moai_hitbox = dynamic_cast<BoxCollider2D *>(
                moai->AddComponent(new BoxCollider2D(moai, Vector2(0, 0),
                    Vector2(87 * moai->transform.scale.x, 112 * moai->transform.scale.y), true))
            );
            moai_hitbox->OnCollisionEnter.addHandler([moai](Collider2D *collider) {
                if (collider->layer == CollisionMatrix::PLAYER) {
                    HPController *hpController = collider->gameObject->GetComponent<HPController>();
                    if (hpController)
                        hpController->TakeDamage(MOAI_DAMAGE);
                }
            });
            moai_hitbox->layer = CollisionMatrix::E_PROJECTILE;

            moai->AddComponent(new HPController(moai, MOAI_HP, 0));

            ParticleSystem *enemyHurtParticleSystem = dynamic_cast<ParticleSystem *>(
                moai->AddComponent(new ParticleSystem(moai, enemyHurtParticle, 50, 2000, Vector2(0, -1), 20, 360)));
            enemyHurtParticleSystem->Stop();
            
            moai->GetComponent<HPController>()->OnDamage.addHandler([enemyHurtParticleSystem]() {
                enemyHurtParticleSystem->Emit(2);
            });

            moai->AddComponent(new MoaiAI(moai, MOAI_SPEED, MOAI_DETECT_RANGE, MOAI_ATTACK_RANGE, MOAI_ATTACK_COOLDOWN));

            moai->GetComponent<MoaiAI>()->AddSpawnFunction(CreateMelee);
            moai->GetComponent<MoaiAI>()->AddSpawnFunction(CreateRanged);
            moai->GetComponent<MoaiAI>()->SetCreateAttack(CreateMoaiProjectile);

            moai->GetComponent<MoaiAI>()->SetTarget(player);
            return moai;
        };

        GameObjectManager::GetInstance()->AddGameObject(CreateMoai(
            tilemap->GetComponent<Tilemap>()->GetPositionFromTile(99, 5)
        ));

        
#pragma endregion

#pragma region UI
        GameObject *coinUI = new GameObject("CoinUI");
        coinUI->transform.position = Vector2(0, 0);
        coinUI->transform.scale = Vector2(2, 2);

        //Dummy sprite
        coinUI->AddComponent(new SpriteRenderer(coinUI, Vector2(100, 20), 20, nullptr));
        coinUI->AddComponent(new TextRenderer(coinUI, "Coins: 0", SDL_Color{255, 255, 255, 255}, 20, "Assets/Fonts/arial.ttf"));

        player->GetComponent<CoinCollector>()->OnCoinCollect.addHandler([coinUI, player](){
            int coinCount = player->GetComponent<CoinCollector>()->GetCoinCount();
            coinUI->GetComponent<TextRenderer>()->SetText("Coins: " + std::to_string(coinCount));
            Game::coin = coinCount;
        });

        coinUI->AddComponent(new BindToCamera(coinUI, Vector2(WIDTH / 2 - 300, - HEIGHT / 2 + 20)));

        GameObjectManager::GetInstance()->AddGameObject(coinUI);

        GameObject *HPBar = new GameObject("HPBar");
        HPBar->transform.position = Vector2(0, 0);
        HPBar->transform.scale = Vector2(2, 2);

        HPBar->AddComponent(new SpriteRenderer(HPBar, Vector2(100, 10), 20, LoadSpriteSheet("Assets/Sprites/Menu/HPBar.png")));

        HPBar->AddComponent(new Animator(HPBar,
        {
            AnimationClip("3", "Assets/Sprites/Menu/HPBar.png", Vector2(57, 18), 1000, true, 1.0, 0, 0),
            AnimationClip("2", "Assets/Sprites/Menu/HPBar.png", Vector2(57, 18), 1000, true, 1.0, 1, 1),
            AnimationClip("1", "Assets/Sprites/Menu/HPBar.png", Vector2(57, 18), 1000, true, 1.0, 2, 2),
            AnimationClip("0", "Assets/Sprites/Menu/HPBar.png", Vector2(57, 18), 1000, true, 1.0, 3, 3),
        }));
        HPBar->GetComponent<Animator>()->Play("3");

        player->GetComponent<HPController>()->OnHPChange.addHandler([HPBar, player](){
            int hp = player->GetComponent<HPController>()->GetCurrentHP();
            HPBar->GetComponent<Animator>()->Play(std::to_string(hp));
        });

        HPBar->AddComponent(new BindToCamera(HPBar, Vector2(- WIDTH / 2 + 70, - HEIGHT / 2 + 20)));

        GameObjectManager::GetInstance()->AddGameObject(HPBar);

#pragma endregion

    });
    SceneManager::GetInstance()->AddScene(gameScene);
#endif

    // Game Over Scene
    Scene *gameOverScene = new Scene("GameOver");
    gameOverScene->AssignLogic([gameOverScene, this]() {
        Game::state = GAMEOVER;

        SoundManager::GetInstance()->PlaySound("GameOver");

        GameObject *gameOver = new GameObject("GameOver");
        gameOver->transform.position = Vector2(640, 200);
        
        gameOver->AddComponent(new TextRenderer(gameOver, "Game Over", SDL_Color{255, 255, 255, 255}, 60, "Assets/Fonts/arial.ttf"));

        GameObjectManager::GetInstance()->AddGameObject(gameOver);

        GameObject *score = new GameObject("Score");
        score->transform.position = Vector2(640, 300);

        score->AddComponent(new TextRenderer(score, "Coin: " + Game::coin, SDL_Color{255, 255, 255, 255}, 40, "Assets/Fonts/arial.ttf"));
        GameObjectManager::GetInstance()->AddGameObject(score);

        GameObject *quitButton = new GameObject("QuitButton");
        quitButton->transform.scale = Vector2(2, 2);

        quitButton->transform.position = Vector2(1280 - 32 * 2 / 2, 32 * 2 / 2);

        quitButton->AddComponent(new SpriteRenderer(quitButton, Vector2(32, 32), 0, LoadSpriteSheet("Assets/Sprites/Menu/Quit_button.png")));

        quitButton->AddComponent(new BoxCollider2D(quitButton, Vector2(0, 0), 
            Vector2(32 * quitButton->transform.scale.x, 32 * quitButton->transform.scale.y)
        ,true));

        quitButton->AddComponent(new Button(quitButton));
        quitButton->GetComponent<Button>()->AddOnClickHandler([this](){
            Game::state = MENU;
        });

        GameObjectManager::GetInstance()->AddGameObject(quitButton);
    });
    SceneManager::GetInstance()->AddScene(gameOverScene);

    SceneManager::GetInstance()->LoadScene("Menu");
}

void Game::handleEvents() {

    SDL_PollEvent(&Game::event);

    if (event.type == SDL_QUIT) {
        isRunning = false;
        return;
    }

    if (event.type == SDL_KEYDOWN) {
        if (event.key.keysym.sym == SDLK_ESCAPE) {
            state = MENU;
            return;
        }
    }
}

void Game::handleSceneChange() {
    switch (state) {
    case MENU:
        if (SceneManager::GetInstance()->GetCurrentScene()->GetName() != "Menu"){
            Game::CAMERA = nullptr;
            SceneManager::GetInstance()->LoadScene("Menu");
        }
        break;
    case OPTION:
        if (SceneManager::GetInstance()->GetCurrentScene()->GetName() != "Option"){
            Game::CAMERA = nullptr;
            SceneManager::GetInstance()->LoadScene("Option");
        }
        break;
    case GAME:
        if (SceneManager::GetInstance()->GetCurrentScene()->GetName() != "Game"){
            Game::coin = 0;
            SceneManager::GetInstance()->LoadScene("Game");
        }
        break;
    case GAMEOVER:
        if (SceneManager::GetInstance()->GetCurrentScene()->GetName() != "GameOver"){
            Game::CAMERA = nullptr;
            SceneManager::GetInstance()->LoadScene("GameOver");
        }
        break;
    }
}

void Game::update() {
    // TEST
    // TEST

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
