#ifndef HELPER_HPP
#define HELPER_HPP

#include "Global.hpp"
#include "Core.hpp"
#include "Physic2D.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

// Player cosmetics
class RotateTowardVelocity : public Component {
private:
    Vector2 originalForward;
    Rigidbody2D *rigidbody;
    Vector2 lastVelocity;

    void Update();
    void Draw();
    Component *Clone(GameObject *parent);

public:
    RotateTowardVelocity(GameObject *parent, Vector2 originalForward);
};

/* Use difference in position, or rigidbody */
class VelocityToAnimSpeedController : public Component {
private:
    Rigidbody2D *rigidbody;
    Rigidbody2D* backupRigidbody;
    Animator *animator;
    std::string animName;
    float speedMultiplier;
    Vector2 lastPosition;
    bool useRB;

public:
    VelocityToAnimSpeedController(GameObject *parent, std::string animName, float speedMultiplier = 1.0f, bool useRB = true);
    ~VelocityToAnimSpeedController();
    void Update();
    void Draw();
    void SetBackupRigidbody(Rigidbody2D* backupRigidbody);
    Component *Clone(GameObject *parent);
};

class StayInBounds : public Component {
public:
    bool teleport;
    StayInBounds(GameObject *parent, bool teleport);
    ~StayInBounds();
    void Update();
    void Draw();
    Component *Clone(GameObject *parent);
};

SDL_Texture* LoadFontTexture(const std::string& text, const std::string& fontPath, SDL_Color color, int fontSize);
void RenderTexture(SDL_Texture* texture, int x, int y);

class PositionTracker : public Component {
public:
    PositionTracker(GameObject *parent);
    void Update();
    void Draw();
    Component *Clone(GameObject *parent);
};

SDL_Texture *LoadFontTexture(const std::string &text, const std::string &fontPath, SDL_Color color, int fontSize);
void RenderTexture(SDL_Texture *texture, int x, int y);

#endif // HELPER_HPP