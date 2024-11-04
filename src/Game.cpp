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
    objectInit();
}

GameObject *player = new GameObject("Player");

void Game::objectInit() {

    // Add sounds and music
    //  SoundManager::GetInstance();
    //  SoundManager::GetInstance()->AddMusic("MenuBgm", "Assets/SFX/fairyfountain.mp3", 100);
    //  SoundManager::GetInstance()->AddMusic("GameBgm", "Assets/SFX/papyrus.mp3", 32);

    // SoundManager::GetInstance()->AddSound("ball_bounce", "Assets/SFX/ball_bounce.mp3", 128);
    // SoundManager::GetInstance()->AddSound("ball_kick", "Assets/SFX/ball_kick.mp3", 128);

    // SoundManager::GetInstance()->AddSound("Game_Over", "Assets/SFX/gameover.mp3", 128);
    // SoundManager::GetInstance()->AddSound("Goal", "Assets/SFX/score.mp3", 64);

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

#pragma region Wall Setup
        // GameObject *wall = new GameObject("Wall");
        // wall->layer = CollisionMatrix::WALL;
        // wall->transform.rotation = 0;
        // wall->transform.position = Vector2(1000, 700);
        // wall->transform.scale = Vector2(500, 2);
        // wall->AddComponent(new SpriteRenderer(wall, Vector2(15, 30), 0, LoadSpriteSheet("Assets/wall.png")));
        // wall->AddComponent(new BoxCollider2D(wall, Vector2(0, 0), Vector2(7500, 60), false));

        // GameObjectManager::GetInstance()->AddGameObject(wall);
        // // Left Wall
        // GameObject *leftWall = new GameObject("LeftWall");
        // leftWall->layer = CollisionMatrix::WALL;
        // leftWall->transform.rotation = 0;
        // leftWall->transform.position = Vector2(-200, 400); // Adjust position as needed
        // leftWall->transform.scale = Vector2(3, 100);       // Adjust scale as needed
        // leftWall->AddComponent(new SpriteRenderer(leftWall, Vector2(15, 30), 0, LoadSpriteSheet("Assets/wall.png")));
        // leftWall->AddComponent(new BoxCollider2D(leftWall, Vector2(0, 0), Vector2(45, 3000), false));

        // GameObjectManager::GetInstance()->AddGameObject(leftWall);

        // // Right Wall
        // GameObject *rightWall = new GameObject("RightWall");
        // rightWall->layer = CollisionMatrix::WALL;
        // rightWall->transform.rotation = 0;
        // rightWall->transform.position = Vector2(2000, 400); // Adjust position as needed
        // rightWall->transform.scale = Vector2(2, 100);       // Adjust scale as needed
        // rightWall->AddComponent(new SpriteRenderer(rightWall, Vector2(15, 30), 0, LoadSpriteSheet("Assets/wall.png")));
        // rightWall->AddComponent(new BoxCollider2D(rightWall, Vector2(0, 0), Vector2(30, 1500), false));

        // GameObjectManager::GetInstance()->AddGameObject(rightWall);

        // // AI Test Walls
        // GameObject *lowWall1 = new GameObject("LowWall1");
        // lowWall1->layer = CollisionMatrix::WALL;
        // lowWall1->transform.rotation = 0;
        // lowWall1->transform.position = Vector2(50, 600); // Adjust position as needed
        // lowWall1->transform.scale = Vector2(5, 5);       // Adjust scale as needed
        // lowWall1->AddComponent(new SpriteRenderer(lowWall1, Vector2(15, 30), 0, LoadSpriteSheet("Assets/wall.png")));
        // lowWall1->AddComponent(new BoxCollider2D(lowWall1, Vector2(0, 0),
        //                                          Vector2(15 * lowWall1->transform.scale.x, 30 * lowWall1->transform.scale.y),
        //                                          false));

        // GameObjectManager::GetInstance()->AddGameObject(lowWall1);

        // GameObject *lowWall2 = new GameObject("LowWall2");
        // lowWall2->layer = CollisionMatrix::WALL;
        // lowWall2->transform.rotation = 0;
        // lowWall2->transform.position = Vector2(1000, 600); // Adjust position as needed
        // lowWall2->transform.scale = Vector2(100, 5);       // Adjust scale as needed
        // lowWall2->AddComponent(new SpriteRenderer(lowWall2, Vector2(15, 30), 0, LoadSpriteSheet("Assets/wall.png")));
        // lowWall2->AddComponent(new BoxCollider2D(lowWall2, Vector2(0, 0),
        //                                          Vector2(15 * lowWall2->transform.scale.x, 30 * lowWall2->transform.scale.y),
        //                                          false));

        // GameObjectManager::GetInstance()->AddGameObject(lowWall2);
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
            GameObject *shell = new GameObject("Shell" + std::to_string(rand() + rand()));
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
            GameObject *hook = new GameObject("hook" + std::to_string(rand() + rand()));
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
        //tile 81 14 for boss room
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
            player->AddComponent(new PlayerWeapon(player, 40, 600, 1000, PLAYER_SHOTGUN_PELLET, 10, aimStick, shellDropPS))
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
            GameObject *heal = new GameObject("Heal" + std::to_string(rand() + rand()));
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
            GameObject *hookUpgrade = new GameObject("HookUpgrade" + std::to_string(rand() + rand()));
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
                    player->AddComponent(new PlayerWeapon(player, 30, 600, 3000, 1, 0, aimStick, nullptr))
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
            GameObject *bootUpgrade = new GameObject("BootUpgrade" + std::to_string(rand() + rand()));
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
            GameObject *shield = new GameObject("Shield" + std::to_string(rand() + rand()));
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
            GameObject *powerUpBox = new GameObject("PowerUpBox" + std::to_string(rand() + rand()));
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
                        GameObjectManager::GetInstance()->RemoveGameObject(powerUpBox->GetName());
                    }
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
            GameObject *coin = new GameObject("Coin" + std::to_string(rand() + rand()));
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
        ));

        GameObjectManager::GetInstance()->AddGameObject(CreatePowerUpBox(
            tilemap->GetComponent<Tilemap>()->GetPositionFromTile(36, 4), CreateHeal
        ));


        GameObjectManager::GetInstance()->AddGameObject(CreatePowerUpBox(
            tilemap->GetComponent<Tilemap>()->GetPositionFromTile(25, 8), CreateHookUpgrade
        ));

        GameObjectManager::GetInstance()->AddGameObject(CreatePowerUpBox(
            tilemap->GetComponent<Tilemap>()->GetPositionFromTile(41, 3), CreateBootUpgrade
        ));

        GameObjectManager::GetInstance()->AddGameObject(CreatePowerUpBox(
            tilemap->GetComponent<Tilemap>()->GetPositionFromTile(60, 4), CreateShield
        ));

        // GameObjectManager::GetInstance()->AddGameObject(CreateHeal(tilemap->GetComponent<Tilemap>()->GetPositionFromTile(15, 8)));
        // GameObjectManager::GetInstance()->AddGameObject(CreateHeal(tilemap->GetComponent<Tilemap>()->GetPositionFromTile(46, 4)));
        // GameObjectManager::GetInstance()->AddGameObject(CreateHookUpgrade(tilemap->GetComponent<Tilemap>()->GetPositionFromTile(25, 8)));
        // GameObjectManager::GetInstance()->AddGameObject(CreateBootUpgrade(tilemap->GetComponent<Tilemap>()->GetPositionFromTile(41, 4)));
        // GameObjectManager::GetInstance()->AddGameObject(CreateShield(tilemap->GetComponent<Tilemap>()->GetPositionFromTile(59, 4)));

#pragma endregion

#pragma region Physic Box
        auto CreateBox = [](float mass, Vector2 scale, Vector2 position){
            GameObject *box = new GameObject("Box" + std::to_string(rand() + rand()));
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
            GameObject *gate = new GameObject("Gate" + std::to_string(rand() + rand()));
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
        GameObject *meleeProjectile = new GameObject("MeleeProjectile" + std::to_string(rand() + rand()));
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
            GameObject *melee = new GameObject("Melee" + std::to_string(rand() + rand()));
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
        GameObject *rangedProjectile = new GameObject("RangedProjectile" + std::to_string(rand() + rand()));
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
            GameObject *ranged = new GameObject("Ranged");
            ranged->layer = CollisionMatrix::ENEMY;
            ranged->transform.position = Vector2(2000, 100);
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
