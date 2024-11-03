#include "AI.hpp"

#pragma region MeleeAI

MeleeAI::MeleeAI(GameObject *parent, float speed, float attackRange, float attackCooldown) : Component(parent) {
    this->speed = speed;
    this->attackRange = attackRange;
    this->attackCooldown = attackCooldown;

    Init();
}

// Init 2 detection colliders based on the base collider
void MeleeAI::Init() {
    baseCol = gameObject->GetComponent<BoxCollider2D>();
    if (baseCol == nullptr) {
        throw "BoxCollider2D not found in MeleeAI::Init()";
    }

    // Wall detection collider is exactly the half the size of the base collider
    wallDetectionColl = new BoxCollider2D(gameObject,
                                          Vector2(0, 0),
                                          baseCol->size / 2,
                                          true);
    wallDetectionColl->OnCollisionEnter.addHandler([this](Collider2D *collider) {
        if (collider->gameObject->layer == CollisionMatrix::WALL) {
            wallDetected = true;
        }
    });

    // Cliff detection collider is half width of the base collider
    cliffDetectionColl = new BoxCollider2D(gameObject,
                                           Vector2(0, 0),
                                           Vector2(baseCol->size.x / 2, baseCol->size.y),
                                           true);
    cliffDetectionColl->OnCollisionEnter.addHandler([this](Collider2D *collider) {
        if (collider->gameObject->layer == CollisionMatrix::WALL) {
            floorDetected = true;
        }
    });

    wallDetectionColl->layer = CollisionMatrix::DETECTION;
    cliffDetectionColl->layer = CollisionMatrix::DETECTION;

    gameObject->AddComponent(wallDetectionColl);
    gameObject->AddComponent(cliffDetectionColl);
}

MeleeAI::~MeleeAI() {}

void MeleeAI::SetCreateAttack(std::function<GameObject *(Vector2 direction, float lifeTime, Vector2 position)> createAttack) {
    this->createAttack = createAttack;
}

void MeleeAI::SetTarget(GameObject *target) {
    this->target = target;

    target->GetComponent<HPController>()->OnDeath.addHandler([this]() {
        this->SetTarget(nullptr);
    });
}

void MeleeAI::Update() {

    if (!hp) {
        hp = gameObject->GetComponent<HPController>();
        if (hp == nullptr) {
            throw "HPController not found in MeleeAI::Update()";
        }
    }

    if (!enabled || !target || hp->IsDead())
        return;

    if (rb == nullptr) {
        rb = gameObject->GetComponent<Rigidbody2D>();
        if (rb == nullptr) {
            throw "Rigidbody2D not found in MeleeAI::Update()";
        }
    }

    if (animator == nullptr) {
        animator = gameObject->GetComponent<Animator>();
        if (animator == nullptr) {
            throw "Animator not found in MeleeAI::Update()";
        }
    }

    if (state == IDLE) {
        animator->Play("Idle");
        state = WALK;
    } else if (state == WALK) {
        animator->Play("Walk");

        Vector2 distance = target->transform.position - gameObject->transform.position;

        // In range & in front & off cooldown
        if (target && distance.Magnitude() <= attackRange && Vector2::Dot(walkDirection, distance) > 0 && SDL_GetTicks() - lastAttackTime >= attackCooldown) {
            state = ATTACK;
        } else
            Move();
    } else if (state == ATTACK) {
        Attack();
        animator->Play("Attack");

        if (animator->GetClip("Attack")->IsFinished())
            state = WALK;
    }

    ResetDetection();
}

void MeleeAI::Draw() {}

Component *MeleeAI::Clone(GameObject *parent) {
    // Clone implementation
    return nullptr;
}

// Speed grows as HP lowers
void MeleeAI::Move() {
    if (wallDetected || !floorDetected)
        walkDirection = -1 * walkDirection;

    float newSpeed = speed / ((float)hp->GetCurrentHP() / hp->GetMaxHP());

    if (newSpeed > speed * 4) {
        newSpeed = speed * 4;
    }

    rb->velocity = Vector2(walkDirection.x * newSpeed, rb->velocity.y);
}

void MeleeAI::Attack() {
    if (SDL_GetTicks() - lastAttackTime < attackCooldown)
        return;
    GameObject *attack = createAttack(walkDirection, 400, gameObject->transform.position + walkDirection * attackRange);

    lastAttackTime = SDL_GetTicks();

    GameObjectManager::GetInstance()->AddGameObject(attack);
}

void MeleeAI::ResetDetection() {
    wallDetected = false;
    floorDetected = false;

    wallDetectionColl->SetOffset(
        Vector2(baseCol->size.x / 2 * walkDirection.x, 0));

    cliffDetectionColl->SetOffset(
        Vector2(baseCol->size.x / 2 * walkDirection.x, baseCol->size.y / 2));
}

#pragma endregion
#pragma region RangedAI

RangedAI::RangedAI(GameObject *parent, float speed, float attackRange, float attackCooldown) : Component(parent) {
    this->speed = speed;
    this->attackRange = attackRange;
    this->attackCooldown = attackCooldown;
}

RangedAI::~RangedAI() {}

void RangedAI::SetCreateAttack(std::function<GameObject *(Vector2 direction, float speed, float lifeTime, Vector2 position)> createAttack) {
    this->createAttack = createAttack;
}

void RangedAI::SetTarget(GameObject *target) {
    this->target = target;

    HPController *hp = target->GetComponent<HPController>();

    hp->OnDeath.addHandler([this]() {
        this->target = nullptr; // Fix the lambda function
    });
}

void RangedAI::Update() {
    if (!hp) {
        hp = gameObject->GetComponent<HPController>();
        if (hp == nullptr) {
            throw "HPController not found in RangedAI::Update()";
        }
    }

    if (!enabled || !target || hp->IsDead())
        return;

    if (rb == nullptr) {
        rb = gameObject->GetComponent<Rigidbody2D>();
        if (rb == nullptr) {
            throw "Rigidbody2D not found in RangedAI::Update()";
        }
    }

    if (animator == nullptr) {
        animator = gameObject->GetComponent<Animator>();
        if (animator == nullptr) {
            throw "Animator not found in RangedAI::Update()";
        }
    }

    animator->Play("Walk");
    if (target && (target->transform.position - gameObject->transform.position).Magnitude() <= attackRange) {
        if (SDL_GetTicks() - lastAttackTime >= attackCooldown) {
            Attack();
        }
    } else if (state == WALK) {
        Move();
    }
}

void RangedAI::Draw() {}

Component *RangedAI::Clone(GameObject *parent) {
    return new RangedAI(parent, speed, attackRange, attackCooldown);
}

void RangedAI::Move() {
    Vector2 direction = (target->transform.position - gameObject->transform.position).Normalize();
    rb->AddForce(direction * speed);

    if (rb->velocity.Magnitude() > speed) {
        rb->velocity = rb->velocity.Normalize() * speed;
    }
}

void RangedAI::Attack() {
    if (SDL_GetTicks() - lastAttackTime < attackCooldown)
        return;
    lastAttackTime = SDL_GetTicks();

    Vector2 compensation = target->GetComponent<Rigidbody2D>()->velocity * 2;
    Vector2 direction = (target->transform.position - gameObject->transform.position + compensation).Normalize();

    GameObject *attack = createAttack(direction, RANGED_PROJECTILE_SPEED, RANGED_PROJECTILE_LIFETIME, gameObject->transform.position);

    GameObjectManager::GetInstance()->AddGameObject(attack);
}

#pragma endregion