#ifndef COMPONENTS_HPP
#define COMPONENTS_HPP

#include "Core.hpp"
#include "Game.hpp"
#include "Helper.hpp"
#include "Physic2D.hpp"
class AutoDestroy : public Component {
private:
    float timeToDestroy;
    float startTime;

public:
    AutoDestroy(GameObject *parent, float timeToDestroy);
    void Update();
    void Draw();
    Component *Clone(GameObject *parent);
};

class ParticleSystem : public Component {
private:
    GameObject *particlePrefab;
    float spawnRate;
    float lastSpawnTime;
    float particleLifeTime;
    float emitForce;
    Vector2 emitDirection;
    float emitAngle;
    bool isPlaying;
    int emitTime;

public:
    ParticleSystem(GameObject *parent, GameObject *particlePrefab, float spawnRate, float particleLifeTime, Vector2 emitDirection, float emitForce, float emitAngle);
    void Update();
    void Draw();
    void setEmitDirection(Vector2 direction);
    void Play();
    void Stop();
    void Emit(int time);
    Component *Clone(GameObject *parent);
};

class Joystick : public Component {
private:
    SDL_Keycode upKey, downKey, leftKey, rightKey;
    float upSpeed, downSpeed, leftSpeed, rightSpeed;
    Vector2 direction;

public:
    Joystick(GameObject *parent, SDL_Keycode upKey, SDL_Keycode downKey, SDL_Keycode leftKey, SDL_Keycode rightKey);
    void SetKey(SDL_Keycode upKey, SDL_Keycode downKey, SDL_Keycode leftKey, SDL_Keycode rightKey);
    Vector2 GetDirection();
    void Update();
    void Draw();
    Component *Clone(GameObject *parent);
};

class MovementController : public Component {
private:
    Rigidbody2D *rigidbody;
    Joystick* joystick;

public:
    float speed;
    float jumpForce;
    MovementController(GameObject *parent, float speed, float jumpForce, Joystick* joystick);
    void Update();
    void Enable();
    void Disable();
    bool GetEnabled();
    void Draw();
    Component *Clone(GameObject *parent);
};

class JumpController : public Component {
private:
    Rigidbody2D *rigidbody;
    SDL_KeyCode jumpKey;
    float jumpForce;
    float cooldown;
    float lastJumpTime;
    bool grounded;
    CollisionMatrix::Layers groundLayer;
    Vector2 lastNormal;
    void OnCollisionEnter(Collider2D *collider);

public:
    JumpController(GameObject *parent, SDL_KeyCode jumpKey, float jumpForce, float cooldown, CollisionMatrix::Layers whatIsGround);
    void Update();
    void Draw();
    void BindCollider(Collider2D *collider);
    Component *Clone(GameObject *parent);
};

class PlayerAnimController : public Component {
private:
    Animator *animator;
    Rigidbody2D *rigidbody;
    enum State { IDLE, WALK, JUMP } state;

public:
    PlayerAnimController(GameObject *parent);
    void Update();
    void Draw();
    Component *Clone(GameObject *parent);
};

class ShellBehavior : public Component {
private:
    Rigidbody2D *rigidbody;
    float lifeTime;
    float startTime;
    float speed;
    Vector2 direction;

public:
    ShellBehavior(GameObject *parent, float lifeTime, float speed, Vector2 direction);
    void Update();
    void Draw();
    Component *Clone(GameObject *parent);
};

class PlayerShoot : public Component {
private:
    std::function<GameObject *(float speed, Vector2 direction, float lifeTime, Vector2 position)> createShell;
    float shellSpeed;
    float shellLifetime;
    float shootCooldown;
    float lastShootTime;
    int shootAmount;
    float shootAngle;
    GameObject* particle;
    Vector2 lastDirection;
    ParticleSystem *particleSystem;
    Joystick *joystick;

public:
    PlayerShoot(GameObject *parent, float shellSpeed, float shellLifeTime, float shootCooldown, float shootAmount, float shootAngle, Joystick *joystick);
    void setSpawnFunction(std::function<GameObject *(float speed, Vector2 direction, float lifeTime, Vector2 position)> createShell);
    void Update();
    void Draw();
    Component *Clone(GameObject *parent);
};

class Orbit : public Component {
private:
    GameObject *target;
    float radius;
    float angle;
    SpriteRenderer *spRenderer;
    Vector2 lastDirection;
    Vector2 originalForward;
    Joystick *joystick;

public:
    Orbit(GameObject *parent, GameObject *target, float radius, Vector2 originalForward, Joystick *joystick);
    void Update();
    void Draw();
    Component *Clone(GameObject *parent);
};

class FLipToVelocity : public Component {
private:
    SpriteRenderer *spRenderer;
    Rigidbody2D *rigidbody;
    Vector2 origin;

public:
    FLipToVelocity(GameObject *parent, Vector2 origin);
    void Update();
    void Draw();
    Component *Clone(GameObject *parent);
};

class Camera : public Component {
private:
    GameObject *follow;
    Vector2 offset;
    Vector2 deadZone;
    Vector2 size;
    Rigidbody2D *rigidbody;
    float speed;

public:
    Camera(GameObject *parent, GameObject *follow, Vector2 size, Vector2 offset, float speed, Vector2 deadZone);
    void Update();
    void Draw();
    Vector2 WorldToScreen(Vector2 worldPos);
    Vector2 ScreenToWorld(Vector2 screenPos);
    void SetFollow(GameObject *follow);
    Component *Clone(GameObject *parent);
};

#endif