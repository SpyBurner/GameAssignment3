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
    GameObject *particlePrefab = nullptr;
    float spawnRate;
    float lastSpawnTime;
    float particleLifeTime;
    float emitForce;
    Vector2 emitDirection;
    float emitAngle = 0;
    bool isPlaying = true;
    int emitTime = 0;

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
    float upSpeed = 0, downSpeed = 0, leftSpeed = 0, rightSpeed = 0;
    Vector2 direction = Vector2(0, 1);

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
    Rigidbody2D *rigidbody = nullptr;
    Joystick* joystick;

public:
    float speed = 0;
    MovementController(GameObject *parent, float speed, Joystick* joystick);
    void Update();
    void Enable();
    void Disable();
    bool GetEnabled();
    void Draw();
    Component *Clone(GameObject *parent);
};

class JumpController : public Component {
private:
    Rigidbody2D *rigidbody = nullptr;
    SDL_KeyCode jumpKey;
    float jumpForce = 0;
    float cooldown = 0;
    float lastJumpTime = 0;
    bool grounded = false;
    CollisionMatrix::Layer groundLayer = CollisionMatrix::DEFAULT;
    Vector2 lastNormal = Vector2(0, 0);
    void OnCollisionEnter(Collider2D *collider);

public:
    JumpController(GameObject *parent, SDL_KeyCode jumpKey, float jumpForce, float cooldown, CollisionMatrix::Layer whatIsGround);
    void Update();
    void Draw();
    void BindCollider(Collider2D *collider);
    Component *Clone(GameObject *parent);
};

class PlayerAnimController : public Component {
private:
    Animator *animator = nullptr;
    Rigidbody2D *rigidbody = nullptr;
    enum State { IDLE, WALK, JUMP } state = IDLE;

public:
    PlayerAnimController(GameObject *parent);
    void Update();
    void Draw();
    Component *Clone(GameObject *parent);
};

class ShellBehavior : public Component {
private:
    Rigidbody2D *rigidbody = nullptr;
    float lifeTime;
    float startTime;
    float speed;
    Vector2 direction;

    GameObject *sender = nullptr;
public:
    ShellBehavior(GameObject *parent, float lifeTime, float speed, Vector2 direction);

    void Update();
    void Draw();

    void SetSender(GameObject *sender);
    GameObject * GetSender();

    Component *Clone(GameObject *parent);
};

class PlayerShoot : public Component {
private:
    std::function<GameObject *(float speed, Vector2 direction, float lifeTime, Vector2 position)> createShell = nullptr;
    float shellSpeed = 0;
    float shellLifetime = 0;
    float shootCooldown = 0;
    float lastShootTime = 0;
    int shootAmount = 1;
    float shootAngle = 0;
    GameObject* particle = nullptr;
    Vector2 lastDirection = Vector2(0, 1);
    ParticleSystem *particleSystem = nullptr;
    Joystick *joystick = nullptr;

    float lastHandOff = 0;
public:
    PlayerShoot(GameObject *parent, float shellSpeed, float shellLifeTime, float shootCooldown, float shootAmount, float shootAngle, Joystick *joystick);
    void setSpawnFunction(std::function<GameObject *(float speed, Vector2 direction, float lifeTime, Vector2 position)> createShell);
    void Update();
    void Draw();
    Component *Clone(GameObject *parent);
};

class Orbit : public Component {
private:
    GameObject *target = nullptr;
    float radius = 0;
    float angle = 0;
    SpriteRenderer *spRenderer = nullptr;
    Vector2 lastDirection = Vector2(0, 1);
    Vector2 originalForward = Vector2(0, 1);
    Joystick *joystick = nullptr;

public:
    Orbit(GameObject *parent, GameObject *target, float radius, Vector2 originalForward, Joystick *joystick);
    void Update();
    void Draw();
    Component *Clone(GameObject *parent);
};

class FLipToVelocity : public Component {
private:
    SpriteRenderer *spRenderer = nullptr;
    Rigidbody2D *rigidbody = nullptr;
    Vector2 origin;

public:
    FLipToVelocity(GameObject *parent, Vector2 origin);
    void Update();
    void Draw();
    Component *Clone(GameObject *parent);
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
    Camera(GameObject *parent, GameObject *follow, Vector2 size, Vector2 offset, float speed, Vector2 deadZone);
    void Update();
    void Draw();
    Vector2 WorldToScreen(Vector2 worldPos);
    Vector2 ScreenToWorld(Vector2 screenPos);
    void SetFollow(GameObject *follow);
    Component *Clone(GameObject *parent);
};

class HPController : public Component {
private:
    int maxHP;
    int currentHP;
    bool isDead = false;
    bool isInvincible = false;

    float invincibleTime = 0;
    float lastDamageTime = 0;

    ParticleSystem *particleSystem = nullptr;
public:
    Event<> OnDeath = Event<>();
    Event<> OnDamage = Event<>();
    Event<> OnHPChange = Event<>();
    HPController(GameObject *parent, int maxHP, float invincibleTime);

    void Update();
    void Draw();
    void TakeDamage(int damage);
    void Heal(int amount);

    void SetInvincible(bool invincible);
    void SetParticleSystem(ParticleSystem *particleSystem);

    int GetCurrentHP();
    int GetMaxHP();

    bool IsDead();

    Component *Clone(GameObject *parent);
};

#endif