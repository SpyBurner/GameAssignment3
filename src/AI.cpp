#include "AI.hpp"

#pragma region MeleeAI

MeleeAI::MeleeAI(GameObject *parent, float speed,  float attackRange, float attackCooldown) : Component(parent) {
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

    //Wall detection collider is exactly the half the size of the base collider
    wallDetectionColl = new BoxCollider2D(gameObject,
                                          Vector2(0, 0),
                                          baseCol->size / 2,
                                          true);
    wallDetectionColl->OnCollisionEnter.addHandler([this](Collider2D *collider) {
        if (collider->gameObject->layer == CollisionMatrix::WALL) {
            wallDetected = true;
        }
    });

    //Cliff detection collider is half width of the base collider
    cliffDetectionColl = new BoxCollider2D(gameObject,
                                           Vector2(0, 0),
                                           Vector2(baseCol->size.x/2, baseCol->size.y),
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

        //In range & in front & off cooldown
        if (target && distance.Magnitude() <= attackRange
            && Vector2::Dot(walkDirection, distance) > 0
            && SDL_GetTicks() - lastAttackTime >= attackCooldown) {
            state = ATTACK;
        }
        else
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

//Speed grows as HP lowers
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
        Vector2(baseCol->size.x / 2 * walkDirection.x, 0)
    );

    cliffDetectionColl->SetOffset(
        Vector2(baseCol->size.x / 2 * walkDirection.x, baseCol->size.y / 2)
    );
}

#pragma endregion