#include "Core.hpp"
#include "Global.hpp"
#include "Physic2D.hpp"
#include <cmath>
#include <iostream>
#include <algorithm>
#include <list>
#include <json/json.h>
#include <fstream>

#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>


// INIT STATIC
GameObjectManager *GameObjectManager::instance = nullptr;
SceneManager *SceneManager::instance = nullptr;
SoundManager *SoundManager::instance = nullptr;

SDL_Renderer *RENDERER = nullptr;
std::vector<SDL_Texture *> TEXTURES;

//

#pragma region Vector2
// Vector2 class implementation
Vector2::Vector2() : x(0), y(0) {}

Vector2::Vector2(float x, float y) : x(x), y(y) {}

bool Vector2::operator==(Vector2 v) {
    return x == v.x && y == v.y;
}

Vector2 Vector2::operator+(Vector2 v) {
    return Vector2(x + v.x, y + v.y);
}

Vector2 Vector2::operator-(Vector2 v) {
    return Vector2(x - v.x, y - v.y);
}

Vector2 Vector2::operator*(float f) {
    return Vector2(x * f, y * f);
}

Vector2 operator*(float f, Vector2 v) {
    return Vector2(v.x * f, v.y * f);
}

Vector2 Vector2::operator/(float f) {
    return Vector2(x / f, y / f);
}

Vector2 Vector2::operator+=(Vector2 v) {
    x += v.x;
    y += v.y;
    return *this;
}

float Vector2::Magnitude() {
    if (x == 0 && y == 0) {
        return 0;
    }
    return std::sqrt(x * x + y * y);
}

Vector2 Vector2::Normalize() {
    float magnitude = Magnitude();
    if (magnitude == 0) {
        return Vector2(0, 0);
    }
    return Vector2(x / magnitude, y / magnitude);
}

float Vector2::Distance(Vector2 v) {
    return std::sqrt((v.x - x) * (v.x - x) + (v.y - y) * (v.y - y));
}

float Vector2::Distance(Vector2 v1, Vector2 v2) {
    return std::sqrt((v1.x - v2.x) * (v1.x - v2.x) + (v1.y - v2.y) * (v1.y - v2.y));
}

float Vector2::Dot(Vector2 v) {
    return x * v.x + y * v.y;
}

float Vector2::Dot(Vector2 v1, Vector2 v2) {
    return v1.x * v2.x + v1.y * v2.y;
}

float Vector2::Cross(Vector2 v) {
    return x * v.y - y * v.x;
}

float Vector2::Cross(Vector2 v1, Vector2 v2) {
    return v1.x * v2.y - v1.y * v2.x;
}

float Vector2::Angle(Vector2 v1, Vector2 v2) {
    float angle = std::acos(v1.Dot(v2) / (v1.Magnitude() * v2.Magnitude()));
    return angle * (180.0f / M_PI); // Convert radians to degrees
}

float Vector2::SignedAngle(Vector2 v1, Vector2 v2) {
    float angle = std::atan2(v2.y, v2.x) - std::atan2(v1.y, v1.x);
    return angle * (180.0f / M_PI); // Convert radians to degrees
}

#pragma endregion

#pragma region GameObjectManager
// GameObjectManager class implementation
GameObjectManager::GameObjectManager() {}

GameObjectManager::~GameObjectManager() {
    for (auto &pair : gameObjects) {
        delete pair.second;
    }
    gameObjects.clear();
}

GameObjectManager *GameObjectManager::GetInstance() {
    if (instance == nullptr) {
        instance = new GameObjectManager();
    }
    return instance;
}

void GameObjectManager::AddGameObject(std::vector<GameObject *> gameObjects) {
    for (auto &gameObject : gameObjects) {
        this->gameObjects[gameObject->GetName()] = gameObject;
    }
}

void GameObjectManager::AddGameObject(GameObject *gameObject) {
    gameObjects[gameObject->GetName()] = gameObject;
}

void GameObjectManager::RemoveGameObject(std::string name) {
    auto it = gameObjects.find(name);
    if (it != gameObjects.end()) {
        delete it->second;
        gameObjects.erase(it);
    }
}

