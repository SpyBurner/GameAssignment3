#ifndef COMPONENTS_HPP
#define COMPONENTS_HPP

#include "Core.hpp"
#include "Game.hpp"
#include "Helper.hpp"
#include "Physic2D.hpp"
#include "cmath"
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
            GameObject::Destroy(gameObject->GetName());
        }
    }

    void Draw() {}

    Component *Clone(GameObject *parent) {
        AutoDestroy *newAutoDestroy = new AutoDestroy(parent, timeToDestroy);
        return newAutoDestroy;
    }
};

class ParticleSystem : public Component {
private:
    GameObject *particlePrefab;
    //Per second
    float spawnRate;
    float lastSpawnTime;
    float particleLifeTime;

    float emitForce;

    Vector2 emitDirection = Vector2(0, -1);
    float emitAngle = 0;

    bool isPlaying = true;

    int emitTime = 0;
public: 
    ParticleSystem(GameObject *parent, GameObject *particlePrefab, float spawnRate, float particleLifeTime, Vector2 emitDirection, float emitForce, float emitAngle) : Component(parent) {
        this->particlePrefab = particlePrefab;
        this->spawnRate = spawnRate;
        this->particleLifeTime = particleLifeTime;

        this->emitDirection = emitDirection;
        this->emitForce = emitForce;
        this->emitAngle = emitAngle;

        lastSpawnTime = SDL_GetTicks();
    }

    void Update() {
        if (!isPlaying && emitTime <= 0)
            return;
        if (emitTime > 0)
            emitTime--;

        if (SDL_GetTicks() - lastSpawnTime > 1000 / spawnRate) {
            GameObject *particle = GameObject::Instantiate("Particle" + std::to_string(rand() + rand()), particlePrefab, gameObject->transform.position,
                                                          particlePrefab->transform.rotation, particlePrefab->transform.scale);
            particle->AddComponent(new AutoDestroy(particle, particleLifeTime));

            Rigidbody2D *rb = particle->GetComponent<Rigidbody2D>();
            if (rb) {
                //Emit in random angles around emitDirection
                int r = 0;
                if (emitAngle > 0)
                    r = rand() % ((int)emitAngle * 2) - (int)emitAngle;
                Vector2 direction = Vector2::Rotate(emitDirection, r);
                rb->AddForce(direction * emitForce);
            }

            lastSpawnTime = SDL_GetTicks();

            GameObjectManager::GetInstance()->AddGameObject(particle);
        }
    }

    void Draw() {}

    void setEmitDirection(Vector2 direction) {
        emitDirection = direction;
    }

    void Play(){
        isPlaying = true;
    }

    void Stop(){
        isPlaying = false;
    }

    void Emit(int time){
        emitTime += time;
    }

    Component *Clone(GameObject *parent) {
        ParticleSystem *newParticleSystem = new ParticleSystem(parent, particlePrefab, spawnRate, particleLifeTime, emitDirection, emitForce, emitAngle);
        return newParticleSystem;
    }
};


#pragma region Game specifics

class Joystick : public Component{
private:
    SDL_Keycode upKey, downKey, leftKey, rightKey;

    float upSpeed = 0, downSpeed = 0, leftSpeed = 0, rightSpeed = 0;

    Vector2 direction = Vector2(0, 0);
public:
    Joystick(GameObject *parent, SDL_Keycode upKey, SDL_Keycode downKey, SDL_Keycode leftKey, SDL_Keycode rightKey) : Component(parent) {
        this->upKey = upKey;
        this->downKey = downKey;
        this->leftKey = leftKey;
        this->rightKey = rightKey;
    }

    void SetKey(SDL_Keycode upKey, SDL_Keycode downKey, SDL_Keycode leftKey, SDL_Keycode rightKey){
        this->upKey = upKey;
        this->downKey = downKey;
        this->leftKey = leftKey;
        this->rightKey = rightKey;
    }

    Vector2 GetDirection(){
        direction = Vector2(leftSpeed + rightSpeed, upSpeed + downSpeed);
        return direction.Normalize();
    }

    void Update() {
        if (!enabled)
            return;

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
    }

    void Draw() {}

    Component *Clone(GameObject *parent) {
        Joystick *newJoystick = new Joystick(parent, upKey, downKey, leftKey, rightKey);
        return newJoystick;
    }
};

class MovementController : public Component {

private:
    Rigidbody2D *rigidbody = nullptr;

    Joystick* joystick = nullptr;
public:
    float speed = 0;
    float jumpForce = 0;

