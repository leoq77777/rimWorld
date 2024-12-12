#pragma once

#include <vector>
#include <queue>
#include <unordered_map>
#include <algorithm>
#include <limits.h>
#include "../entities/entity.hpp"

using Dir = std::pair<int, int>;
using Path = std::vector<Dir>;
const std::vector<Dir> directions = {{0, 1}, {0, -1}, {1, 0}, {-1, 0}};

class Router {
    ComponentManager& component_manager_;
    EntityManager& entity_manager_;
    std::vector<std::vector<Entities>>& entity_map_;
    int MAP_SIZE;
public:
    Router();
    
    Router(ComponentManager& component_manager, EntityManager& entity_manager, std::vector<std::vector<Entities>>& entity_map, int map_size) :
        component_manager_(component_manager),
        entity_manager_(entity_manager),
        entity_map_(entity_map),
        MAP_SIZE(map_size) {
            std::cout << "Router initialized" << std::endl;
        }

    bool is_in_locations(const Location& pos, const Locations& target_locations) {
        for(auto& target_location : target_locations) {
            if(pos == target_location) {
                return true;
            }
        }
        return false;
    }

    bool is_in_field(const Location& pos, const Field& field) {
        int minX = std::min(field.first.x, field.second.x);
        int maxX = std::max(field.first.x, field.second.x);
        int minY = std::min(field.first.y, field.second.y);
        int maxY = std::max(field.first.y, field.second.y);
        return pos.x >= minX && pos.x <= maxX && pos.y >= minY && pos.y <= maxY;
    }
    
    bool is_valid_position(const Location& pos) {
        if (pos.x < 0 || pos.x >= MAP_SIZE || pos.y < 0 || pos.y >= MAP_SIZE) {
            return false;
        }

        Entities entities = entity_map_[pos.x][pos.y];
        for(auto& entity : entities) {
            if (component_manager_.has_component<CollisionComponent>(entity)) {
                CollisionComponent collision = component_manager_.get_component<CollisionComponent>(entity);
                if(collision.collidable) {
                    return false;
                }
            }
        }
        //std::cout << "valid position at (" << pos.x << ", " << pos.y << ")" << std::endl;
        return true;
    }

    Locations get_locations_around(const Location& pos) {
        Locations locations;
        for(int i = -1; i <= 1; ++i) {
            for(int j = -1; j <= 1; ++j) {
                if (i == 0 && j == 0 || pos.x + i < 0 || pos.x + i >= MAP_SIZE || pos.y + j < 0 || pos.y + j >= MAP_SIZE) continue;
                locations.push_back({pos.x + i, pos.y + j});
            }
        }
        return locations;
    }

    Path find_path_to_locations(const Location& start, const Locations& end_locations) {
        for (int i = 0; i < end_locations.size(); ++i) {
            auto path = find_path(start, end_locations[i]);
            if (path.size() > 0) {
                std::cout << "path found for one end!" << std::endl;
                return path;
            }
        }
        return {};
    }

    Path find_path(const Location& start, const Location& end) {
        std::cout << "try to find path from (" << start.x << ", " << start.y << ") to (" << end.x << ", " << end.y << ")" << std::endl;
        if (start == end) return {};

        std::queue<Location> queue;
        std::unordered_map<Location, Location, std::hash<Location>> came_from;
        queue.push(start);

        while (!queue.empty()) {
            Location current = queue.front();
            queue.pop();

            for (const auto& direction : directions) {
                Location next = {current.x + direction.first, current.y + direction.second};

                // 检查下一个位置是否在地图范围内并且没有被访问过
                if (is_valid_position(next) && came_from.find(next) == came_from.end()) {
                    queue.push(next);
                    came_from[next] = current;

                    // 如果找到目标位置，构建路径
                    if (next == end) {
                        Path path;
                        for (Location step = end; step != start; step = came_from[step]) {
                            path.push_back({step.x, step.y});
                        }
                        std::reverse(path.begin(), path.end());
                        return path;
                    }
                }
            }
        }
        return {}; // 如果没有找到路径，返回空路径
    }


    Entities get_all_entities() {
        return entity_manager_.get_all_entities();
    }

    template<typename... ComponentTypes>
    Entities get_entities_with_components() {
        Entities entities;
        for (auto entity : entity_manager_.get_all_entities()) {
            if (component_manager_.has_component<ComponentTypes...>(entity)) {
                entities.push_back(entity);
            }
        }
        return entities;
    }

    Entities get_entity_at_location(const Location& pos) {
        return entity_map_[pos.x][pos.y];
    }

    Entities get_characters() {
        Entities characters;
        //std::cout << "try: get characters" << std::endl;
        for(auto& entity : entity_manager_.get_all_entities()) {
            //std::cout << "entity: " << entity << std::endl;
            if (component_manager_.has_component<RenderComponent>(entity) && component_manager_.get_component<RenderComponent>(entity).entityType == EntityType::CHARACTER) {
                //std::cout << "character found" << std::endl;
                characters.emplace_back(entity);
            }
        }
        return characters;
    }

    Entities get_animals() {
        Entities animals;
        //std::cout << "try: get animals" << std::endl;
        for(auto& entity : get_all_entities()) {
            //std::cout << "entity: " << entity << std::endl;
            if (component_manager_.has_component<RenderComponent>(entity) && component_manager_.get_component<RenderComponent>(entity).entityType == EntityType::DOG) {
                //std::cout << "animal found" << std::endl;
                animals.emplace_back(entity);
            }
        }
        return animals;
    }

    Entities get_storage_areas() {
        Entities storage_areas;
        for(auto& entity : get_all_entities()) {
            if (component_manager_.has_component<RenderComponent>(entity) && component_manager_.get_component<RenderComponent>(entity).entityType == EntityType::STORAGE) {
                storage_areas.emplace_back(entity);
            }
        }
        return storage_areas;
    }

    Entities get_placeable_areas() {
        Entities placeable_areas;
        for(auto& entity : entity_manager_.get_all_entities()) {
            if (!component_manager_.has_component<RenderComponent>(entity)) {
                continue;
            }

            auto& type = component_manager_.get_component<RenderComponent>(entity).entityType;
            if ( type == EntityType::STORAGE || 
                ((type == EntityType::WALL || type == EntityType::DOOR) && 
                !component_manager_.get_component<ConstructionComponent>(entity).allocated)) {
                placeable_areas.emplace_back(entity);
            }
        }
        return placeable_areas;
    }

    int calculate_distance(const Location& src, const Location& dst) {
        return std::abs(src.x - dst.x) + std::abs(src.y - dst.y);
    }

    std::pair<std::string, Location> printer(Entity entity) {
        //std::cout << "printer running at entity: " << entity << std::endl;
        auto& type = component_manager_.get_component<RenderComponent>(entity).entityType;
        std::string entity_type;
        if (type == EntityType::CHARACTER) {
            entity_type = "character";
        } else if (type == EntityType::DOG) {
            entity_type = "dog";
        } else if (type == EntityType::STORAGE) {
            entity_type = "storage";
        } else if (type == EntityType::WALL) {
            entity_type = "wall";
        } else if (type == EntityType::DOOR) {
            entity_type = "door";
        } else if (type == EntityType::TREE) {
            entity_type = "tree";
        }
        Location loc = component_manager_.get_component<LocationComponent>(entity).loc;
        return {entity_type, loc};
    }

    void print_all_live_entities() {
        for(auto& entity : get_all_entities()) {
            std::string type = printer(entity).first;
            Location loc = printer(entity).second;
            std::cout << "Entity " << entity << " is alive, type: " << type << std::endl;
        }
    }
};








