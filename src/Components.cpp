#include "Components.hpp"
#include "cmath"

AutoDestroy::AutoDestroy(GameObject *parent, float timeToDestroy) : Component(parent) {
    this->timeToDestroy = timeToDestroy;
    this->startTime = SDL_GetTicks();
}

void AutoDestroy::Update() {
    if (SDL_GetTicks() - startTime > timeToDestroy) {
        GameObject::Destroy(gameObject->GetName());
    }
}

void Draw() {}

Component *AutoDestroy::Clone(GameObject *parent) {
    AutoDestroy *newAutoDestroy = new AutoDestroy(parent, timeToDestroy);
    return newAutoDestroy;
}

ParticleSystem::ParticleSystem(GameObject *parent, GameObject *particlePrefab, float spawnRate, float particleLifeTime, Vector2 emitDirection, float emitForce, float emitAngle) : Component(parent) {
    this->particlePrefab = particlePrefab;
    this->spawnRate = spawnRate;
    this->particleLifeTime = particleLifeTime;

    this->emitDirection = emitDirection;
    this->emitForce = emitForce;
    this->emitAngle = emitAngle;

    lastSpawnTime = SDL_GetTicks();
}

void ParticleSystem::Update() {
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
            // Emit in random angles around emitDirection
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

void ParticleSystem::Draw() {}

void ParticleSystem::setEmitDirection(Vector2 direction) {
    emitDirection = direction;
}

void ParticleSystem::Play() {
    isPlaying = true;
}

void ParticleSystem::Stop() {
    isPlaying = false;
}

void ParticleSystem::Emit(int time) {
    emitTime += time;
}

Component *ParticleSystem::Clone(GameObject *parent) {
    ParticleSystem *newParticleSystem = new ParticleSystem(parent, particlePrefab, spawnRate, particleLifeTime, emitDirection, emitForce, emitAngle);
    return newParticleSystem;
}

#pragma region Game specifics

Joystick::Joystick(GameObject *parent, SDL_Keycode upKey, SDL_Keycode downKey, SDL_Keycode leftKey, SDL_Keycode rightKey) : Component(parent) {
    this->upKey = upKey;
    this->downKey = downKey;
    this->leftKey = leftKey;
    this->rightKey = rightKey;
}

void Joystick::SetKey(SDL_Keycode upKey, SDL_Keycode downKey, SDL_Keycode leftKey, SDL_Keycode rightKey) {
    this->upKey = upKey;
    this->downKey = downKey;
    this->leftKey = leftKey;
    this->rightKey = rightKey;
}

Vector2 Joystick::GetDirection() {
    direction = Vector2(leftSpeed + rightSpeed, upSpeed + downSpeed);
    return direction.Normalize();
}

void Joystick::Update() {
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

void Joystick::Draw() {}

Component *Joystick::Clone(GameObject *parent) {
    Joystick *newJoystick = new Joystick(parent, upKey, downKey, leftKey, rightKey);
    return newJoystick;
}

MovementController::MovementController(GameObject *parent, float speed, float jumpForce, Joystick *joystick) : Component(parent) {
    this->speed = speed;
    this->jumpForce = jumpForce;

    this->rigidbody = this->gameObject->GetComponent<Rigidbody2D>();
    this->joystick = this->gameObject->GetComponent<Joystick>();

    SDL_KeyCode upKey, downKey, leftKey, rightKey;

    this->joystick = joystick;
}

void MovementController::Update() {
    if (!enabled)
        return;
    if (rigidbody == nullptr) {
        rigidbody = gameObject->GetComponent<Rigidbody2D>();
        if (rigidbody == nullptr)
            return;
    }
    if (joystick == nullptr) {
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

void MovementController::Enable() {
    enabled = true;
}
void MovementController::Disable() {
    enabled = false;
}
bool MovementController::GetEnabled() {
    return enabled;
}

void MovementController::Draw() {}

Component *MovementController::Clone(GameObject *parent) {
    MovementController *newMovementController = new MovementController(parent, speed, jumpForce, joystick);
    return newMovementController;
}

void JumpController::OnCollisionEnter(Collider2D *collider) {
    grounded = false;
    lastNormal = Vector2(0, 0);

    if (SDL_GetTicks() - lastJumpTime < cooldown)
        return;
    if (collider->gameObject->layer != groundLayer)
        return;

    // > 0 for wall jump
    Vector2 normal = collider->GetNormal(gameObject->transform.position);
    lastNormal = normal;
    if (normal.y > 0)
        return;

    grounded = true;
}

JumpController::JumpController(GameObject *parent, SDL_KeyCode jumpKey,
                               float jumpForce, float cooldown, CollisionMatrix::Layers whatIsGround) : Component(parent) {
    this->jumpKey = jumpKey;
    this->jumpForce = jumpForce;
    this->cooldown = cooldown;

    this->groundLayer = whatIsGround;

    this->rigidbody = this->gameObject->GetComponent<Rigidbody2D>();
}

void JumpController::Update() {
    if (!enabled)
        return;
    if (rigidbody == nullptr) {
        rigidbody = gameObject->GetComponent<Rigidbody2D>();
        if (rigidbody == nullptr)
            return;
    }

    if (SDL_GetTicks() - lastJumpTime < cooldown)
        return;
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

void JumpController::Draw() {}

void JumpController::BindCollider(Collider2D *collider) {
    collider->OnCollisionEnter.addHandler([this](Collider2D *collider) {
        OnCollisionEnter(collider);
    });
}

Component *JumpController::Clone(GameObject *parent) {
    JumpController *newJumpController = new JumpController(parent, jumpKey, jumpForce, cooldown, groundLayer);
    return newJumpController;
}

PlayerAnimController::PlayerAnimController(GameObject *parent) : Component(parent) {
    animator = gameObject->GetComponent<Animator>();
    rigidbody = gameObject->GetComponent<Rigidbody2D>();
}

void PlayerAnimController::Update() {
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

void PlayerAnimController::Draw() {}

Component *PlayerAnimController::Clone(GameObject *parent) {
    PlayerAnimController *newPlayerAnimController = new PlayerAnimController(parent);
    return newPlayerAnimController;
}

ShellBehavior::ShellBehavior(GameObject *parent, float lifeTime, float speed, Vector2 direction) : Component(parent) {
    this->lifeTime = lifeTime;
    this->speed = speed;
    this->direction = direction;

    this->rigidbody = gameObject->GetComponent<Rigidbody2D>();
    if (!rigidbody)
        this->rigidbody = dynamic_cast<Rigidbody2D *>(gameObject->AddComponent(new Rigidbody2D(gameObject, 1, 0.001, 0, 0.0)));

    this->startTime = SDL_GetTicks();
}

void ShellBehavior::Update() {
    if (SDL_GetTicks() - startTime > lifeTime) {
        GameObject::Destroy(gameObject->GetName());
        return;
    }

    if (rigidbody == nullptr)
        return;

    rigidbody->velocity = direction * speed;
}

void ShellBehavior::Draw() {}

Component *ShellBehavior::Clone(GameObject *parent) {
    ShellBehavior *newShellBehavior = new ShellBehavior(parent, lifeTime, speed, direction);
    return newShellBehavior;
}

/*Spawn shell and apply direction
call setSpawnFunction to set the function that will create the shell
*/

PlayerShoot::PlayerShoot(GameObject *parent, float shellSpeed, float shellLifeTime, float shootCooldown,
                         float shootAmount, float shootAngle, Joystick *joystick) : Component(parent) {
    this->shellSpeed = shellSpeed;
    this->shellLifetime = shellLifeTime;

    this->shootCooldown = shootCooldown;
    this->shootAmount = shootAmount;
    this->shootAngle = shootAngle;

    particleSystem = gameObject->GetComponent<ParticleSystem>();
    this->joystick = joystick;
}

void PlayerShoot::setSpawnFunction(std::function<GameObject *(float speed, Vector2 direction, float lifeTime, Vector2 position)> createShell) {
    this->createShell = createShell;
}

void PlayerShoot::Update() {
    if (createShell == nullptr)
        return;
    if (joystick == nullptr) {
        joystick = gameObject->GetComponent<Joystick>();
        if (joystick == nullptr)
            return;
    }

    bool shoot = false;

    // Get shoot direction
    if (joystick->GetDirection().Magnitude() > VELOCITY_EPS) {
        shoot = true;
        lastDirection = joystick->GetDirection().Normalize();
    }

    if (shoot && SDL_GetTicks() - lastShootTime > shootCooldown) {

        for (int i = 0; i < shootAmount; i++) {

            // Rotate direction
            Vector2 direction = Vector2::Rotate(lastDirection, (rand() % (int)shootAngle * 2 - (int)shootAngle));

            GameObject *shell = createShell(shellSpeed, direction, shellLifetime, gameObject->transform.position);

            GameObjectManager::GetInstance()->AddGameObject(shell);
        }
        lastShootTime = SDL_GetTicks();

        if (particleSystem) {
            particleSystem->Emit(1);
        }
    }
}

void PlayerShoot::Draw() {}

Component *PlayerShoot::Clone(GameObject *parent) {
    PlayerShoot *newPlayerShoot = new PlayerShoot(parent, shellSpeed, shellLifetime, shootCooldown, shootAmount, shootAngle, joystick);
    return newPlayerShoot;
}

Orbit::Orbit(GameObject *parent, GameObject *target, float radius, Vector2 originalForward, Joystick *joystick) : Component(parent) {
    this->target = target;
    this->radius = radius;
    this->originalForward = originalForward;
    lastDirection = originalForward;

    spRenderer = gameObject->GetComponent<SpriteRenderer>();
    this->joystick = joystick;
}

void Orbit::Update() {
    if (target == nullptr)
        return;

    if (joystick == nullptr) {
        joystick = gameObject->GetComponent<Joystick>();
        if (joystick == nullptr)
            return;
    }

    if (!spRenderer) {
        spRenderer = gameObject->GetComponent<SpriteRenderer>();
        if (!spRenderer)
            return;
    }

    if (joystick->GetDirection().Magnitude() > VELOCITY_EPS)
        lastDirection = joystick->GetDirection().Normalize();

    angle = Vector2::SignedAngle(originalForward, lastDirection);
    gameObject->transform.position = target->transform.position + Vector2::Rotate(originalForward, angle) * radius;
    gameObject->transform.rotation = angle;

    // Flip if positioned backwards
    spRenderer->isFlippedV = Vector2::Dot(lastDirection, originalForward) < 0;
}

void Orbit::Draw() {}

Component *Orbit::Clone(GameObject *parent) {
    Orbit *newOrbit = new Orbit(parent, target, radius, originalForward, joystick);
    return newOrbit;
}

#pragma endregion

FLipToVelocity::FLipToVelocity(GameObject *parent, Vector2 origin) : Component(parent) {
    rigidbody = parent->GetComponent<Rigidbody2D>();
    spRenderer = parent->GetComponent<SpriteRenderer>();

    this->origin = origin;
}

void FLipToVelocity::Update() {
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

void FLipToVelocity::Draw() {}

Component *FLipToVelocity::Clone(GameObject *parent) {
    FLipToVelocity *newFlipToVelocity = new FLipToVelocity(parent, origin);
    return newFlipToVelocity;
}

Camera::Camera(GameObject *parent, GameObject *follow, Vector2 size, Vector2 offset, float speed, Vector2 deadZone) : Component(parent) {
    this->follow = follow;

    this->size = size;
    this->offset = offset;

    this->deadZone = deadZone;

    this->speed = speed;

    rigidbody = gameObject->GetComponent<Rigidbody2D>();
}

void Camera::Update() {
    if (follow == nullptr)
        return;

    if (!rigidbody) {
        rigidbody = gameObject->GetComponent<Rigidbody2D>();
        if (!rigidbody)
            return;
    }

    Vector2 followPos = follow->transform.position + offset;

    // Deadzone check
    bool move = false;
    if (fabs(followPos.x - gameObject->transform.position.x) > deadZone.x)
        move = true;
    if (fabs(followPos.y - gameObject->transform.position.y) > deadZone.y)
        move = true;

    // Only move if the target is out of deadzone
    if (move) {
        rigidbody->AddForce((follow->transform.position - gameObject->transform.position) * speed * 1 / FPS);
    }
}

void Camera::Draw() {}

Vector2 Camera::WorldToScreen(Vector2 worldPos) {
    Vector2 screenPos = worldPos - gameObject->transform.position + size / 2;
    return screenPos;
}

Vector2 Camera::ScreenToWorld(Vector2 screenPos) {
    Vector2 worldPos = screenPos + gameObject->transform.position - size / 2;
    return worldPos;
}

void Camera::SetFollow(GameObject *follow) {
    this->follow = follow;
}

Component *Camera::Clone(GameObject *parent) {
    Camera *newCamera = new Camera(parent, follow, size, offset, speed, deadZone);
    return newCamera;
}
