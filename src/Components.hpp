#ifndef COMPONENTS_HPP
#define COMPONENTS_HPP

#include "Core.hpp"
#include "Game.hpp"
#include "Helper.hpp"
#include "Physic2D.hpp"
#include "cmath"

class MovementController : public Component {

private:
    Rigidbody2D *rigidbody;
    SDL_Keycode upKey, downKey, leftKey, rightKey;

    float upSpeed = 0, downSpeed = 0, leftSpeed = 0, rightSpeed = 0;

public:
    float speed = 0;
    float jumpForce = 0;

    MovementController(GameObject *parent, float speed, float jumpForce, bool useWASD) : Component(parent) {
        this->speed = speed;
        this->jumpForce = jumpForce;
        this->rigidbody = this->gameObject->GetComponent<Rigidbody2D>();

        if (useWASD) {
            upKey = SDLK_w;
            downKey = SDLK_s;
            leftKey = SDLK_a;
            rightKey = SDLK_d;
        } else {
            upKey = SDLK_UP;
            downKey = SDLK_DOWN;
            leftKey = SDLK_LEFT;
            rightKey = SDLK_RIGHT;
        }
    }

    void Update() {
        if (!enabled)
            return;
        if (rigidbody == nullptr)
            return;

        float actualSpeed = speed * 1 / FPS;

        if (Game::event.type == SDL_KEYDOWN || Game::event.type == SDL_KEYUP) {
            if (Game::event.key.keysym.sym == upKey) {
                upSpeed = (Game::event.type == SDL_KEYDOWN ? -1 : 0) * jumpForce;
            }
            if (Game::event.key.keysym.sym == downKey) {
                downSpeed = Game::event.type == SDL_KEYDOWN ? 1 : 0;
            }
            if (Game::event.key.keysym.sym == leftKey) {
                leftSpeed = Game::event.type == SDL_KEYDOWN ? -1 : 0;
            }
            if (Game::event.key.keysym.sym == rightKey) {
                rightSpeed = Game::event.type == SDL_KEYDOWN ? 1 : 0;
            }
        }

        Vector2 force = Vector2(leftSpeed + rightSpeed, 0).Normalize() * actualSpeed + Vector2(0, upSpeed);

        if (force.Magnitude() > VELOCITY_EPS) {
            rigidbody->AddForce(force);
        }
        // else{
        //     //If not moving horizontally, remove horizontal speed for snappy movement
        //     rigidbody->velocity -= Vector2::ProjectToVector(rigidbody->velocity, Vector2(1, 0));
        // }
    }

    void Enable() {
        ClearSpeed();
        enabled = true;
    }
    void Disable() {
        ClearSpeed();
        enabled = false;
    }
    bool GetEnabled() {
        return enabled;
    }

    void ClearSpeed() {
        upSpeed = 0;
        downSpeed = 0;
        leftSpeed = 0;
        rightSpeed = 0;
    }

    void Draw() {}

    Component *Clone(GameObject *parent) {
        MovementController *newMovementController = new MovementController(parent, speed, jumpForce, upKey == SDLK_w);
        return newMovementController;
    }
};

class PlayerAnimController : public Component {
private:
    Animator *animator;
    Rigidbody2D *rigidbody;

    enum State {
        IDLE,
        WALK,
        JUMP
    } state = IDLE;

public:
    PlayerAnimController(GameObject *parent) : Component(parent) {
        animator = gameObject->GetComponent<Animator>();
        rigidbody = gameObject->GetComponent<Rigidbody2D>();
    }

    void Update() {
        if (animator == nullptr || rigidbody == nullptr) {
            animator = gameObject->GetComponent<Animator>();
            rigidbody = gameObject->GetComponent<Rigidbody2D>();
            if (animator == nullptr || rigidbody == nullptr)
                return;
        }

        // std::cout << "Player velocity: " << rigidbody->velocity.x << ", " << rigidbody->velocity.y << std::endl;

        if (fabs(rigidbody->velocity.x) > VELOCITY_EPS * 10) {
            if (state != WALK) {
                animator->Play("Walk");
                state = WALK;
            }
        } else {
            if (state != IDLE) {
                animator->Play("Idle");
                state = IDLE;
            }
        }
    }

    void Draw() {}

    Component *Clone(GameObject *parent) {
        PlayerAnimController *newPlayerAnimController = new PlayerAnimController(parent);
        return newPlayerAnimController;
    }
};

class FLipToVelocity : public Component {
private:
    SpriteRenderer *spRenderer = nullptr;
    Rigidbody2D *rigidbody = nullptr;

