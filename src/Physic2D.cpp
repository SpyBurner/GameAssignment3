#include "Physic2D.hpp"
#include <math.h>
CollisionManager *CollisionManager::instance = nullptr;

#pragma region Rigidbody2D

Rigidbody2D::Rigidbody2D(GameObject *parent, float mass, float drag, float bounciness, float gravityScale) : Component(parent) {
    this->mass = mass;
    this->velocity = Vector2(0, 0);
    this->acceleration = Vector2(0, 0);
    this->drag = drag;
    this->bounciness = bounciness;

    this->gravityScale = gravityScale;
}

Rigidbody2D::~Rigidbody2D() {}

void Rigidbody2D::Update() {
    this->velocity += this->acceleration + gravityScale * Vector2(0, 1) * GRAVITY_ACCELERATION;
    this->acceleration = Vector2(0, 0);

    this->velocity = this->velocity * (1 - this->drag);

    const float MINIMUM_VELOCITY = 0.05f;
    if (this->velocity.Magnitude() < MINIMUM_VELOCITY) {
        this->velocity = Vector2(0, 0);
    }

    this->gameObject->transform.position += this->velocity;
}

void Rigidbody2D::Draw() {}

void Rigidbody2D::AddForce(Vector2 force) {
    this->acceleration += force / this->mass;
}

void Rigidbody2D::SetDrag(float drag) {
    this->drag = drag;
}

void Rigidbody2D::SetBounciness(float bounciness) {
    this->bounciness = bounciness;
}

void Rigidbody2D::BounceOff(Vector2 normal) {
    if (Vector2::Dot(this->velocity, normal) > 0) {
        return;
    }
    
    this->acceleration = Vector2(0, 0);
    // if (velocity.Magnitude() < 0.01f) {
    //     velocity = normal * 2.0f;
    // }
    this->velocity = Reflect(this->velocity, normal) * this->bounciness;
}

Vector2 Rigidbody2D::Reflect(Vector2 velocity, Vector2 normal) {
    // if (normal.Magnitude() < EPS) {
    //     //Ensure minimum velocity
    //     velocity = velocity.Normalize();
    // }
    return velocity - 2 * (velocity.Dot(normal)) * normal;
}

Component *Rigidbody2D::Clone(GameObject *parent) {
    Rigidbody2D *newRigidbody = new Rigidbody2D(parent, this->mass, this->drag, this->bounciness, this->gravityScale);
    return newRigidbody;
}

#pragma endregion

#pragma region Collider2D

// Collider2D Implementation
Collider2D::Collider2D(GameObject *parent, Vector2 offset, bool isTrigger) : Component(parent) {
    this->offset = offset;
    CollisionManager::GetInstance()->AddCollider(this);

    this->isTrigger = isTrigger;
    if (!isTrigger){
        this->OnCollisionEnter.addHandler([this](Collider2D *collider){
            defaultCollision(collider);
        });
    }

    // Default layer set to parent's layer
    this->layer = parent->layer;
}

// Default setting used for physical collision
void Collider2D::defaultCollision(Collider2D *other) {
    if (other->isTrigger) return;
    if (other->layer == CollisionMatrix::DETECTION) return;
    Rigidbody2D *rb = other->gameObject->GetComponent<Rigidbody2D>();
    if (rb != nullptr) {
        Vector2 normal = this->GetNormal(other->gameObject->transform.position + other->offset);

        // Project velocity onto the collision normal
        Vector2 projected = Vector2::ProjectToVector(rb->velocity, normal);

        if (projected.Magnitude() * rb->bounciness > rb->gravityScale * GRAVITY_ACCELERATION) {
            rb->BounceOff(normal);
            return;
        }

        if (Vector2::Dot(projected, normal) < 0) {
            other->gameObject->transform.position += projected * -1;
            rb->velocity = rb->velocity - projected;
            if (normal == Vector2(0, 1)) {
                rb->velocity = rb->velocity + Vector2(0, -1) * rb->gravityScale * GRAVITY_ACCELERATION;
            }
        }
    }
}

Collider2D::~Collider2D() {
    CollisionManager::GetInstance()->RemoveCollider(this);
}

void Collider2D::Update() {}
void Collider2D::Draw() {}

void Collider2D::SetOffset(Vector2 offset) {
    this->offset = offset;
}

// CollisionManager Implementation
CollisionManager::CollisionManager() {}

