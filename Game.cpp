#include "Game.hpp"
#include "Components.hpp"
#include "CustomClasses.hpp"
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
    SoundManager::GetInstance();
    SoundManager::GetInstance()->AddMusic("MenuBgm", "Assets/SFX/fairyfountain.mp3", 100);
    SoundManager::GetInstance()->AddMusic("GameBgm", "Assets/SFX/papyrus.mp3", 32);

    SoundManager::GetInstance()->AddSound("ball_bounce", "Assets/SFX/ball_bounce.mp3", 128);
    SoundManager::GetInstance()->AddSound("ball_kick", "Assets/SFX/ball_kick.mp3", 128);

    SoundManager::GetInstance()->AddSound("Game_Over", "Assets/SFX/gameover.mp3", 128);
    SoundManager::GetInstance()->AddSound("Goal", "Assets/SFX/score.mp3", 64);

    std::cout << "Object Initialisation..." << std::endl;

    Scene *menuScene = new Scene("MainMenu");
    menuScene->AssignLogic([menuScene, this]() {
        Game::state = MENU;
        SoundManager::GetInstance()->PlayMusic("MenuBgm");

        GameObject *background = new GameObject("Background");
        background->transform.position = Vector2(640, 360);
        background->transform.scale = Vector2(1, 1);

        background->AddComponent(new SpriteRenderer(background, Vector2(2560, 1707), -10, LoadSpriteSheet("Assets/Sprites/UI/MenuBG.jpg")));

        GameObjectManager::GetInstance()->AddGameObject(background);

        GameObject *title = new GameObject("Title");
        title->transform.position = Vector2(640, 200);
        title->transform.scale = Vector2(10, 10);
        
        title->AddComponent(new SpriteRenderer(title, Vector2(64, 16), 0, LoadSpriteSheet("Assets/Sprites/UI/Game_Name.png")));
        title->AddComponent(new Animator(title, {AnimationClip("Idle", "Assets/Sprites/UI/Game_Name.png", Vector2(64, 16), 200, true, 1.0, 0, 1)}));
        title->GetComponent<Animator>()->Play("Idle");
        
        GameObjectManager::GetInstance()->AddGameObject(title);

        GameObject *playButtonSingle = new GameObject("PlayButton");
        playButtonSingle->transform.position = Vector2(640, 400);
        playButtonSingle->transform.scale = Vector2(5, 5);

        playButtonSingle->AddComponent(new SpriteRenderer(playButtonSingle, Vector2(48, 16), 0, LoadSpriteSheet("Assets/Sprites/UI/Play_button1p.png")));

        playButtonSingle->AddComponent(new BoxCollider2D(playButtonSingle, Vector2(0, 0), 
            Vector2(48 * playButtonSingle->transform.scale.x, 16 * playButtonSingle->transform.scale.y)
        ));

        playButtonSingle->AddComponent(new Button(playButtonSingle));
        playButtonSingle->GetComponent<Button>()->AddOnClickHandler(
            [menuScene, this]() {
                Game::state = GAME;
                Player2Mode = false;
                TestMode = false;
            }
        );
        
        GameObjectManager::GetInstance()->AddGameObject(playButtonSingle);

        GameObject *playButtonMulti = new GameObject("PlayButtonMulti");
        playButtonMulti->transform.position = Vector2(640, 550);
        playButtonMulti->transform.scale = Vector2(5, 5);

        playButtonMulti->AddComponent(new SpriteRenderer(playButtonMulti, Vector2(48, 16), 0, LoadSpriteSheet("Assets/Sprites/UI/Play_Button2p.png")));

        playButtonMulti->AddComponent(new BoxCollider2D(playButtonMulti, Vector2(0, 0), 
            Vector2(48 * playButtonMulti->transform.scale.x, 16 * playButtonMulti->transform.scale.y)
        ));

        playButtonMulti->AddComponent(new Button(playButtonMulti));
        playButtonMulti->GetComponent<Button>()->AddOnClickHandler(
            [menuScene, this]() {
                Game::state = GAME;
                Player2Mode = true;
                TestMode = false;
            }
        );
        
        GameObjectManager::GetInstance()->AddGameObject(playButtonMulti);

        GameObject *testButton = new GameObject("TestButton");
        testButton->transform.position = Vector2(1100, 700);
        testButton->transform.scale = Vector2(2, 2);

        testButton->AddComponent(new SpriteRenderer(testButton, Vector2(32, 16), 0, LoadSpriteSheet("Assets/Sprites/UI/Play_button.png")));

        testButton->AddComponent(new BoxCollider2D(testButton, Vector2(0, 0), 
            Vector2(32 * testButton->transform.scale.x, 16 * testButton->transform.scale.y)
        ));

        testButton->AddComponent(new Button(testButton));
        testButton->GetComponent<Button>()->AddOnClickHandler(
            [menuScene, this]() {
                Game::state = GAME;
                Player2Mode = true;
                TestMode = true;
            }
        );
        
        GameObjectManager::GetInstance()->AddGameObject(testButton);

        GameObject *quitButton = new GameObject("QuitButton");
        quitButton->transform.scale = Vector2(2, 2);

        quitButton->transform.position = Vector2(1280 - 32 * 2 / 2, 32 * 2 / 2);

        quitButton->AddComponent(new SpriteRenderer(quitButton, Vector2(32, 32), 0, LoadSpriteSheet("Assets/Sprites/UI/Quit_button.png")));

        quitButton->AddComponent(new BoxCollider2D(quitButton, Vector2(0, 0), 
            Vector2(32 * quitButton->transform.scale.x, 32 * quitButton->transform.scale.y)
        ));

        quitButton->AddComponent(new Button(quitButton));
        quitButton->GetComponent<Button>()->AddOnClickHandler(
            [menuScene, this]() {
                SDL_Event *SDL_Quit = new SDL_Event();
                SDL_Quit->type = SDL_QUIT;
                SDL_PushEvent(SDL_Quit);
            }
        );

        GameObjectManager::GetInstance()->AddGameObject(quitButton);
    });

    SceneManager::GetInstance()->AddScene(menuScene);

    Scene *gameoverScene = new Scene("GameOver");

    gameoverScene->AssignLogic([gameoverScene, this]() {
        SoundManager::GetInstance()->StopMusic();
        SoundManager::GetInstance()->PlaySound("Game_Over");

        GameObject* gameoverText = new GameObject("GameOverText");
        gameoverText->transform.position = Vector2(640, 200);
        gameoverText->transform.scale = Vector2(5, 5);

        gameoverText->AddComponent(new SpriteRenderer(gameoverText, Vector2(128, 64), 1, LoadSpriteSheet("Assets/Sprites/UI/GameOver.png")));

        GameObjectManager::GetInstance()->AddGameObject(gameoverText);

        GameObject *quitButton = new GameObject("QuitButton");
        quitButton->transform.scale = Vector2(2, 2);

        quitButton->transform.position = Vector2(1280 - 32 * 2 / 2, 32 * 2 / 2);

        quitButton->AddComponent(new SpriteRenderer(quitButton, Vector2(32, 32), 0, LoadSpriteSheet("Assets/Sprites/UI/Quit_button.png")));

        quitButton->AddComponent(new BoxCollider2D(quitButton, Vector2(0, 0), 
            Vector2(32 * quitButton->transform.scale.x, 32 * quitButton->transform.scale.y)
        ));

        quitButton->AddComponent(new Button(quitButton));
        quitButton->GetComponent<Button>()->AddOnClickHandler(
            [gameoverScene, this]() {
                Game::state = MENU;
                scoreTeam1 = scoreTeam2 = 0;
            }
        );

        GameObjectManager::GetInstance()->AddGameObject(quitButton);
    });

    SceneManager::GetInstance()->AddScene(gameoverScene);

    Scene *gameScene = new Scene("Game");
    gameScene->AssignLogic([gameScene, this]() {
        Game::state = GAME;
        SoundManager::GetInstance()->PlayMusic("GameBgm");

#pragma region Background Setup
        GameObject *background = new GameObject("Background");
        background->transform.position = Vector2(640, 360);
        background->transform.scale = Vector2(1, 1);

        background->AddComponent(new SpriteRenderer(background, Vector2(1280, 720), -10, LoadSpriteSheet("Assets/Sprites/yard.png")));

        GameObjectManager::GetInstance()->AddGameObject(background);
#pragma endregion

#pragma region Ball Setup
        GameObject *ball = new GameObject("Ball");
        ball->tag = 3;
        ball->transform.position = Vector2(640, 360);
        ball->transform.scale = Vector2(2, 2);

        ball->AddComponent(new SpriteRenderer(ball, Vector2(15, 15), 10, LoadSpriteSheet("Assets/default.png")));

        ball->AddComponent(new Animator(ball, {AnimationClip("Roll", "Assets/soccer_ball.png", Vector2(15, 15), 1000, true, 1.0, 0, 1)}));
        ball->GetComponent<Animator>()->Play("Roll");

        ball->AddComponent(new Rigidbody2D(ball, 1, 0.025, .9));

        ball->AddComponent(new VelocityToAnimSpeedController(ball, "Roll"));
        ball->AddComponent(new StayInBounds(ball, false));

        ball->AddComponent(new CircleCollider2D(ball, Vector2(0, 0), 7.5));

        ball->AddComponent(new BallStateMachine(ball, 2.0, 700, 1000));

        ball->GetComponent<CircleCollider2D>()->OnCollisionEnter.addHandler(
            [ball](Collider2D *collider) {
                ball->GetComponent<BallStateMachine>()->OnCollisionEnter(collider);
            });

        GameObjectManager::GetInstance()->AddGameObject(ball);
#pragma endregion

#pragma region Player setup

        GameObject *player1 = new GameObject("Player1");
        player1->transform.scale = Vector2(2, 2);

        player1->AddComponent(new SpriteRenderer(player1, Vector2(31, 82), 0, LoadSpriteSheet("Assets/actor.png")));
        player1->AddComponent(new Rigidbody2D(player1, 1, 0.04, .2));
        player1->AddComponent(new CircleCollider2D(player1, Vector2(0, 0), 17 * player1->transform.scale.x));
        player1->AddComponent(new StayInBounds(player1, false));
        player1->AddComponent(new RotateTowardVelocity(player1, Vector2(0, -1)));
        player1->AddComponent(new VelocityToAnimSpeedController(player1, "Run"));

        player1->transform.position = Vector2(100, HEIGHT / 2); // Centered vertically
        GameObject *player2 = GameObject::Instantiate("Player2", player1, Vector2(175, HEIGHT / 2 + 60), 0, Vector2(2, 2));
        GameObject *player3 = GameObject::Instantiate("Player3", player1, Vector2(175, HEIGHT / 2 - 60), 0, Vector2(2, 2));

        GameObject *player4 = GameObject::Instantiate("Player4", player1, Vector2(WIDTH - 175, HEIGHT / 2 - 60), 0, Vector2(2, 2));
        GameObject *player6 = GameObject::Instantiate("Player6", player1, Vector2(WIDTH - 100, HEIGHT / 2), 0, Vector2(2, 2));
        GameObject *player5 = GameObject::Instantiate("Player5", player1, Vector2(WIDTH - 175, HEIGHT / 2 + 60), 0, Vector2(2, 2));

        player1->tag = player2->tag = player3->tag = 1;
        player4->tag = player5->tag = player6->tag = 2;

        player1->AddComponent(new Animator(player1, {AnimationClip("Run", "Assets/Sprites/football.png", Vector2(32, 32), 1000, true, 1.0, 0, 5)}));
        player2->AddComponent(new Animator(player2, {AnimationClip("Run", "Assets/Sprites/football2.png", Vector2(32, 32), 1000, true, 1.0, 0, 5)}));
        player3->AddComponent(new Animator(player3, {AnimationClip("Run", "Assets/Sprites/football3.png", Vector2(32, 32), 1000, true, 1.0, 0, 5)}));
        
        player4->AddComponent(new Animator(player4, {AnimationClip("Run", "Assets/Sprites/football4.png", Vector2(32, 32), 1000, true, 1.0, 0, 5)}));

        player5->AddComponent(new Animator(player5, {AnimationClip("Run", "Assets/Sprites/football5.png", Vector2(32, 32), 1000, true, 1.0, 0, 5)}));
        player6->AddComponent(new Animator(player6, {AnimationClip("Run", "Assets/Sprites/football6.png", Vector2(32, 32), 1000, true, 1.0, 0, 5)}));

        auto setupCollisionHandler = [](GameObject *player) {
            player->GetComponent<CircleCollider2D>()->OnCollisionEnter.addHandler(
                [player](Collider2D *collider) {
                    if (collider->gameObject->tag == 4) {
                        Rigidbody2D *rigidbody = player->GetComponent<Rigidbody2D>();
                        SoundManager::GetInstance()->PlaySound("ball_bounce");
                        rigidbody->BounceOff(collider->GetNormal(player->transform.position));
                    }
                });
        };

        setupCollisionHandler(player1);
        setupCollisionHandler(player2);
        setupCollisionHandler(player3);
        setupCollisionHandler(player4);
        setupCollisionHandler(player5);
        setupCollisionHandler(player6);

        player1->AddComponent(new MovementController(player1, GoalKeeperSpeed, true));
        player2->AddComponent(new MovementController(player2, DefenderSpeed, true));
        player3->AddComponent(new MovementController(player3, AttackerSpeed, true));

        if (Player2Mode) {
            player4->AddComponent(new MovementController(player4, AttackerSpeed, false));
            player5->AddComponent(new MovementController(player5, DefenderSpeed, false));
            player6->AddComponent(new MovementController(player6, GoalKeeperSpeed, false));
        }

        player1->AddComponent(new KickControl(player1, ball, SDLK_SPACE, HIGH_KICK_FORCE));
        player2->AddComponent(new KickControl(player2, ball, SDLK_SPACE, LOW_KICK_FORCE));
        player3->AddComponent(new KickControl(player3, ball, SDLK_SPACE, HIGH_KICK_FORCE));

        if (Player2Mode) {
            player4->AddComponent(new KickControl(player4, ball, SDLK_KP_ENTER, HIGH_KICK_FORCE));
            player5->AddComponent(new KickControl(player5, ball, SDLK_KP_ENTER, LOW_KICK_FORCE));
            player6->AddComponent(new KickControl(player6, ball, SDLK_KP_ENTER, HIGH_KICK_FORCE));
        }

        player1->AddComponent(new AIGoalKeeper(player1, ball, GoalKeeperSpeed, true));
        player2->AddComponent(new AIDefender(player2, ball, DefenderSpeed, true));
        player3->AddComponent(new AIAttacker(player3, ball, AttackerSpeed, true));

        player4->AddComponent(new AIAttacker(player4, ball, AttackerSpeed, false));
        player5->AddComponent(new AIDefender(player5, ball, DefenderSpeed, false));
        player6->AddComponent(new AIGoalKeeper(player6, ball, GoalKeeperSpeed, false));

        // First controller switcher for player1, player2, and player3
        GameObject *controllerSwitcher1 = new GameObject("ControllerSwitcher1");
        TeamControl *movementControllerSwitcher1 = dynamic_cast<TeamControl *>(controllerSwitcher1->AddComponent(
            new TeamControl(controllerSwitcher1, LoadSpriteSheet("Assets/blue_indicator.png"), 75.0)));
        movementControllerSwitcher1->AddMovementController(SDLK_1, player1->GetComponent<MovementController>());
        movementControllerSwitcher1->AddMovementController(SDLK_2, player2->GetComponent<MovementController>());
        movementControllerSwitcher1->AddMovementController(SDLK_3, player3->GetComponent<MovementController>());
        GameObjectManager::GetInstance()->AddGameObject(controllerSwitcher1);

        if (Player2Mode || TestMode) {
            // Second controller switcher for player4, player5, and player6
            GameObject *controllerSwitcher2 = new GameObject("ControllerSwitcher2");
            TeamControl *movementControllerSwitcher2 = dynamic_cast<TeamControl *>(controllerSwitcher2->AddComponent(
                new TeamControl(controllerSwitcher2, LoadSpriteSheet("Assets/red_indicator.png"), 75.0)));
            movementControllerSwitcher2->AddMovementController(SDLK_KP_6, player6->GetComponent<MovementController>());
            movementControllerSwitcher2->AddMovementController(SDLK_KP_4, player4->GetComponent<MovementController>());
            movementControllerSwitcher2->AddMovementController(SDLK_KP_5, player5->GetComponent<MovementController>());
            GameObjectManager::GetInstance()->AddGameObject(controllerSwitcher2);
        }

        GameObjectManager::GetInstance()->AddGameObject(player1);
        
        GameObjectManager::GetInstance()->AddGameObject(player6);

        if (!TestMode){
            GameObjectManager::GetInstance()->AddGameObject(player2);
            GameObjectManager::GetInstance()->AddGameObject(player3);
            GameObjectManager::GetInstance()->AddGameObject(player4);
            GameObjectManager::GetInstance()->AddGameObject(player5);
        }
        else{
            player2->GetComponent<Collider2D>()->enabled = false;
            player3->GetComponent<Collider2D>()->enabled = false;
            player4->GetComponent<Collider2D>()->enabled = false;
            player5->GetComponent<Collider2D>()->enabled = false;
        }


#pragma endregion

#pragma region Goal Setup
        GameObject *goal1 = new GameObject("Goal1");
        goal1->transform.position = Vector2(30, 360);
        goal1->transform.scale = Vector2(5, 4);
        goal1->tag = 5;

        goal1->AddComponent(new SpriteRenderer(goal1, Vector2(16, 53), 0, LoadSpriteSheet("Assets/Sprites/goal.png")));
        goal1->AddComponent(new BoxCollider2D(goal1, Vector2(0, 0), 
            Vector2(16 * goal1->transform.scale.x, 53 * goal1->transform.scale.y)
        ));

        goal1->GetComponent<BoxCollider2D>()->OnCollisionEnter.addHandler(
            [goal1, this](Collider2D *collider) {
                BoxCollider2D *goal1Col = goal1->GetComponent<BoxCollider2D>();
                if (collider->gameObject->tag == 3) {
                    if (goal1Col->GetNormal(collider->gameObject->transform.position) == Vector2(1, 0)) {
                        std::cout << "Goal!!! Right team scored!" << std::endl;
                        SoundManager::GetInstance()->PlaySound("Goal");
                        this->scoreTeam2++;
                        SceneManager::GetInstance()->LoadScene("Game");
                    } else {
                        Rigidbody2D *rigidbody = collider->gameObject->GetComponent<Rigidbody2D>();
                        rigidbody->BounceOff(goal1Col->GetNormal(collider->gameObject->transform.position));
                    }
                } else {
                    Rigidbody2D *rigidbody = collider->gameObject->GetComponent<Rigidbody2D>();
                    collider->gameObject->transform.position += goal1Col->GetNormal(collider->gameObject->transform.position) * rigidbody->velocity.Magnitude();
                }
            });

        GameObjectManager::GetInstance()->AddGameObject(goal1);

        GameObject *goal2 = new GameObject("Goal2");
        goal2->transform.position = Vector2(1250, 360);
        goal2->transform.rotation = 180;
        goal2->transform.scale = Vector2(5, 4);
        goal2->tag = 6;

        goal2->AddComponent(new SpriteRenderer(goal2, Vector2(16, 53), 0, LoadSpriteSheet("Assets/Sprites/goal.png")));
        goal2->AddComponent(new BoxCollider2D(goal2, Vector2(0, 0), 
            Vector2(16 * goal2->transform.scale.x, 53 * goal2->transform.scale.y)
        ));

        goal2->GetComponent<BoxCollider2D>()->OnCollisionEnter.addHandler(
            [goal2, this](Collider2D *collider) {
                BoxCollider2D *goal2Col = goal2->GetComponent<BoxCollider2D>();
                if (collider->gameObject->tag == 3) {
                    if (goal2Col->GetNormal(collider->gameObject->transform.position) == Vector2(-1, 0)) {
                        std::cout << "Goal!!! Left team scored!" << std::endl;
                        SoundManager::GetInstance()->PlaySound("Goal");
                        this->scoreTeam1++;
                        SceneManager::GetInstance()->LoadScene("Game");
                        return;
                    } else {
                        Rigidbody2D *rigidbody = collider->gameObject->GetComponent<Rigidbody2D>();
                        rigidbody->BounceOff(goal2Col->GetNormal(collider->gameObject->transform.position));
                    }
                } else {
                    Rigidbody2D *rigidbody = collider->gameObject->GetComponent<Rigidbody2D>();
                    collider->gameObject->transform.position += goal2Col->GetNormal(collider->gameObject->transform.position) * rigidbody->velocity.Magnitude();
                }
            });

        GameObjectManager::GetInstance()->AddGameObject(goal2);

#pragma endregion
    });

    SceneManager::GetInstance()->AddScene(gameScene);
    SceneManager::GetInstance()->LoadScene("MainMenu");
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
            scoreTeam1 = scoreTeam2 = 0;
            return;
        }
    }

    //End condition
    if (scoreTeam1 + scoreTeam2 >= 5) {
        state = GAMEOVER;
        return;
    }

}

