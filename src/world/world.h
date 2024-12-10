#ifndef WORLD_HPP
#define WORLD_HPP
#include "../entities/entity.hpp"
#include "system/actionsystem.hpp"
#include "system/taskSystem.hpp"
#include "system/savesystem.hpp"
#include <list>
#define MAP_SIZE 50
#define TILE_SIZE 32
class World {
public:  
    World() {
        characters_.reserve(10);
        entity_map_.resize(MAP_SIZE, std::vector<Entities>(MAP_SIZE));
        mark_map_.resize(MAP_SIZE, std::vector<int>(MAP_SIZE, -1));
        register_all_components();
        init_world();   
    };
    
    Entity create_character(int x, int y);
    Entity create_tree(int x, int y);
    Entity create_wood(int x, int y);
    Entity create_wall(int x, int y);
    Entity create_door(int x, int y);
    Entity create_dog(int x, int y);
    Entity create_storage(int x, int y);
    Entity create_blueprint(EntityType entityType, int x, int y);
    void make_storage_area(sf::Vector2i start, sf::Vector2i end);
    void update();
    bool is_valid_position(int x, int y);
    void generate_random_trees(int count);
    std::vector<Entity> get_active_entities();
    void entity_query(Entity entity, ComponentType componentType);

    EntityManager entity_manager_;
    ComponentManager component_manager_;
    TaskSystem task_system_;
    ActionSystem action_system_;
    SaveSystem save_system_;
    std::vector<Entity> characters_;
    std::vector<std::vector<Entities>> entity_map_;
    std::vector<std::vector<int>> mark_map_;
    void save_game(const std::string& filename = "save.json") {
        SaveSystem::save_world(*this, filename);
    }
    
    void load_game(const std::string& filename = "save.json") {
        SaveSystem::load_world(*this, filename);
    }
private:
    void register_all_components();
    void init_world();
};
#endif