GameObject *GameObjectManager::GetGameObject(std::string name) {
    auto it = gameObjects.find(name);
    if (it != gameObjects.end()) {
        return it->second;
    }
    return nullptr;
}

void GameObjectManager::Clear() {
    for (auto &pair : gameObjects) {
        delete pair.second;
    }
    gameObjects.clear();
}

void GameObjectManager::Update() {
    for (auto &pair : gameObjects) {
        pair.second->Update();
    }
}

//Draw ordered by SpriteRenderer drawOrder
void GameObjectManager::Draw() {

    std::list<GameObject *> sortedGameObjects;
    for (auto &pair : gameObjects) {
        if (pair.second->GetComponent<SpriteRenderer>() == nullptr) {
            sortedGameObjects.push_front(pair.second);
            continue;
        }
        sortedGameObjects.push_back(pair.second);
    }

    sortedGameObjects.sort([](GameObject *a, GameObject *b) {
        SpriteRenderer *aRenderer = a->GetComponent<SpriteRenderer>();
        SpriteRenderer *bRenderer = b->GetComponent<SpriteRenderer>();

        int sortOrderA = aRenderer ? aRenderer->GetDrawOrder() : 0;
        int sortOrderB = bRenderer ? bRenderer->GetDrawOrder() : 0;

        if (sortOrderA == sortOrderB) {
            return a->transform.position.y < b->transform.position.y;
        }
        
        return sortOrderA < sortOrderB;
    });

    for (auto &gameObject : sortedGameObjects) {
        gameObject->Draw();
    }
}

#pragma endregion

#pragma region GameObject
// GameObject class implementation
Transform::Transform() : position(Vector2(0, 0)), rotation(0), scale(Vector2(1, 1)) {}
Transform::Transform(Vector2 position, float rotation, Vector2 scale) {
    this->position = position;
    this->rotation = rotation;
    this->scale = scale;
}

GameObject::GameObject() {
}

GameObject::GameObject(std::string name) {
    this->name = name;
}

GameObject::~GameObject() {
    for (auto &component : components) {
        delete component;
    }
    components.clear();
}

void GameObject::Update() {
    for (auto &component : components) {
        component->Update();
    }
}

void GameObject::Draw() {
    for (auto &component : components) {
        component->Draw();
    }
}

std::string GameObject::GetName() {
    return name;
}

Component *GameObject::AddComponent(Component *component) {
    components.push_back(component);
    return component;
}

GameObject *GameObject::Instantiate(std::string name, const GameObject *origin, Vector2 position, float rotation, Vector2 scale) {
    GameObject *newObject = new GameObject(name);

    newObject->transform.position = position;
    newObject->transform.rotation = rotation;
    newObject->transform.scale = scale;

    // Deep copy components
    for (auto &component : origin->components) {
        Component *newComponent = component->Clone(newObject);
        newObject->components.push_back(newComponent);
    }

    return newObject;
}

void GameObject::Destroy(std::string name) {
    GameObjectManager::GetInstance()->RemoveGameObject(name);
}
#pragma endregion

#pragma region Component
// Component classes implementation
Component::Component(GameObject *parent) : gameObject(parent) {}

Component::~Component() {}

#pragma region SpriteRenderer
// SpriteRenderer class implementation
// void SpriteRenderer::SetRenderer(SDL_Renderer *renderer){
//     SpriteRenderer::renderer = renderer;
// }

SpriteRenderer::SpriteRenderer(GameObject *gameObject, Vector2 spriteSize, int drawOrder, SDL_Texture *defaultSpriteSheet) : Component(gameObject) {
    this->drawOrder = drawOrder;

    this->spriteSheet = spriteSheet;

    this->spriteRect = SDL_Rect();
    this->spriteRect.x = 0;
    this->spriteRect.y = 0;
    this->spriteRect.w = (int)spriteSize.x;
    this->spriteRect.h = (int)spriteSize.y;

    if (defaultSpriteSheet) {
        this->spriteSheet = defaultSpriteSheet;
    }
}

SpriteRenderer::~SpriteRenderer() {}

