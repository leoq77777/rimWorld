#pragma once
#include <memory>
#include "../entities/entity.hpp"
#include "../utils/path.hpp"
#include <SFML/Graphics.hpp>
#include "../system/actionsystem.hpp"
#include "../system/taskSystem.hpp"
#include "../system/createsystem.hpp"
#include "../utils/controller.hpp"

#define MAP_SIZE 50
#define MAX_DIST MAP_SIZE*MAP_SIZE
#define DEFAULT_SPEED 32
#define FRAMERATE 60
#define TILE_SIZE 32
class World {
    friend class UI;
public:
    //systems
    CreateSystem create_system_;
    ActionSystem action_system_;
    TaskSystem task_system_;

    // managers
    EntityManager entity_manager_;
    ComponentManager component_manager_;

    //helper
    Router router_;
    Controller controller_;

    World() :
          entity_map_(std::vector<std::vector<Entities>>(MAP_SIZE, std::vector<Entities>(MAP_SIZE))),
          mark_map_(std::vector<std::vector<int>>(MAP_SIZE, std::vector<int>(MAP_SIZE, -1))),
          entity_manager_(),
          component_manager_(),
          router_(component_manager_, entity_manager_, entity_map_, MAP_SIZE),
          controller_(entity_manager_, component_manager_),
          create_system_(component_manager_, entity_manager_, router_, entity_map_, TILE_SIZE),
          action_system_(component_manager_, entity_manager_, router_, MAP_SIZE, FRAMERATE),
          task_system_(component_manager_, entity_manager_, router_, entity_map_) {
        register_all_components();
        init_world();
    }

    void update_world() {
        create_system_.update();
        task_system_.update();
        action_system_.update();
    }
    
    // Functions operated by USER (maybe by UI)
    bool mark_tree();
    bool make_storage_area(Location start, Location end);
    bool set_door_blueprint(Location pos);
    bool set_wall_blueprint(Location pos);

    // More methods related to entity
    std::vector<std::vector<Entities>> load_entity_map();
    void load_characters();
    void load_animals();
    void generate_random_trees(int count);
    void set_speed(Entity entity, int speed);

    // Related to UI
    int get_world_width() {return MAP_SIZE;}
    int get_world_height() {return MAP_SIZE;}

    // About world save and load
    void save_world();
    void load_world();

private:
    // Starter function run every time
    void register_all_components();
    void init_world();

    // Some containers
    std::vector<Entity> characters_;
    std::vector<Entity> animals_;
    std::vector<std::vector<Entities>> entity_map_;
    std::vector<std::vector<int>> mark_map_;
};

bool World::mark_tree() {
    bool marked = false;
    for(auto& entity : router_.get_all_entities()) {
        if (!component_manager_.has_component<RenderComponent>(entity)) {
            continue;
        }
        auto& render = component_manager_.get_component<RenderComponent>(entity);
        if (render.entityType == EntityType::TREE
            && render.is_selected) {
            if (!component_manager_.has_component<TargetComponent>(entity)) {
                component_manager_.add_component(entity, TargetComponent{0.0, 0.0, true, false, false, Entity()});
            } 
            auto& target = component_manager_.get_component<TargetComponent>(entity);
            target.is_target = true;
            marked = true;
        }
    }
    return marked;
}

bool World::make_storage_area(Location start, Location end) {
    int start_x = std::min(start.x, end.x);
    int start_y = std::min(start.y, end.y);
    int end_x = std::max(start.x, end.x);
    int end_y = std::max(start.y, end.y);
    std::cout << "try create storage area from (" << start_x << ", " << start_y << ") to (" << end_x << ", " << end_y << ")" << std::endl;
    for(int i = start_x; i <= end_x; ++i) {
        for(int j = start_y; j <= end_y; ++j) {
            if (!router_.is_valid_position(Location{i, j})) {
                std::cout << "invalid storage area" << std::endl;
                return false;
            }
        }
    }

    for(int i = start_x; i <= end_x; ++i) {
        for(int j = start_y; j <= end_y; ++j) {
            Entity temp = entity_manager_.create_entity();
            component_manager_.add_component(temp, LocationComponent{Location{i, j}});
            component_manager_.add_component(temp, CreateComponent{true, 1, EntityType::STORAGE});
        }
    }
    return true;
}

