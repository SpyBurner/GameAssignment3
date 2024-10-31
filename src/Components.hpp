#ifndef COMPONENTS_HPP
#define COMPONENTS_HPP

#include "Core.hpp"
#include "Game.hpp"
#include "Helper.hpp"
#include "Physic2D.hpp"
#include "cmath"

#pragma region Game specifics
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

        // Vector2 force = Vector2(leftSpeed + rightSpeed, 0).Normalize() * actualSpeed + Vector2(0, upSpeed);
        Vector2 force = Vector2(leftSpeed + rightSpeed, 0).Normalize() * actualSpeed + Vector2(0, 0);

        if (force.Magnitude() > VELOCITY_EPS) {
            rigidbody->AddForce(force);
        }
        // else{
        //     //If not moving horizontally, remove horizontal speed for snappy movement
        //     rigidbody->velocity -= Vector2::ProjectToVector(rigidbody->velocity, Vector2(1, 0));
        // }
    }

    Vector2 extractSpeed() {
        return Vector2(leftSpeed + rightSpeed, upSpeed + downSpeed);
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
    
    SDL_KeyCode shootKey;
    
    float shellSpeed = 0;
    float shellLifetime = 0;

    float shootCooldown = 0;
    float lastShootTime = 0;

    int shootAmount = 1;
    float shootAngle = 0;

    GameObject* particle = nullptr;

    Vector2 lastDirection = Vector2(0, 1);
public:
    PlayerShoot(GameObject *parent, SDL_KeyCode shootKey, float shellSpeed, float shellLifeTime, float shootCooldown, 
                float shootAmount, float shootAngle) : Component(parent) {
        this->shootKey = shootKey;

        this->shellSpeed = shellSpeed;
        this->shellLifetime = shellLifeTime;

        this->shootCooldown = shootCooldown;
        this->shootAmount = shootAmount;
        this->shootAngle = shootAngle;
    }

    void setSpawnFunction(std::function<GameObject *(float speed, Vector2 direction, float lifeTime, Vector2 position)> createShell) {
        this->createShell = createShell;
    }

    void Update() {
        if (createShell == nullptr) return;

        static bool shoot = false;
        if (Game::event.type == SDL_KEYDOWN || Game::event.type == SDL_KEYUP) {
            if (Game::event.key.keysym.sym == shootKey) {
                shoot = Game::event.type == SDL_KEYDOWN;
            }
        }

        if (shoot && SDL_GetTicks() - lastShootTime > shootCooldown) {
            MovementController* movementController = gameObject->GetComponent<MovementController>();
            if (!movementController) return;
            
            //Get shoot direction
            if (movementController->extractSpeed().Magnitude() > VELOCITY_EPS)
                lastDirection = movementController->extractSpeed().Normalize();

            for (int i = 0; i < shootAmount; i++) {

                //Rotate direction
                Vector2 direction = Vector2::Rotate(lastDirection, (rand() % (int)shootAngle * 2 - (int)shootAngle));

                GameObject *shell = createShell(shellSpeed, direction, shellLifetime, gameObject->transform.position);

                GameObjectManager::GetInstance()->AddGameObject(shell);
            }
            lastShootTime = SDL_GetTicks();
        }
    }

    void Draw() {}

    Component *Clone(GameObject *parent) {
        PlayerShoot *newPlayerShoot = new PlayerShoot(parent, shootKey, shellSpeed, shellLifetime, shootCooldown, shootAmount, shootAngle);
        return newPlayerShoot;
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
public: 
    ParticleSystem(GameObject *parent, GameObject *particlePrefab, float spawnRate, float particleLifeTime, float emitForce, float emitAngle) : Component(parent) {
        this->particlePrefab = particlePrefab;
        this->spawnRate = spawnRate;
        this->particleLifeTime = particleLifeTime;
        this->emitForce = emitForce;
        this->emitAngle = emitAngle;

        lastSpawnTime = SDL_GetTicks();
    }

    void Update() {
        if (!isPlaying)
            return;
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

    Component *Clone(GameObject *parent) {
        ParticleSystem *newParticleSystem = new ParticleSystem(parent, particlePrefab, spawnRate, particleLifeTime, emitForce, emitAngle);
        return newParticleSystem;
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