void SpriteRenderer::Update() {}

void SpriteRenderer::Draw() {
    if (!RENDERER) {
        throw "Renderer is null in SpriteRenderer::Draw()";
        return;
    }
    if (!spriteSheet) {
        throw "SpriteSheet is null in SpriteRenderer::Draw()";
        return;
    }
    SDL_Rect destRect;
    // Moving the rect to center the sprite

    Transform *transform = &(gameObject->transform);

    destRect.x = transform->position.x - spriteRect.w * transform->scale.x / 2;
    destRect.y = transform->position.y - spriteRect.h * transform->scale.y / 2;
    destRect.w = spriteRect.w * transform->scale.x;
    destRect.h = spriteRect.h * transform->scale.y;

    // Copy the sprite to the renderer
    // SDL_RenderCopy(renderer, spriteSheet, &spriteRect, &destRect);
    SDL_RenderCopyEx(RENDERER, spriteSheet, &spriteRect, &destRect, transform->rotation, nullptr, SDL_FLIP_NONE);
}

Component *SpriteRenderer::Clone(GameObject *parent) {
    SpriteRenderer *newRenderer = new SpriteRenderer(parent, Vector2(spriteRect.w, spriteRect.h), drawOrder, spriteSheet);

    return newRenderer;
}

SDL_Texture *LoadSpriteSheet(std::string path) {
    SDL_Surface *surface = IMG_Load(path.c_str());
    if (!surface) {
        std::cerr << "Failed to load image: " << path << std::endl;
        return nullptr;
    }

    SDL_Texture *texture = SDL_CreateTextureFromSurface(RENDERER, surface);
    std::string error = SDL_GetError();
    SDL_FreeSurface(surface);

    TEXTURES.push_back(texture);

    return texture;
}

int SpriteRenderer::GetDrawOrder() {
    return drawOrder;
}

#pragma endregion

#pragma region Animator
// AnimationClip class implementation

AnimationClip::AnimationClip() {}

AnimationClip::AnimationClip(std::string name, std::string path, Vector2 spriteSize,
                             float length, bool loop, float speedScale, int startSprite, int endSprite) {
    // Stats setup
    this->name = name;
    this->spriteSize = spriteSize;
    this->length = length;
    this->loop = loop;
    this->speedScale = speedScale;

    this->startSprite = startSprite;
    this->endSprite = endSprite;

    currentSprite = startSprite;
    isPlaying = false; // Initialize isPlaying to false

    // Sprite setup
    spriteSheet = LoadSpriteSheet(path);

    currentSpriteRect.x = currentSprite * (int)spriteSize.x;
    currentSpriteRect.y = 0; // Assuming the sprites are in a single row
    currentSpriteRect.w = (int)spriteSize.x;
    currentSpriteRect.h = (int)spriteSize.y;

    animCooldown = length / (endSprite - startSprite); // Calculate animation cooldown based on length and number of frames

    onComplete = new Event<>();
}

AnimationClip::AnimationClip(const AnimationClip &clip) {
    name = clip.name;
    spriteSize = clip.spriteSize;
    length = clip.length;
    loop = clip.loop;
    speedScale = clip.speedScale;

    startSprite = clip.startSprite;
    endSprite = clip.endSprite;

    currentSprite = clip.startSprite;
    isPlaying = false; // Initialize isPlaying to false

    // Sprite setup
    spriteSheet = clip.spriteSheet;

    currentSpriteRect.x = currentSprite * (int)spriteSize.x;
    currentSpriteRect.y = 0; // Assuming the sprites are in a single row
    currentSpriteRect.w = (int)spriteSize.x;
    currentSpriteRect.h = (int)spriteSize.y;

    animCooldown = clip.animCooldown; // Calculate animation cooldown based on length and number of frames

    onComplete = new Event<>();
}

AnimationClip::~AnimationClip() {
    // if (onComplete != nullptr)
    //     delete onComplete;
}

std::string AnimationClip::GetName() {
    return name;
}

