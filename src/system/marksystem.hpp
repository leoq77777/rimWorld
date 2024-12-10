#pragma once
#include <SFML/Graphics.hpp>
#include "world/world.h"

class MarkSystem{
    public:
        MarkSystem(World& world);
        void mark(Entity entity, sf::Vector2f start, sf::Vector2f end) {
            for(int i = start.x; i <= end.x; ++i) {
                for(int j = start.y; j <= end.y; ++j) {
                    world.mark_map[i][j] = entity;
                }
            }
        }
    private:
        World& world;
};