std::vector<std::vector<Entities>> World::load_entity_map() {
    std::vector<std::vector<Entities>> entity_map;
    auto entities = router_.get_all_entities();
    for(auto& entity : entities) {
        auto loc = component_manager_.get_component<LocationComponent>(entity).loc;
        entity_map[loc.x][loc.y].push_back(entity);
        if (!component_manager_.has_component<RenderComponent>(entity)) {
            continue;
        }
        auto type = component_manager_.get_component<RenderComponent>(entity).entityType;
        if (type == EntityType::CHARACTER) {
            characters_.push_back(entity);
        }
    }
    return entity_map;
}

/*save and load
void World::load_characters() {
    // 实现代码...
}

void World::load_animals() {
    // 实现代码...
}
void World::save_world() {
    // 实现代码...
}

void World::load_world() {
    // 实现代码...
}
*/



void World::generate_random_trees(int count) {
    while (count > 0) {
        int x = rand() % MAP_SIZE;
        int y = rand() % MAP_SIZE;
        Location loc{x, y};
        std::cout << "try create tree at (" << x << ", " << y << ")" << std::endl;
        if (router_.is_valid_position(loc)) {
            Entity temp = entity_manager_.create_entity();
            component_manager_.add_component(temp, LocationComponent{loc});
            component_manager_.add_component(temp, CreateComponent{true, 1, EntityType::TREE});
            --count;
        }
    }
}

void World::set_speed(Entity entity, int speed) {
    assert(component_manager_.has_component<MovementComponent>(entity) && "Entity does not have moveComponent");
    component_manager_.get_component<MovementComponent>(entity).speed = speed;
}

void World::register_all_components() {
    std::cout << "registering components: \nlocationComponent\ncollisionComponent\nmoveComponent\nResourceComponent\nRenderComponent\nConstructionComponent" << std::endl;
    component_manager_.register_component<LocationComponent>();
    component_manager_.register_component<CollisionComponent>();
    component_manager_.register_component<MovementComponent>();
    component_manager_.register_component<ResourceComponent>();
    component_manager_.register_component<RenderComponent>();
    component_manager_.register_component<ConstructionComponent>();
    component_manager_.register_component<StorageComponent>();
    component_manager_.register_component<TaskComponent>();
    component_manager_.register_component<TargetComponent>();
    component_manager_.register_component<CreateComponent>();
}

void World::init_world() {
    std::cout << "initializing world" << std::endl;
    int spawn_x = rand()%MAP_SIZE / 2;
    int spawn_y = rand()%MAP_SIZE / 2;
    std::cout << "try create character at (" << spawn_x << ", " << spawn_y << ")" << std::endl;
    Entity temp_character = entity_manager_.create_entity();
    component_manager_.add_component(temp_character, LocationComponent{Location{spawn_x, spawn_y}});
    component_manager_.add_component(temp_character, CreateComponent{true, 1, EntityType::CHARACTER});
    
    std::cout << "try create dog at (" << spawn_x + 1 << ", " << spawn_y + 1 << ")" << std::endl;
    Entity temp_dog = entity_manager_.create_entity();
    component_manager_.add_component(temp_dog, LocationComponent{Location{spawn_x + 1, spawn_y + 1}});
    component_manager_.add_component(temp_dog, CreateComponent{true, 1, EntityType::DOG});
    
    std::cout << "try generate 10 trees" << std::endl;
    generate_random_trees(10);

    create_system_.update();
}

bool World::set_door_blueprint(Location pos) {
    if (!router_.is_valid_position(pos)) {
        std::cout << "invalid position to create door" << std::endl;
        return false;
    }
    Entity temp = entity_manager_.create_entity();
    component_manager_.add_component(temp, LocationComponent{pos});
    component_manager_.add_component(temp, CreateComponent{true, 1, EntityType::DOOR});
    return true;
}

bool World::set_wall_blueprint(Location pos) {
    if (!router_.is_valid_position(pos)) {
        std::cout << "invalid position to create wall" << std::endl;
        return false;
    }
    Entity temp = entity_manager_.create_entity();
    component_manager_.add_component(temp, LocationComponent{pos});
    component_manager_.add_component(temp, CreateComponent{true, 1, EntityType::WALL});
    return true;
}