void AnimationClip::AdvanceFrame() {
    if (!isPlaying)
        return;

    float currentTime = SDL_GetTicks();
    if (currentTime - lastFrameTime <= animCooldown / speedScale)
        return;

    lastFrameTime = currentTime;
    currentSprite++;

    if (currentSprite > endSprite) {
        if (loop) {
            currentSprite = startSprite;
        } else {
            currentSprite = endSprite;
            isPlaying = false;
            if (onComplete != nullptr) {
                onComplete->raise();
            }
        }
    }

    currentSpriteRect.x = currentSprite * spriteSize.x;
    currentSpriteRect.y = 0;
}

void AnimationClip::Ready() {
    currentSprite = startSprite;
    isPlaying = true;
    startTime = SDL_GetTicks();
    lastFrameTime = SDL_GetTicks() - animCooldown * speedScale;
}

std::pair<SDL_Texture *, SDL_Rect> AnimationClip::GetCurrentSpriteInfo() {
    return {spriteSheet, currentSpriteRect};
}

// Animator class implementation
Animator::Animator(GameObject *gameObject, std::vector<AnimationClip> clips) : Component(gameObject) {
    for (auto &clip : clips) {
        this->clips.insert({clip.GetName(), clip});
    }
    currentClip = &(this->clips.begin()->second);

    if (gameObject->GetComponent<SpriteRenderer>()) {
        std::pair<SDL_Texture *, SDL_Rect> spriteInfo = currentClip->GetCurrentSpriteInfo();
        gameObject->GetComponent<SpriteRenderer>()->spriteSheet = spriteInfo.first;
        gameObject->GetComponent<SpriteRenderer>()->spriteRect = spriteInfo.second;
    }

    currentClip->Ready();
}

Animator::~Animator() {
    clips.clear();
}

void Animator::Update() {
    // Advance the current clip's frame
    if (currentClip)
        currentClip->AdvanceFrame();

    // Update the SpriteRenderer with the current sprite
    std::pair<SDL_Texture *, SDL_Rect> sheetInfo = currentClip->GetCurrentSpriteInfo();
    SpriteRenderer *renderer = gameObject->GetComponent<SpriteRenderer>();
    if (renderer) {
        renderer->spriteSheet = sheetInfo.first;
        renderer->spriteRect = sheetInfo.second;
    }
}

void Animator::Draw() {}

void Animator::Play(std::string name) {
    AnimationClip *clip = GetClip(name);
    if (clip) {
        currentClip = clip;
        currentClip->Ready();
    }
}

void Animator::Stop() {
    currentClip->isPlaying = false;
}

AnimationClip *Animator::GetCurrentClip() {
    return currentClip;
}

AnimationClip *Animator::GetClip(std::string name) {
    auto it = clips.find(name);
    if (it != clips.end()) {
        return &(it->second);
    }
    return nullptr;
}

std::vector<AnimationClip> Animator::GetAllClips() {
    std::vector<AnimationClip> clipList;
    for (auto &pair : clips) {
        clipList.push_back(AnimationClip(pair.second));
    }
    return clipList;
}

Component *Animator::Clone(GameObject *parent) {
    Animator *newAnimator = new Animator(parent, GetAllClips());
    return newAnimator;
}

#pragma endregion

#pragma endregion

#pragma region SceneManager
// Scene class implementation
Scene::Scene(std::string name) {
    this->name = name;
}

Scene::~Scene() {
}

void Scene::AssignLogic(std::function<void()> logic) {
    this->logic = logic;
}

void Scene::RunLogic() {
    if (logic) {
        logic();
    }
}

void Scene::Load() {
    // Clear all objects
    GameObjectManager::GetInstance()->Clear();
    CollisionManager::GetInstance()->Clear();

    RunLogic();
}

std::string Scene::GetName() {
    return name;
}

// SceneManager class implementation
SceneManager::SceneManager() {}
SceneManager::~SceneManager() {
    for (auto &pair : scenes) {
        delete pair.second;
    }
    scenes.clear();
    delete CollisionManager::GetInstance();
    delete GameObjectManager::GetInstance();
}

SceneManager *SceneManager::GetInstance() {
    if (instance == nullptr) {
        instance = new SceneManager();
    }
    return instance;
}

