#ifndef TILEMAP_HPP
#define TILEMAP_HPP

#include "Core.hpp"

class Tilemap : public Component {
private:
    SDL_Texture *texture;

    std::vector<std::vector<int>> map;

    std::map<int, bool> noCollisionMap;

    std::map<int, bool> damageMap;

    Vector2 tileSize;
    Vector2 tilesetDimension;
    bool moving = false;
    float speed;

    std::string mapPath;
    void LoadMap();
    void BuildMap();

    std::vector<GameObject *> tiles;

public:

    Tilemap(GameObject *gameObject, Vector2 tileSize, Vector2 tilesetDimension, bool moving, float speed, SDL_Texture *texture, std::string mapPath);
    ~Tilemap();

    std::vector<Vector2> AStar(Vector2 start, Vector2 end);

    Vector2 GetPositionFromTile(int x, int y);
    Vector2 GetTileFromPosition(Vector2 position);

    void Update();
    void Draw();

    Component *Clone(GameObject *parent);

};

#endif