CollisionManager *CollisionManager::GetInstance() {
    if (instance == nullptr) {
        instance = new CollisionManager();
    }
    return instance;
}

CollisionManager::~CollisionManager() {
    this->Clear();
}

void CollisionManager::AddCollider(Collider2D *collider) {
    this->colliders.push_back(collider);
}

void CollisionManager::RemoveCollider(Collider2D *collider) {
    for (int i = 0; i < this->colliders.size(); i++) {
        if (this->colliders[i] == collider) {
            this->colliders.erase(this->colliders.begin() + i);
            return;
        }
    }
}

void CollisionManager::Update() {

    for (size_t i = 0; i < this->colliders.size(); ++i) {
        Collider2D *collider1 = this->colliders[i];
        if (!collider1->enabled) {
            continue;
        }
        for (size_t j = i + 1; j < this->colliders.size(); ++j) {
            Collider2D *collider2 = this->colliders[j];
            if (!collider2->enabled ||
            (!CollisionMatrix::checkCollisionMatrix(collider1->layer, collider2->layer))
            ) {
                continue;
            }
            if (collider1->CheckCollision(collider2)) {
                collider1->OnCollisionEnter.raise(collider2);
                collider2->OnCollisionEnter.raise(collider1);
            }
        }
    }
}

void CollisionManager::Clear() {
    this->colliders.clear();
}

// CircleCollider2D Implementation
CircleCollider2D::CircleCollider2D(GameObject *parent, Vector2 offset, float radius, bool isTrigger) : Collider2D(parent, offset, isTrigger) {
    this->radius = radius;
}

CircleCollider2D::~CircleCollider2D() {}

void CircleCollider2D::SetRadius(float radius){
    this->radius = radius;
}

Component *CircleCollider2D::Clone(GameObject *parent) {
    CircleCollider2D *newCollider = new CircleCollider2D(parent, this->offset, this->radius, this->isTrigger);
    return newCollider;
}

bool CircleCollider2D::CheckCollision(Collider2D *other) {
    if (dynamic_cast<CircleCollider2D *>(other)) {
        return this->CheckCollision(dynamic_cast<CircleCollider2D *>(other));
    } else if (dynamic_cast<BoxCollider2D *>(other)) {
        return this->CheckCollision(dynamic_cast<BoxCollider2D *>(other));
    }
}

bool CircleCollider2D::CheckCollision(CircleCollider2D *other) {
    Vector2 thisPosition = this->gameObject->transform.position + this->offset;
    Vector2 otherPosition = other->gameObject->transform.position + other->offset;
    
    float distance = (thisPosition - otherPosition).Magnitude();
    return distance < (this->radius + other->radius);
}

bool CircleCollider2D::CheckCollision(BoxCollider2D *other) {
    return ::CheckCollision(this, other);
}

bool CircleCollider2D::CheckCollision(SDL_Rect rect){
    Vector2 thisPosition = this->gameObject->transform.position + this->offset;
    Vector2 closestPoint = Vector2(
        std::max((float)rect.x, std::min(thisPosition.x, (float)rect.x + rect.w)),
        std::max((float)rect.y, std::min(thisPosition.y, (float)rect.y + rect.h))
    );
    float distance = (thisPosition - closestPoint).Magnitude();
    return distance < this->radius;
}

bool CircleCollider2D::CheckCollision(Vector2 point) {
    Vector2 thisPosition = this->gameObject->transform.position + this->offset;
    float distance = (thisPosition - point).Magnitude();
    return distance < this->radius;
}

Vector2 CircleCollider2D::GetNormal(Vector2 point) {
    return (point - this->gameObject->transform.position).Normalize();
}

// BoxCollider2D Implementation
BoxCollider2D::BoxCollider2D(GameObject *parent, Vector2 offset, Vector2 size, bool isTrigger) : Collider2D(parent, offset, isTrigger) {
    this->size = size;
}

BoxCollider2D::~BoxCollider2D() {}

void BoxCollider2D::SetSize(Vector2 size) {
    this->size = size;
}

Component *BoxCollider2D::Clone(GameObject *parent) {
    BoxCollider2D *newCollider = new BoxCollider2D(parent, this->offset, this->size, this->isTrigger);
    return newCollider;
}

