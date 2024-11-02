#include "Helper.hpp"
// Player cosmetics

void RotateTowardVelocity::Update() {
    if (rigidbody == nullptr) {
        rigidbody = gameObject->GetComponent<Rigidbody2D>();
    }
    if (rigidbody == nullptr)
        return;

    if (rigidbody->velocity.Magnitude() > 0.1f)
        lastVelocity = rigidbody->velocity;
    gameObject->transform.rotation = Vector2::SignedAngle(originalForward, lastVelocity);
}

void RotateTowardVelocity::Draw() {}

Component *RotateTowardVelocity::Clone(GameObject *parent) {
    RotateTowardVelocity *newRotateTowardVelocity = new RotateTowardVelocity(parent, originalForward);
    return newRotateTowardVelocity;
}

RotateTowardVelocity::RotateTowardVelocity(GameObject *parent, Vector2 originalForward) : Component(parent) {
    this->originalForward = originalForward;
    rigidbody = gameObject->GetComponent<Rigidbody2D>();
}

VelocityToAnimSpeedController::VelocityToAnimSpeedController(GameObject *parent, std::string animName, float speedMultiplier, bool useRB) : Component(parent) {
    this->animName = animName;
    this->backupRigidbody = backupRigidbody;
    this->speedMultiplier = speedMultiplier;

    this->useRB = useRB;

    lastPosition = gameObject->transform.position;
}

VelocityToAnimSpeedController::~VelocityToAnimSpeedController() {}

void VelocityToAnimSpeedController::Update() {
    if (rigidbody == nullptr || animator == nullptr) {
        rigidbody = gameObject->GetComponent<Rigidbody2D>();
        animator = gameObject->GetComponent<Animator>();
    }

    if (rigidbody == nullptr || animator == nullptr)
        return;

    if (animator->GetCurrentClip()->GetName() != animName)
        return;

    if (useRB) {
        if (rigidbody->velocity.Magnitude() > VELOCITY_EPS) {
            if (animator->GetCurrentClip()->isPlaying == false)
                animator->Play(animName);
            animator->GetCurrentClip()->speedScale = rigidbody->velocity.Magnitude() * speedMultiplier;
        } else {
            if (backupRigidbody != nullptr && backupRigidbody->velocity.Magnitude() > VELOCITY_EPS) {
                if (animator->GetCurrentClip()->GetName() == animName && animator->GetCurrentClip()->isPlaying == false)
                    animator->Play(animName);
                animator->GetCurrentClip()->speedScale = backupRigidbody->velocity.Magnitude() * speedMultiplier;
            } else {
                animator->GetCurrentClip()->speedScale = 1.0;
                animator->Stop();
            }
        }
    } else {
        Vector2 velocity = (gameObject->transform.position - lastPosition);

        if ((gameObject->transform.position - lastPosition).Magnitude() > VELOCITY_EPS * 10) {
            if (animator->GetCurrentClip()->isPlaying == false)
                animator->Play(animName);
            animator->GetCurrentClip()->speedScale = (gameObject->transform.position - lastPosition).Magnitude() * speedMultiplier;
        } else {
            animator->GetCurrentClip()->speedScale = 1.0;
            animator->Stop();
        }
    }
    lastPosition = gameObject->transform.position;
}

void VelocityToAnimSpeedController::Draw() {}

void VelocityToAnimSpeedController::SetBackupRigidbody(Rigidbody2D *backupRigidbody) {
    this->backupRigidbody = backupRigidbody;
}

Component *VelocityToAnimSpeedController::Clone(GameObject *parent) {
    VelocityToAnimSpeedController *newRollSpeedController = new VelocityToAnimSpeedController(parent, this->animName, this->speedMultiplier, this->useRB);
    return newRollSpeedController;
}

StayInBounds::StayInBounds(GameObject *parent, bool teleport) : Component(parent) {
    this->teleport = teleport;
}

StayInBounds::~StayInBounds() {}

void StayInBounds::Update() {
    Rigidbody2D *rigidbody = this->gameObject->GetComponent<Rigidbody2D>();
    bool bounced = false;
    if (this->gameObject->transform.position.x < 0) {
        if (teleport) {
            this->gameObject->transform.position.x = WIDTH;
        } else {
            if (rigidbody != nullptr) {
                rigidbody->BounceOff(Vector2(1, 0));
                bounced = true;
            } else
                this->gameObject->transform.position.x = 0;
        }
    }
    if (this->gameObject->transform.position.x > WIDTH) {
        if (teleport) {
            this->gameObject->transform.position.x = 0;
        } else {
            if (rigidbody != nullptr) {
                rigidbody->BounceOff(Vector2(-1, 0));
                bounced = true;
            } else
                this->gameObject->transform.position.x = WIDTH;
        }
    }
    if (this->gameObject->transform.position.y < 0) {
        if (teleport) {
            this->gameObject->transform.position.y = HEIGHT;
        } else {
            if (rigidbody != nullptr) {
                rigidbody->BounceOff(Vector2(0, 1));
                bounced = true;
            } else
                this->gameObject->transform.position.y = 0;
        }
    }
    if (this->gameObject->transform.position.y > HEIGHT) {
        if (teleport) {
            this->gameObject->transform.position.y = 0;
        } else {
            if (rigidbody != nullptr) {
                rigidbody->BounceOff(Vector2(0, -1));
                bounced = true;
            } else
                this->gameObject->transform.position.y = HEIGHT;
        }
    }
}

void StayInBounds::Draw() {}

Component *StayInBounds::Clone(GameObject *parent) {
    StayInBounds *newStayInBounds = new StayInBounds(parent, this->teleport);
    return newStayInBounds;
}

SDL_Texture *LoadFontTexture(const std::string &text, const std::string &fontPath, SDL_Color color, int fontSize) {
    // Load the font
    TTF_Font *font = TTF_OpenFont(fontPath.c_str(), fontSize);
    if (!font) {
        std::cerr << "Failed to load font: " << TTF_GetError() << std::endl;
        return nullptr;
    }

    // Render the text to a surface
    SDL_Surface *textSurface = TTF_RenderText_Solid(font, text.c_str(), color);
    if (!textSurface) {
        std::cerr << "Failed to create text surface: " << TTF_GetError() << std::endl;
        TTF_CloseFont(font);
        return nullptr;
    }

    // Create a texture from the surface
    SDL_Texture *textTexture = SDL_CreateTextureFromSurface(RENDERER, textSurface);
    if (!textTexture) {
        std::cerr << "Failed to create texture: " << SDL_GetError() << std::endl;
    }

    // Clean up
    SDL_FreeSurface(textSurface);
    TTF_CloseFont(font);

    return textTexture;
}

void RenderTexture(SDL_Texture *texture, int x, int y) {
    if (!texture) {
        std::cerr << "Texture is null" << std::endl;
        return;
    }

    // Query the texture to get its width and height
    int width, height;
    SDL_QueryTexture(texture, nullptr, nullptr, &width, &height);

    // Define the destination rectangle
    SDL_Rect destRect = {x - width / 2, y - height / 2, width, height};

    // Render the texture
    SDL_RenderCopy(RENDERER, texture, nullptr, &destRect);
}

PositionTracker::PositionTracker(GameObject *parent) : Component(parent) {}

void PositionTracker::Update() {
    std::cout << "Position x: " << gameObject->transform.position.x << " y: " << gameObject->transform.position.y << std::endl;
}

void PositionTracker::Draw() {}

Component *PositionTracker::Clone(GameObject *parent) {
    PositionTracker *newPositionTracker = new PositionTracker(parent);
    return newPositionTracker;
}
