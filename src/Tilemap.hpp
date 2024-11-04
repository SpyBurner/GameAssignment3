#ifndef TILEMAP_HPP
#define TILEMAP_HPP

#include "Core.hpp"

class Tilemap : public Component {
private:
    SDL_Texture *texture;

    std::vector<std::vector<int>> map;

    std::map<int, bool> noCollisionMap;

    std::map<int, bool> damageMap;
public:


};

#endif