    MovementController(GameObject *parent, float speed, float jumpForce, Joystick* joystick) : Component(parent) {
        this->speed = speed;
        this->jumpForce = jumpForce;

        this->rigidbody = this->gameObject->GetComponent<Rigidbody2D>();
        this->joystick = this->gameObject->GetComponent<Joystick>();

        SDL_KeyCode upKey, downKey, leftKey, rightKey;

        this->joystick = joystick;
    }

    void Update() {
        if (!enabled)
            return;
        if (rigidbody == nullptr){
            rigidbody = gameObject->GetComponent<Rigidbody2D>();
            if (rigidbody == nullptr)
                return;
        }
        if (joystick == nullptr){
            joystick = gameObject->GetComponent<Joystick>();
            if (joystick == nullptr)
                return;
        }

        float actualSpeed = speed * 1 / FPS;

        Vector2 direction = joystick->GetDirection();

        Vector2 force = Vector2(direction.x, 0).Normalize() * actualSpeed;

        if (force.Magnitude() > VELOCITY_EPS) {
            rigidbody->AddForce(force);
        }
    }

    void Enable() {
        enabled = true;
    }
    void Disable() {
        enabled = false;
    }
    bool GetEnabled() {
        return enabled;
    }

    void Draw() {}

    Component *Clone(GameObject *parent) {
        MovementController *newMovementController = new MovementController(parent, speed, jumpForce, joystick);
        return newMovementController;
    }
};

/*Grounded condition:
*/
class JumpController : public Component {
private:
    Rigidbody2D *rigidbody = nullptr;

    SDL_KeyCode jumpKey;
    float jumpForce = 0;
    
    float cooldown = 0;
    float lastJumpTime = 0;
    bool grounded = false;

    CollisionMatrix::Layers groundLayer = CollisionMatrix::DEFAULT;
    
    Vector2 lastNormal = Vector2(0, 0);

    void OnCollisionEnter(Collider2D *collider) {
        grounded = false;
        lastNormal = Vector2(0, 0);

        if (SDL_GetTicks() - lastJumpTime < cooldown) return;
        if (collider->gameObject->layer != groundLayer) return;
        
        // > 0 for wall jump
        Vector2 normal = collider->GetNormal(gameObject->transform.position);
        lastNormal = normal;
        if (normal.y > 0) return;

        grounded = true;
    }
public:

    JumpController(GameObject *parent, SDL_KeyCode jumpKey, 
                float jumpForce, float cooldown, CollisionMatrix::Layers whatIsGround) : Component(parent) {
        this->jumpKey = jumpKey;
        this->jumpForce = jumpForce;
        this->cooldown = cooldown;

        this->groundLayer = whatIsGround;

        this->rigidbody = this->gameObject->GetComponent<Rigidbody2D>();
    }

    void Update() {
        if (!enabled)
            return;
        if (rigidbody == nullptr){
            rigidbody = gameObject->GetComponent<Rigidbody2D>();
            if (rigidbody == nullptr)
                return;
        }

        if (SDL_GetTicks() - lastJumpTime < cooldown) return;
        if (Game::event.type == SDL_KEYDOWN) {
            if (Game::event.key.keysym.sym == jumpKey && grounded) {
                Vector2 direction = Vector2(0, -1);
                if (lastNormal.x != 0)
                    direction += lastNormal / 4;
                
                rigidbody->AddForce(direction.Normalize() * jumpForce);
                grounded = false;
                lastJumpTime = SDL_GetTicks();
            }
        }
    }

    void Draw() {}

    void BindCollider(Collider2D *collider) {
        collider->OnCollisionEnter.addHandler([this](Collider2D *collider) {
            OnCollisionEnter(collider);
        });
    }