void Game::handleSceneChange() {
    switch (state) {
    case MENU:
        if (SceneManager::GetInstance()->GetCurrentScene()->GetName() != "MainMenu")
            SceneManager::GetInstance()->LoadScene("MainMenu");
        break;
    case GAME:
        if (SceneManager::GetInstance()->GetCurrentScene()->GetName() != "Game")
            SceneManager::GetInstance()->LoadScene("Game");
        break;
    case GAMEOVER:
        if (SceneManager::GetInstance()->GetCurrentScene()->GetName() != "GameOver")
            SceneManager::GetInstance()->LoadScene("GameOver");
        break;
    }
}

void Game::update() {
    SceneManager::GetInstance()->Update();
}

void Game::render() {
    SDL_RenderClear(renderer);
    SceneManager::GetInstance()->Draw();

    // Show score
    if (state == GAME){
        SDL_Color textColor = {0, 0, 0, 255};
        std::string scoreText = std::to_string(scoreTeam1) + " - " + std::to_string(scoreTeam2);
        SDL_Texture* scoreTexture = LoadFontTexture(scoreText, "Assets/Fonts/arial.ttf", textColor, 50);
        if (scoreTexture) {
            RenderTexture(scoreTexture, 640, 20);
            SDL_DestroyTexture(scoreTexture);
        } else {
            std::cerr << "Failed to load score texture" << std::endl;
        }
    }

    if (state == GAMEOVER){
        // Render final scores
        SDL_Color textColor = {255, 255, 255, 255};
        std::string scoreText = "Final Score: " + std::to_string(scoreTeam1) + " - " + std::to_string(scoreTeam2);
        SDL_Texture* scoreTexture = LoadFontTexture(scoreText, "Assets/Fonts/arial.ttf", textColor, 75);
        if (scoreTexture) {
            RenderTexture(scoreTexture, 640, 400); // Centered below "Game Over!"
            SDL_DestroyTexture(scoreTexture);
        } else {
            std::cerr << "Failed to load score texture" << std::endl;
        }
    }

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