bool BoxCollider2D::CheckCollision(Collider2D *other) {

    if (dynamic_cast<CircleCollider2D *>(other)) {
        return this->CheckCollision(dynamic_cast<CircleCollider2D *>(other));
    } else if (dynamic_cast<BoxCollider2D *>(other)) {
        return this->CheckCollision(dynamic_cast<BoxCollider2D *>(other));
    }
}

bool BoxCollider2D::CheckCollision(CircleCollider2D *other) {
    return ::CheckCollision(other, this);
}

bool BoxCollider2D::CheckCollision(BoxCollider2D *other) {
    Vector2 box1Min = this->gameObject->transform.position - this->size / 2 + this->offset;
    Vector2 box1Max = this->gameObject->transform.position + this->size / 2 + this->offset;
    Vector2 box2Min = other->gameObject->transform.position - other->size / 2 + other->offset;
    Vector2 box2Max = other->gameObject->transform.position + other->size / 2 + other->offset;

    bool collisionX = box1Max.x >= box2Min.x && box1Min.x <= box2Max.x;
    bool collisionY = box1Max.y >= box2Min.y && box1Min.y <= box2Max.y;

    return collisionX && collisionY;
}

bool BoxCollider2D::CheckCollision(SDL_Rect rect){
    Vector2 boxMin = this->gameObject->transform.position - this->size / 2 + this->offset;
    Vector2 boxMax = this->gameObject->transform.position + this->size / 2 + this->offset;

    bool collisionX = boxMax.x >= rect.x && boxMin.x <= rect.x + rect.w;
    bool collisionY = boxMax.y >= rect.y && boxMin.y <= rect.y + rect.h;

    bool insideX = boxMin.x >= rect.x && boxMax.x <= rect.x + rect.w;
    bool insideY = boxMin.y >= rect.y && boxMax.y <= rect.y + rect.h;

    return (collisionX && collisionY) || (insideX && insideY);
}

bool BoxCollider2D::CheckCollision(Vector2 point) {
    Vector2 boxMin = this->gameObject->transform.position - this->size / 2 + this->offset;
    Vector2 boxMax = this->gameObject->transform.position + this->size / 2 + this->offset;

    return point.x >= boxMin.x && point.x <= boxMax.x && point.y >= boxMin.y && point.y <= boxMax.y;
}

Vector2 BoxCollider2D::GetNormal(Vector2 point) {
    Vector2 boxMin = this->gameObject->transform.position - this->size / 2 + this->offset;
    Vector2 boxMax = this->gameObject->transform.position + this->size / 2 + this->offset;

    // Calculate distances to each side of the box
    float leftDist = point.x - boxMin.x;
    float rightDist = boxMax.x - point.x;
    float bottomDist = point.y - boxMin.y;
    float topDist = boxMax.y - point.y;

    // Find the minimum distance to determine the closest side
    float minDist = std::min(leftDist, std::min(rightDist, std::min(bottomDist, topDist)));

    if (minDist == leftDist) {
        return Vector2(-1, 0); // Left side
    }
    if (minDist == rightDist) {
        return Vector2(1, 0); // Right side
    }
    if (minDist == bottomDist) {
        return Vector2(0, -1); // Bottom side
    }
    if (minDist == topDist) {
        return Vector2(0, 1); // Top side
    }

    // Fallback (should not reach here)
    return Vector2(0, 0);
}

// General Collision Functions
bool CheckCollision(CircleCollider2D *circle, BoxCollider2D *box) {
    // Calculate the circle's center with offset
    Vector2 circleCenter = circle->gameObject->transform.position + circle->offset;

    // Calculate the box's min and max points with offset
    Vector2 boxMin = box->gameObject->transform.position - box->size / 2 + box->offset;
    Vector2 boxMax = box->gameObject->transform.position + box->size / 2 + box->offset;

    // Find the closest point on the box to the circle
    float closestX = std::max(boxMin.x, std::min(circleCenter.x, boxMax.x));
    float closestY = std::max(boxMin.y, std::min(circleCenter.y, boxMax.y));

    // Calculate the distance between the circle's center and this closest point
    float distanceX = circleCenter.x - closestX;
    float distanceY = circleCenter.y - closestY;

    // If the distance is less than the circle's radius, an intersection occurs
    float distanceSquared = (distanceX * distanceX) + (distanceY * distanceY);
    return distanceSquared < (circle->radius * circle->radius);
}

#pragma endregion