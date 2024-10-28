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

#endif
