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

    MovementController(GameObject *parent, float speed, bool useWASD) : Component(parent) {
        this->speed = speed;
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
                upSpeed = Game::event.type == SDL_KEYDOWN ? -1 : 0;
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

        rigidbody->AddForce(Vector2(leftSpeed + rightSpeed, upSpeed + downSpeed).Normalize() * actualSpeed);
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
        MovementController *newMovementController = new MovementController(parent, speed, upKey == SDLK_w);
        return newMovementController;
    }
};

class FLipToVelocity : public Component{
private:
    SpriteRenderer *spRenderer = nullptr;
    Rigidbody2D *rigidbody = nullptr;

    Vector2 origin;
public:
    FLipToVelocity(GameObject *parent, Vector2 origin) : Component(parent){
        rigidbody = parent->GetComponent<Rigidbody2D>();
        spRenderer = parent->GetComponent<SpriteRenderer>();

        this->origin = origin;
    }

    void Update(){
        if (rigidbody == nullptr || spRenderer == nullptr){
            rigidbody = gameObject->GetComponent<Rigidbody2D>();
            spRenderer = gameObject->GetComponent<SpriteRenderer>();
            if (rigidbody == nullptr || spRenderer == nullptr) return;
        }
        
        if (fabs(rigidbody->velocity.x) < VELOCITY_EPS) return;
        
        spRenderer->isFlipped = Vector2::Dot(rigidbody->velocity, origin) < 0;
    }

    void Draw(){}

    Component *Clone(GameObject *parent){
        FLipToVelocity *newFlipToVelocity = new FLipToVelocity(parent, origin);
        return newFlipToVelocity;
    }
};



class AutoDestroy : public Component {
private:
    float timeToDestroy;
    float startTime;
public:
    AutoDestroy(GameObject *parent, float timeToDestroy) : Component(parent){
        this->timeToDestroy = timeToDestroy;
        this->startTime = SDL_GetTicks();
    }

    void Update(){
        if (SDL_GetTicks() - startTime > timeToDestroy){
            std::cout << "Destroyed " << gameObject->GetName() << std::endl;
            GameObject::Destroy(gameObject->GetName());
        }
    }

    void Draw(){}

    Component *Clone(GameObject *parent){
        AutoDestroy *newAutoDestroy = new AutoDestroy(parent, timeToDestroy);
        return newAutoDestroy;
    }
};

class SpawnBall : public Component {
private:
    GameObject *ballPrefab;
    SDL_KeyCode spawnKey;
public:
    SpawnBall(GameObject *parent, GameObject *ballPrefab, SDL_KeyCode spawnKey) : Component(parent){
        this->ballPrefab = ballPrefab;
        this->spawnKey = spawnKey;
    }

    void Update(){
        if (Game::event.type == SDL_KEYDOWN){
            if (Game::event.key.keysym.sym == spawnKey){
                GameObject *ball = GameObject::Instantiate("Ball" + std::to_string(rand() + rand()), ballPrefab, gameObject->transform.position, 
                                    ballPrefab->transform.rotation, ballPrefab->transform.scale);
                ball->AddComponent(new AutoDestroy(ball, 3000));
                GameObjectManager::GetInstance()->AddGameObject(ball);
            }
        }
    }

    void Draw(){}

    Component *Clone(GameObject *parent){
        SpawnBall *newSpawnBall = new SpawnBall(parent, ballPrefab, spawnKey);
        return newSpawnBall;
    }

};

#endif
