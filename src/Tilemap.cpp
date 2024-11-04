#include "Tilemap.hpp"
#include "Components.hpp"
#include "Physic2D.hpp"
#include <fstream>
#include <sstream>

// #define DEBUG_LOAD 1
#define DEBUG_BUILD 1

Tilemap::Tilemap(GameObject *gameObject, Vector2 tileSize, Vector2 tilesetDimension, bool moving, float speed, SDL_Texture *texture, std::string mapPath)
    : Component(gameObject) {
    this->tileSize = tileSize;
    this->tilesetDimension = tilesetDimension;

    this->moving = moving;
    this->speed = speed;
    this->texture = texture;

    this->mapPath = mapPath;
    LoadMap();
    BuildMap();
}

Tilemap::~Tilemap() {
    // Clean up if necessary
}

void Tilemap::LoadMap() {
    if (mapPath.empty())
        return;

#ifdef DEBUG_LOAD
    std::cout << "Loading map from " << mapPath << std::endl;
#endif

    std::ifstream file(mapPath);

    int n, m;
    int noCollCount;
    int damageCount;

    file >> m >> n;

#ifdef DEBUG_LOAD
    std::cout << "Map size: " << n << " " << m << std::endl;
#endif

    for (int i = 0; i < n; i++) {
        std::vector<int> row;
        for (int j = 0; j < m; j++) {
            int tile;
            file >> tile;
            row.push_back(tile);

#ifdef DEBUG_LOAD
            std::cout << tile << " ";
#endif
        }
#ifdef DEBUG_LOAD
        std::cout << std::endl;
#endif
        map.push_back(row);
    }

    file >> noCollCount;
#ifdef DEBUG_LOAD
    std::cout << "No collision count: " << noCollCount << std::endl;
#endif
    for (int i = 0; i < noCollCount; i++) {
        int tile;
        file >> tile;
        noCollisionMap[tile] = true;

#ifdef DEBUG_LOAD
        std::cout << "No collision tile: " << tile << std::endl;
#endif
    }

    file >> damageCount;
#ifdef DEBUG_LOAD
    std::cout << "Damage count: " << damageCount << std::endl;
#endif
    for (int i = 0; i < damageCount; i++) {
        int tile;
        file >> tile;
        damageMap[tile] = true;
#ifdef DEBUG_LOAD
        std::cout << "Damage tile: " << tile << std::endl;
#endif
    }

    file.close();
}

void Tilemap::BuildMap() {
    if (map.empty())
        return;

    for (int i = 0; i < map.size(); i++) {
        for (int j = 0; j < map[i].size(); j++) {
            if (map[i][j] == 00)
                continue;

#ifdef DEBUG_BUILD
            std::cout << "Building tile " << i << " " << j << std::endl;
            std::cout << "Tile: " << map[i][j] << std::endl;
#endif

            bool haveCollision = noCollisionMap.find(map[i][j]) == noCollisionMap.end();
            bool haveDamage = damageMap.find(map[i][j]) != damageMap.end();

            // Create a GameObject for each tile
            GameObject *tile = new GameObject("Tile" + std::to_string(i) + "_" + std::to_string(j));
            this->tiles.push_back(tile);

            if (haveDamage) {
#ifdef DEBUG_BUILD
                std::cout << "Tile has damage" << std::endl;
#endif
                tile->layer = CollisionMatrix::SPIKE;
            } else {
#ifdef DEBUG_BUILD
                std::cout << "Tile has no damage" << std::endl;
#endif
                tile->layer = CollisionMatrix::WALL;
            }

            tile->transform.position = Vector2(j * tileSize.x * gameObject->transform.scale.x, i * tileSize.y * gameObject->transform.scale.y);
            tile->transform.scale = gameObject->transform.scale;

            // Add a SpriteRenderer component to the tile
            tile->AddComponent(new SpriteRenderer(tile, tileSize, 0, texture));

            // Modulo to get the tile from the sprite sheet
            SDL_Rect spriteRect;
            spriteRect.x = (map[i][j] * (int)tileSize.x) % (int)tilesetDimension.x;
            spriteRect.y = ((map[i][j] * (int)tileSize.x) / (int)tilesetDimension.x) * (int)tileSize.y;
            spriteRect.w = tileSize.x;
            spriteRect.h = tileSize.y;
#ifdef DEBUG_BUILD
            std::cout << "SpriteRect: " << spriteRect.x << " " << spriteRect.y << std::endl;
#endif

            tile->GetComponent<SpriteRenderer>()->spriteRect = spriteRect;

            // Add a BoxCollider2D component to the tile
            tile->AddComponent(new BoxCollider2D(tile, Vector2(0, 0),
                Vector2(tileSize.x * tile->transform.scale.x, tileSize.y * tile->transform.scale.y),
            !haveCollision));

            // Add a DamageOnCollision component to the tile
            if (haveDamage){
                tile->AddComponent(new DamageOnCollision(tile, TILE_DAMAGE, CollisionMatrix::PLAYER, 0));
                tile->AddComponent(new DamageOnCollision(tile, TILE_DAMAGE, CollisionMatrix::ENEMY, 0));
            }

            // Add the tile to the GameObjectManager
            GameObjectManager::GetInstance()->AddGameObject(tile);
        }
    }
}

std::vector<Vector2> Tilemap::AStar(Vector2 start, Vector2 end) {
    // Implement A* pathfinding algorithm
    return std::vector<Vector2>();
}

Vector2 Tilemap::GetPositionFromTile(int x, int y) {

    Vector2 position = Vector2(x * tileSize.x * gameObject->transform.scale.x,
        y * tileSize.y * gameObject->transform.scale.y);

    return position;
}

Vector2 Tilemap::GetTileFromPosition(Vector2 position) {
    return Vector2(position.x / (tileSize.x * gameObject->transform.scale.x), position.y / (tileSize.y * gameObject->transform.scale.y));
}

void Tilemap::Update() {
    if (!Game::CAMERA || !moving)
        return;

    Rigidbody2D *rb = Game::CAMERA->GetComponent<Rigidbody2D>();
    if (!rb)
        return;

    this->gameObject->transform.position -= rb->velocity.Normalize() * speed;

    for (auto &tile : tiles) {
        tile->transform.position += rb->velocity.Normalize() * speed;
    }
}

void Tilemap::Draw() {}

Component *Tilemap::Clone(GameObject *parent) {
    Tilemap *newTilemap = new Tilemap(parent, tileSize, tilesetDimension, moving, speed, texture, mapPath);

    return newTilemap;
}