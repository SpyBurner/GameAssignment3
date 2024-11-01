#ifndef CUSTOMCLASSES_HPP
#define CUSTOMCLASSES_HPP

#include <SDL2/SDL.h>
#include <iostream>

#include <functional>
#include <map>
#include <utility>
#include <vector>

#include <SDL2/SDL_mixer.h>

class GameObject;

// Event
template <typename... Args>
class Event {
public:
    using Handler = std::function<void(Args...)>;

    Event() {}
    ~Event() { handlers.clear(); }

    void addHandler(Handler handler) {
        handlers.push_back(handler);
    }

    void raise(Args... args) {
        for (auto &handler : handlers) {
            handler(std::forward<Args>(args)...);
        }
    }

private:
    std::vector<Handler> handlers;
};

// Specialization for no arguments
template <>
class Event<> {
public:
    using Handler = std::function<void()>;

    Event() {}
    ~Event() { handlers.clear(); }

    void addHandler(Handler handler) {
        handlers.push_back(handler);
    }

    void raise() {
        for (auto &handler : handlers) {
            handler();
        }
    }

private:
    std::vector<Handler> handlers;
};

class Vector2 {
public:
    float x, y;
    Vector2();
    Vector2(float x, float y);
    bool operator==(Vector2 v);
    Vector2 operator+(Vector2 v);
    Vector2 operator-(Vector2 v);
    Vector2 operator*(float f);
    float operator*(Vector2 v);
    Vector2 operator/(float f);
    Vector2 operator+=(Vector2 v);

    float Magnitude();
    Vector2 Normalize();
    float Distance(Vector2 v);
    static float Distance(Vector2 v1, Vector2 v2);
    float Dot(Vector2 v);
    static float Dot(Vector2 v1, Vector2 v2);
    float Cross(Vector2 v);
    static float Cross(Vector2 v1, Vector2 v2);

    static float Angle(Vector2 v1, Vector2 v2);

    static float SignedAngle(Vector2 v1, Vector2 v2);
};

Vector2 operator*(float f, Vector2 v);

/*Singleton manager for GameObjects, automatic memory management
 */
class GameObjectManager {
private:
    std::map<std::string, GameObject *> gameObjects;
    GameObjectManager();
    static GameObjectManager *instance;
public:

    ~GameObjectManager();

    static GameObjectManager *GetInstance();
    void AddGameObject(std::vector<GameObject *> gameObjects);
    void AddGameObject(GameObject *gameObject);
    void RemoveGameObject(std::string name);
    GameObject *GetGameObject(std::string name);
    void Clear();

    void Update();
    void Draw();

};

class Component {
public:
    bool enabled = true;
    
    GameObject *gameObject = nullptr;
    Component(GameObject *parent);
    virtual ~Component();
    virtual void Update() = 0;
    virtual void Draw() = 0;
    virtual Component *Clone(GameObject *parent) = 0;
};

SDL_Texture *LoadSpriteSheet(std::string path);

class SpriteRenderer : public Component {
private:
    int drawOrder = 0;

public:
    SDL_Texture *spriteSheet = nullptr;
    SDL_Rect spriteRect;

    bool isFlipped;

    // static void SetRenderer(SDL_Renderer *renderer);

    SpriteRenderer(GameObject *gameObject, Vector2 spriteSize, int drawOrder = 0, SDL_Texture *defaultSpriteSheet = nullptr);
    ~SpriteRenderer();
    void Update();
    void Draw();
    Component *Clone(GameObject *parent);

    int GetDrawOrder();
};

class AnimationClip {
private:
    SDL_Texture *spriteSheet;
    SDL_Rect currentSpriteRect;
    std::string name;
    float length = 0;

    // Only used in cloning, by another AnimationClip

public:
    bool loop = false, isPlaying = false;

    Vector2 spriteSize = Vector2(1, 1);

    int currentSprite = 0, startSprite = 0, endSprite = 0;

    float speedScale = 0, animCooldown = 0, lastFrameTime = 0, startTime = 0;

    Event<> *onComplete = nullptr;

    AnimationClip();
    AnimationClip(std::string name, std::string path, Vector2 spriteSize, float length, bool loop, float speedScale, int startSprite, int endSprite);
    AnimationClip(const AnimationClip &clip);

    ~AnimationClip();

    std::string GetName();
    void AdvanceFrame();
    void Ready();
    std::pair<SDL_Texture *, SDL_Rect> GetCurrentSpriteInfo();
};