    Component *Clone(GameObject *parent) {
        JumpController *newJumpController = new JumpController(parent, jumpKey, jumpForce, cooldown, groundLayer);
        return newJumpController;
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

class ShellBehavior : public Component {
private:
    Rigidbody2D *rigidbody;

    float lifeTime;
    float startTime;

    float speed;
    Vector2 direction;
public:
    ShellBehavior(GameObject *parent, float lifeTime, float speed, Vector2 direction) : Component(parent) {
        this->lifeTime = lifeTime;
        this->speed = speed;
        this->direction = direction;

        this->rigidbody = gameObject->GetComponent<Rigidbody2D>();
        if (!rigidbody)
            this->rigidbody = dynamic_cast<Rigidbody2D *>(gameObject->AddComponent(new Rigidbody2D(gameObject, 1, 0.001, 0, 0.0)));

        this->startTime = SDL_GetTicks();
    }

    void Update() {
        if (SDL_GetTicks() - startTime > lifeTime) {
            GameObject::Destroy(gameObject->GetName());
            return;
        }

        if (rigidbody == nullptr)
            return;

        rigidbody->velocity = direction * speed;
    }

    void Draw() {}

    Component *Clone(GameObject *parent) {
        ShellBehavior *newShellBehavior = new ShellBehavior(parent, lifeTime, speed, direction);
        return newShellBehavior;
    }
};

/*Spawn shell and apply direction
call setSpawnFunction to set the function that will create the shell
*/
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
public:
    PlayerShoot(GameObject *parent, float shellSpeed, float shellLifeTime, float shootCooldown, 
                float shootAmount, float shootAngle, Joystick *joystick) : Component(parent) {
        this->shellSpeed = shellSpeed;
        this->shellLifetime = shellLifeTime;

        this->shootCooldown = shootCooldown;
        this->shootAmount = shootAmount;
        this->shootAngle = shootAngle;

        particleSystem = gameObject->GetComponent<ParticleSystem>();
        this->joystick = joystick;
    }

    void setSpawnFunction(std::function<GameObject *(float speed, Vector2 direction, float lifeTime, Vector2 position)> createShell) {
        this->createShell = createShell;
    }

    void Update() {
        if (createShell == nullptr) return;
        if (joystick == nullptr){
            joystick = gameObject->GetComponent<Joystick>();
            if (joystick == nullptr) return;
        }

        bool shoot = false;

        //Get shoot direction
        if (joystick->GetDirection().Magnitude() > VELOCITY_EPS){
            shoot = true;
            lastDirection = joystick->GetDirection().Normalize();
        }

        if (shoot && SDL_GetTicks() - lastShootTime > shootCooldown) {

            for (int i = 0; i < shootAmount; i++) {

                //Rotate direction
                Vector2 direction = Vector2::Rotate(lastDirection, (rand() % (int)shootAngle * 2 - (int)shootAngle));

                GameObject *shell = createShell(shellSpeed, direction, shellLifetime, gameObject->transform.position);

                GameObjectManager::GetInstance()->AddGameObject(shell);
            }
            lastShootTime = SDL_GetTicks();

            if (particleSystem){
                particleSystem->Emit(1);
            }
        }
    }

    void Draw() {}

    Component *Clone(GameObject *parent) {
        PlayerShoot *newPlayerShoot = new PlayerShoot(parent, shellSpeed, shellLifetime, shootCooldown, shootAmount, shootAngle, joystick);
        return newPlayerShoot;
    }
};
class Orbit : public Component {
private:
    GameObject *target = nullptr;
    float radius;
    float angle = 0;

    SpriteRenderer *spRenderer = nullptr;

    Vector2 lastDirection = Vector2(0, 1);

    Vector2 originalForward = Vector2(0, 1);

    Joystick *joystick = nullptr;
public:
    Orbit(GameObject *parent, GameObject *target, float radius, Vector2 originalForward, Joystick *joystick) : Component(parent) {
        this->target = target;
        this->radius = radius;
        this->originalForward = originalForward;
        lastDirection = originalForward;

        spRenderer = gameObject->GetComponent<SpriteRenderer>();
        this->joystick = joystick;
    }

    void Update() {
        if (target == nullptr)
            return;

        if (joystick == nullptr){
            joystick = gameObject->GetComponent<Joystick>();
            if (joystick == nullptr) return;
        }

        if (!spRenderer){
            spRenderer = gameObject->GetComponent<SpriteRenderer>();
            if (!spRenderer) return;
        }

        if (joystick->GetDirection().Magnitude() > VELOCITY_EPS)
            lastDirection = joystick->GetDirection().Normalize();
        
        angle = Vector2::SignedAngle(originalForward, lastDirection);
        gameObject->transform.position = target->transform.position + Vector2::Rotate(originalForward, angle) * radius;
        gameObject->transform.rotation = angle;

        //Flip if positioned backwards
        spRenderer->isFlippedV = Vector2::Dot(lastDirection, originalForward) < 0;       
    }

    void Draw() {}

    Component *Clone(GameObject *parent) {
        Orbit *newOrbit = new Orbit(parent, target, radius, originalForward, joystick);
        return newOrbit;
    }
};

#pragma endregion
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

        spRenderer->isFlippedH = Vector2::Dot(rigidbody->velocity, origin) < 0;
    }

    void Draw() {}

    Component *Clone(GameObject *parent) {
        FLipToVelocity *newFlipToVelocity = new FLipToVelocity(parent, origin);
        return newFlipToVelocity;
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
