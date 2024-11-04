#ifndef AI_HPP
#define AI_HPP

#include "Global.hpp"
#include "Core.hpp"
#include "Physic2D.hpp"
#include "Components.hpp"

class MeleeAI : public Component {
private:
    enum State {
        IDLE,
        WALK,
        ATTACK,
    } state = WALK;

    GameObject *target = nullptr;

    HPController *hp = nullptr;
    Rigidbody2D *rb = nullptr;
    Animator *animator = nullptr;
    BoxCollider2D *baseCol = nullptr;

    //Movement
    float speed = 1.0f;
    Vector2 walkDirection = Vector2(1, 0);
    void Move();

    //Attack
    std::function<GameObject *(Vector2 direction, float lifeTime, Vector2 position)> createAttack = nullptr;
    float attackRange = 1.0f;
    float attackCooldown = 1.0f;
    float lastAttackTime = 0;
    void Attack();
    
    //Detection
    void Init();
    bool wallDetected = false;
    bool floorDetected = false;
    BoxCollider2D *wallDetectionColl = nullptr;
    BoxCollider2D *cliffDetectionColl = nullptr;
    void ResetDetection();

public:
    MeleeAI(GameObject *parent, float speed, float attackRange, float attackCooldown);
    ~MeleeAI();

    void SetCreateAttack(std::function<GameObject *(Vector2 direction, float lifeTime, Vector2 position)>);
    void SetTarget(GameObject *target);

    void Update();
    void Draw();

    Component *Clone(GameObject *parent);
};

class RangedAI : public Component {
private:
    enum State {
        WALK,
    } state = WALK;

    GameObject *target = nullptr;

    HPController *hp = nullptr;
    Rigidbody2D *rb = nullptr;
    Animator *animator = nullptr;
    BoxCollider2D *baseCol = nullptr;

    //Movement
    float speed = 1.0f;
    void Move();

    //Attack
    std::function<GameObject *(Vector2 direction, float speed, float lifeTime, Vector2 position)> createAttack = nullptr;
    float attackRange = 1.0f;
    float attackCooldown = 1.0f;
    float lastAttackTime = 0;
    void Attack();
    
public:
    RangedAI(GameObject *parent, float speed, float attackRange, float attackCooldown);
    ~RangedAI();

    void SetCreateAttack(std::function<GameObject *(Vector2 direction, float speed, float lifeTime, Vector2 position)>);
    void SetTarget(GameObject *target);

    void Update();
    void Draw();

    Component *Clone(GameObject *parent);
};

#endif