class Animator : public Component {
private:
    std::map<std::string, AnimationClip> clips;
    AnimationClip *currentClip = nullptr;

public:
    Animator(GameObject *gameObject, std::vector<AnimationClip> clips);
    ~Animator();

    // Update the SpriteRenderer with the current sprite
    void Update();
    void Draw();

    void Play(std::string name);
    void Stop();

    AnimationClip *GetCurrentClip();
    AnimationClip *GetClip(std::string name);
    std::vector<AnimationClip> GetAllClips();

    Component *Clone(GameObject *parent);
};

class Transform {
public:
    float rotation; // Only for the Z axis
    Vector2 position, scale;
    Transform();
    Transform(Vector2 position, float rotation, Vector2 scale);
};

class GameObject {
private:
    std::string name;
    std::vector<Component *> components;

public:
    Transform transform;
    int tag = 0;

    GameObject();
    GameObject(std::string name);
    ~GameObject();
    void Update();
    void Draw();

    std::string GetName();

    Component *AddComponent(Component *component);

    template <typename T>
    T *GetComponent();

    static GameObject *Instantiate(std::string name, const GameObject *origin, Vector2 position, float rotation, Vector2 scale);
    static void Destroy(std::string name);
};

template <typename T>
T *GameObject::GetComponent() {
    for (auto &component : components) {
        if (dynamic_cast<T *>(component)) {
            return dynamic_cast<T *>(component);
        }
    }
    return nullptr;
}

// More like a template for the GameObjectManager
class Scene {
private:
    std::string name;

    std::function<void()> logic;

public:
    Scene(std::string name);
    ~Scene();

    void AssignLogic(std::function<void()> logic);
    void RunLogic();

    void Load();

    std::string GetName();
};

// Wrapper for all, including GameObjectManager
// Singleton
class SceneManager {
private:
    Scene *currentScene;
    SceneManager();
    static SceneManager *instance;

    std::map<std::string, Scene *> scenes;

public:
    ~SceneManager();
    static SceneManager *GetInstance();

    void RunLogic();

    void AddGameObject(GameObject *gameObject);
    void RemoveGameObject(std::string name);
    GameObject *GetGameObject(std::string name);

    void AddScene(Scene *scene);
    void LoadScene(std::string sceneName);
    Scene *GetCurrentScene();

    void Update();
    void Draw();
};

class SoundManager {
private:
    std::map<std::string, Mix_Music *> music;
    std::map<std::string, Mix_Chunk *> sounds;

    std::map<std::string, int> musicVolumes;
    std::map<std::string, int> soundVolumes;

    std::string currentMusic;

    SoundManager();
    static SoundManager *instance;
public:
    ~SoundManager();
    static SoundManager *GetInstance();

    void AddMusic(std::string name, std::string path, int volume);
    void AddSound(std::string name, std::string path, int volume);

    void PlayMusic(std::string name, int loops = -1);
    void PlaySound(std::string name, int loops = 0);

    void StopMusic();
    void StopSound();

    void PauseMusic();
    void PauseSound();

    void ResumeMusic();
    void ResumeSound();

};

class TileMap : public GameObject{
private:
    SDL_Texture* tileMapSheet;
    // an map to define tileType, each type has different tiles, store in a vector
    std::map<std::string, std::vector<Tile>> tileTypes;
    // 2D array of tile, get from tileTypes map, string field for type, int field for position in vector
    std::vector<std::vector<std::pair<std::string, int>>> tileMap;
    // the size of each tile as pixel
    Vector2 tileSize;
    // the size of map as number of tiles
    Vector2 mapSize;
    // camera position
    Vector2 cameraPos;

    static TileMap *instance;

public:
    TileMap();
    ~TileMap();

    TileMap* GetInstance();

    void InitSheet(std::string sheetPath, std::string jsonPath, Vector2 mapSize, Vector2 tileSize);
    void LoadTileMap(std::string mapPath);
    void SetTile(int x, int y, std::string tileType, int tileIndex);
    Tile GetTile(int x, int y);
    void Update();
    void Draw();
};

struct Tile {
    std::pair<int, int> tilePosInSheet;
    // offset and size are in tileCollider
    BoxCollider2D* tileCollider;
    int layer;

    Tile() : tilePosInSheet(-1, -1), tileCollider(nullptr), layer(-1) {}
    Tile(std::pair<int, int> tilePos, BoxCollider2D* tileCollider, int layer) : tilePosInSheet(tilePos), tileCollider(tileCollider), layer(layer) {}
};

#endif