void SceneManager::RunLogic() {
    if (currentScene) {
        currentScene->RunLogic();
    }
}

void SceneManager::AddGameObject(GameObject *gameObject) {
    GameObjectManager::GetInstance()->AddGameObject(gameObject);
}

void SceneManager::RemoveGameObject(std::string name) {
    GameObjectManager::GetInstance()->RemoveGameObject(name);
}

GameObject *SceneManager::GetGameObject(std::string name) {
    return GameObjectManager::GetInstance()->GetGameObject(name);
}

void SceneManager::AddScene(Scene *scene) {
    scenes[scene->GetName()] = scene;
}

void SceneManager::LoadScene(std::string name) {
    auto it = scenes.find(name);
    if (it != scenes.end()) {
        currentScene = it->second;
        currentScene->Load();
    }
}

Scene *SceneManager::GetCurrentScene() {
    return currentScene;
}

void SceneManager::Update() {
    CollisionManager::GetInstance()->Update();
    GameObjectManager::GetInstance()->Update();
}

void SceneManager::Draw() {
    GameObjectManager::GetInstance()->Draw();
}

#pragma endregion

#pragma region SoundManager

// SoundManager class implementation
SoundManager::SoundManager() {
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024) < 0) {
        std::cerr << "SDL_mixer could not initialize! SDL_mixer Error: " << Mix_GetError() << std::endl;
    }
}

SoundManager::~SoundManager() {
    for (auto &pair : music) {
        Mix_FreeMusic(pair.second);
    }
    music.clear();

    for (auto &pair : sounds) {
        Mix_FreeChunk(pair.second);
    }
    sounds.clear();

    Mix_Quit();

    instance = nullptr;
}

SoundManager* SoundManager::GetInstance() {
    if (instance == nullptr) {
        instance = new SoundManager();
    }
    return instance;
}

void SoundManager::AddMusic(std::string name, std::string path, int volume = 128) {
    Mix_Music *newMusic = Mix_LoadMUS(path.c_str());
    if (!newMusic) {
        std::cerr << "Failed to load music: " << path << " SDL_mixer Error: " << Mix_GetError() << std::endl;
        return;
    }
    music[name] = newMusic;
    musicVolumes[name] = volume;
}

void SoundManager::AddSound(std::string name, std::string path, int volume = 128) {
    Mix_Chunk *newSound = Mix_LoadWAV(path.c_str());
    if (!newSound) {
        std::cerr << "Failed to load sound: " << path << " SDL_mixer Error: " << Mix_GetError() << std::endl;
        return;
    }
    sounds[name] = newSound;
    soundVolumes[name] = volume;
}

void SoundManager::PlayMusic(std::string name, int loops) {
    auto it = music.find(name);
    if (it != music.end()) {
        if (currentMusic == name) {
            return;
        }
        
        currentMusic = name;
        Mix_VolumeMusic(musicVolumes[name]);
        Mix_PlayMusic(it->second, loops);
    } else {
        std::cerr << "Music not found: " << name << std::endl;
    }
}

void SoundManager::PlaySound(std::string name, int loops) {
    auto it = sounds.find(name);
    if (it != sounds.end()) {

        Mix_Volume(-1, soundVolumes[name]);
        Mix_PlayChannel(-1, it->second, loops);
        
    } else {
        std::cerr << "Sound not found: " << name << std::endl;
    }
}

void SoundManager::StopMusic() {
    Mix_HaltMusic();
}

void SoundManager::StopSound() {
    Mix_HaltChannel(-1);
}

void SoundManager::PauseMusic() {
    Mix_PauseMusic();
}

void SoundManager::PauseSound() {
    Mix_Pause(-1);
}

void SoundManager::ResumeMusic() {
    Mix_ResumeMusic();
}

void SoundManager::ResumeSound() {
    Mix_Resume(-1);
}

#pragma endregion

#pragma region TileMap

TileMap::TileMap() : tileMapSheet(nullptr), mapSize(0, 0), tileSize(0, 0), cameraPos(0, 0) {}

