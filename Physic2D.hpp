#ifndef PHYSIC2D_HPP
#define PHYSIC2D_HPP

#include "CustomClasses.hpp"
#include "Global.hpp"


class Rigidbody2D : public Component {
private:
    Vector2 acceleration;
    float mass, drag, bounciness;

public:
    Vector2 velocity;

    Rigidbody2D(GameObject *parent, float mass, float drag, float bounciness);
    ~Rigidbody2D();

    void Update();
    void Draw();
    void AddForce(Vector2 force);

    void SetDrag(float drag);
    void SetBounciness(float bounciness);

    void BounceOff(Vector2 normal);

    static Vector2 Reflect(Vector2 velocity, Vector2 normal);

    Component *Clone(GameObject *parent);
};

class Collider2D : public Component {
public:
    Vector2 offset;
    Event<Collider2D *> OnCollisionEnter;

    Collider2D(GameObject *parent, Vector2 offset);
    ~Collider2D();

    void SetOffset(Vector2 offset);

    void Update();
    void Draw();

    virtual Component *Clone(GameObject *parent) = 0;
    virtual bool CheckCollision(Collider2D *other) = 0;
    virtual bool CheckCollision(Vector2 point) = 0;
    virtual Vector2 GetNormal(Vector2 point) = 0;
};

class CollisionManager {
private:
    CollisionManager();
    static CollisionManager *instance;

    std::vector<Collider2D *> colliders;

public:
    static CollisionManager *GetInstance();

    void AddCollider(Collider2D *collider);
    void RemoveCollider(Collider2D *collider);
    void Update();
    void Clear();
};

class CircleCollider2D;
class BoxCollider2D;

class CircleCollider2D : public Collider2D {
public:
    float radius;
    CircleCollider2D(GameObject *parent, Vector2 offset, float radius);
    ~CircleCollider2D();

    void SetRadius(float radius);

    Component *Clone(GameObject *parent);

    bool CheckCollision(Collider2D *other);

    bool CheckCollision(CircleCollider2D *other);
    bool CheckCollision(BoxCollider2D *other);

    bool CheckCollision(Vector2 point);

    Vector2 GetNormal(Vector2 point);
};

class BoxCollider2D : public Collider2D {
public:
    Vector2 size;
    BoxCollider2D(GameObject *parent, Vector2 offset, Vector2 size);
    ~BoxCollider2D();

    void SetSize(Vector2 size);

    Component *Clone(GameObject *parent);

    bool CheckCollision(Collider2D *other);

    bool CheckCollision(CircleCollider2D *other);
    bool CheckCollision(BoxCollider2D *other);

    bool CheckCollision(Vector2 point);

    Vector2 GetNormal(Vector2 point);
};

bool CheckCollision(CircleCollider2D *circle, BoxCollider2D *box);

#endif