    Vector2 origin;

public:
    FLipToVelocity(GameObject *parent, Vector2 origin) : Component(parent) {
        rigidbody = parent->GetComponent<Rigidbody2D>();
        spRenderer = parent->GetComponent<SpriteRenderer>();

        this->origin = origin;
    }

    void Update() {
        if (rigidbody == nullptr || spRenderer == nullptr) {
            rigidbody = gameObject->GetComponent<Rigidbody2D>();
            spRenderer = gameObject->GetComponent<SpriteRenderer>();
            if (rigidbody == nullptr || spRenderer == nullptr)
                return;
        }

        if (fabs(rigidbody->velocity.x) < VELOCITY_EPS)
            return;

        spRenderer->isFlipped = Vector2::Dot(rigidbody->velocity, origin) < 0;
    }

    void Draw() {}

    Component *Clone(GameObject *parent) {
        FLipToVelocity *newFlipToVelocity = new FLipToVelocity(parent, origin);
        return newFlipToVelocity;
    }
};

class AutoDestroy : public Component {
private:
    float timeToDestroy;
    float startTime;

public:
    AutoDestroy(GameObject *parent, float timeToDestroy) : Component(parent) {
        this->timeToDestroy = timeToDestroy;
        this->startTime = SDL_GetTicks();
    }

    void Update() {
        if (SDL_GetTicks() - startTime > timeToDestroy) {
            std::cout << "Destroyed " << gameObject->GetName() << std::endl;
            GameObject::Destroy(gameObject->GetName());
        }
    }

    void Draw() {}

    Component *Clone(GameObject *parent) {
        AutoDestroy *newAutoDestroy = new AutoDestroy(parent, timeToDestroy);
        return newAutoDestroy;
    }
};

class SpawnBall : public Component {
private:
    GameObject *ballPrefab;
    SDL_KeyCode spawnKey;

public:
    SpawnBall(GameObject *parent, GameObject *ballPrefab, SDL_KeyCode spawnKey) : Component(parent) {
        this->ballPrefab = ballPrefab;
        this->spawnKey = spawnKey;
    }

    void Update() {
        static bool spawn = false;
        if (Game::event.type == SDL_KEYDOWN || Game::event.type == SDL_KEYUP) {
            if (Game::event.key.keysym.sym == spawnKey) {
                spawn = Game::event.type == SDL_KEYDOWN;
            }
        }
        if (spawn) {
            GameObject *ball = GameObject::Instantiate("Ball" + std::to_string(rand() + rand()), ballPrefab, gameObject->transform.position,
                                                       ballPrefab->transform.rotation, ballPrefab->transform.scale);
            ball->AddComponent(new AutoDestroy(ball, 3000));

            Rigidbody2D *rb = gameObject->GetComponent<Rigidbody2D>();
            if (rb) {
                ball->GetComponent<Rigidbody2D>()->AddForce(Vector2(-1 * rb->velocity.x * 0.5, 1));
            }

            GameObjectManager::GetInstance()->AddGameObject(ball);
        }
    }

    void Draw() {}

    Component *Clone(GameObject *parent) {
        SpawnBall *newSpawnBall = new SpawnBall(parent, ballPrefab, spawnKey);
        return newSpawnBall;
    }
};

class Camera : public Component {
private:
    GameObject *follow = nullptr;
    Vector2 offset;
    Vector2 deadZone;
    Vector2 size;

    Rigidbody2D *rigidbody = nullptr;

    float speed;
public:

    Camera(GameObject *parent, GameObject *follow, Vector2 size, Vector2 offset, float speed, Vector2 deadZone) : Component(parent) {
        this->follow = follow;

        this->size = size;
        this->offset = offset;

        this->deadZone = deadZone;

        this->speed = speed;

        rigidbody = gameObject->GetComponent<Rigidbody2D>();
    }

    void Update() {
        if (follow == nullptr)
            return;

        Vector2 followPos = follow->transform.position + offset;

        //Deadzone check
        bool move = false;
        if (fabs(followPos.x - gameObject->transform.position.x) > deadZone.x) {
            gameObject->transform.position.x += (followPos.x - gameObject->transform.position.x) * speed * 1 / FPS;
            move = true;
        }

        if (fabs(followPos.y - gameObject->transform.position.y) > deadZone.y) {
            gameObject->transform.position.y += (followPos.y - gameObject->transform.position.y) * speed * 1 / FPS;
            move = true;
        }

        if (!rigidbody){
            rigidbody = gameObject->GetComponent<Rigidbody2D>();
            if (!rigidbody)
                return;
        }

        //Only move if the target is out of deadzone
        if (move){
            rigidbody->AddForce((follow->transform.position - gameObject->transform.position).Normalize() * speed * 1 / FPS);
        }
    }

    void Draw() {}

};

#endif