TileMap::~TileMap() {}

TileMap* TileMap::GetInstance() {
    if (instance == nullptr) {
        instance = new TileMap();
    }
    return instance;
}

void TileMap::InitSheet(std::string sheetPath, std::string jsonPath, Vector2 mapSize, Vector2 tileSize) {
    tileMapSheet = LoadSpriteSheet(sheetPath);
    this->mapSize = mapSize;
    this->tileSize = tileSize;

    // Load and parse the JSON file using JsonCpp
    std::ifstream jsonFile(jsonPath, std::ifstream::binary);
    Json::Value root;
    jsonFile >> root;

    // Initialize tileTypes from JSON data
    for (const auto& tileData : root["tiles"]) {
        std::string type = tileData["type"].asString();
        int posX = tileData["posX"].asInt();
        int posY = tileData["posY"].asInt();
        int layer = tileData["layer"].asInt();
        bool hasCollider = tileData.isMember("collider") ? tileData["collider"].asBool() : false;

        BoxCollider2D* collider = hasCollider ? new BoxCollider2D(this, Vector2(posX, posY), tileSize) : nullptr;
        tileTypes[type].push_back(Tile({posX, posY}, collider, layer));
    }
}

void TileMap::LoadTileMap(std::string mapPath) {
    // Load and parse the JSON file using JsonCpp
    std::ifstream jsonFile(mapPath, std::ifstream::binary);
    Json::Value root;
    jsonFile >> root;

    // Initialize tileMap from JSON data
    const Json::Value& jsonTileMap = root["tileMap"];
    tileMap.resize(jsonTileMap.size());

    for (Json::Value::ArrayIndex i = 0; i < jsonTileMap.size(); ++i) {
        const Json::Value& row = jsonTileMap[i];
        tileMap[i].resize(row.size());
        for (Json::Value::ArrayIndex j = 0; j < row.size(); ++j) {
            tileMap[i][j].first = row[j]["type"].asString();
            tileMap[i][j].second = row[j]["index"].asInt();
        }
    }
}

void TileMap::SetTile(int x, int y, const std::string tileType, int tileIndex) {
    if (x < 0 || x >= mapSize.x || y < 0 || y >= mapSize.y) return;

    if (tileTypes.find(tileType) != tileTypes.end()) {
        tileMap[y][x] = { tileType, tileIndex };
    }
}

Tile TileMap::GetTile(int x, int y) {
    if (x < 0 || x >= mapSize.x || y < 0 || y >= mapSize.y) return {};

    return tileTypes[tileMap[y][x].first].at(tileMap[y][x].second);
}

void TileMap::Update() {}

void TileMap::Draw() {
    if (!tileMapSheet) return;

    int startX = static_cast<int>(cameraPos.x / tileSize.x);
    int startY = static_cast<int>(cameraPos.y / tileSize.y);
    int endX = static_cast<int>((cameraPos.x + WIDTH) / tileSize.x);
    int endY = static_cast<int>((cameraPos.y + HEIGHT) / tileSize.y);

    for (int y = startY; y <= endY && y < mapSize.y; ++y) {
        for (int x = startX; x <= endX && x < mapSize.x; ++x) {
            const auto& tileInfo = tileMap[y][x];
            if (tileTypes.find(tileInfo.first) == tileTypes.end()) continue;

            Tile& tile = tileTypes[tileInfo.first].at(tileInfo.second);
            SDL_Rect srcRect = {
                tile.tilePosInSheet.first * static_cast<int>(tileSize.x),
                tile.tilePosInSheet.second * static_cast<int>(tileSize.y),
                static_cast<int>(tileSize.x),
                static_cast<int>(tileSize.y)
            };
            SDL_Rect dstRect = {
                static_cast<int>((x * tileSize.x) - cameraPos.x),
                static_cast<int>((y * tileSize.y) - cameraPos.y),
                static_cast<int>(tileSize.x),
                static_cast<int>(tileSize.y)
            };
            SDL_RenderCopy(RENDERER, tileMapSheet, &srcRect, &dstRect);
        }
    }
}

#pragma endregion