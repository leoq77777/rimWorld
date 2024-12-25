#pragma once
#include <memory>
#include <random>
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
#define FRAMERATE 20
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
          entity_manager_(),
          component_manager_(),
          router_(component_manager_, entity_manager_, MAP_SIZE),
          controller_(entity_manager_, component_manager_),
          create_system_(component_manager_, entity_manager_, router_, TILE_SIZE),
          action_system_(component_manager_, entity_manager_, router_, MAP_SIZE, FRAMERATE),
          task_system_(component_manager_, entity_manager_, router_),
          rng(static_cast<unsigned>(std::time(nullptr))), 
          dist(0, MAP_SIZE - 1) {
        std::cout << "starting a new world" << std::endl;
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
    void generate_random_entity(int count, EntityType type);
    void set_speed(Entity entity, int speed);
    Entities get_all_entities() {return router_.get_all_entities();}
    // Related to UI
    int get_world_width() {return MAP_SIZE;}
    int get_world_height() {return MAP_SIZE;}
    int get_woods_at_loc(Location loc);
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

    // Random seed
    std::mt19937 rng;
    std::uniform_int_distribution<int> dist;
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
            //first select a tree will add a target component
            if (!component_manager_.has_component<TargetComponent>(entity)) {
                component_manager_.add_component(entity,
                 TargetComponent{
                    .progress = 0.0,
                    .timer = 0.0,
                    .is_target = true,
                    .is_finished = false,
                    .to_be_deleted = false,
                    .hold_by = -1
                });
            } 
            auto& target = component_manager_.get_component<TargetComponent>(entity);
            target.is_target = !target.is_target;
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


void World::save_world() {
    entity_manager_.save();
    component_manager_.save();
}

void World::load_world() {
    register_all_components();
    entity_manager_.load();
    component_manager_.load();
}


void World::set_speed(Entity entity, int speed) {
    assert(component_manager_.has_component<MovementComponent>(entity) && "Entity does not have moveComponent");
    component_manager_.get_component<MovementComponent>(entity).speed = speed;
}

int World::get_woods_at_loc(Location loc) {
    int count = 0;
    for(auto& entity : router_.get_entities_with_components<RenderComponent>()) {
        auto& type = component_manager_.get_component<RenderComponent>(entity).entityType;
        if (type == EntityType::WOODPACK) {
            auto& tree_loc = component_manager_.get_component<LocationComponent>(entity).loc;
            auto& resource = component_manager_.get_component<ResourceComponent>(entity);
            if (tree_loc == loc && resource.holder == -1) {
                ++count;
            }
        }
    }
    return count;
}

void World::register_all_components() {
    std::cout << "registering components: \nlocationComponent\ncollisionComponent\nmoveComponent\nResourceComponent\nRenderComponent\nConstructionComponent" << std::endl;
    component_manager_.register_component<LocationComponent>();
    component_manager_.register_component<MovementComponent>();
    component_manager_.register_component<ResourceComponent>();
    component_manager_.register_component<RenderComponent>();
    component_manager_.register_component<ConstructionComponent>();
    component_manager_.register_component<StorageComponent>();
    component_manager_.register_component<TaskComponent>();
    component_manager_.register_component<TargetComponent>();
    component_manager_.register_component<CreateComponent>();
    component_manager_.register_component<ActionComponent>();
}

void World::generate_random_entity(int count, EntityType type) {
    while (count > 0) {
        int x = dist(rng);
        int y = dist(rng);
        Location loc{x, y};
        std::cout << "try create tree at (" << x << ", " << y << ")" << std::endl;
        if (router_.is_valid_position(loc)) {
            Entity temp = entity_manager_.create_entity();
            component_manager_.add_component(temp, LocationComponent{loc});
            component_manager_.add_component(temp, CreateComponent{true, 1, type});
            --count;     
        }
    }
}

void World::init_world() {
    std::cout << "initializing world" << std::endl;
    generate_random_entity( 2, EntityType::CHARACTER );
    generate_random_entity( 1, EntityType::DOG );
    generate_random_entity( 10, EntityType::TREE );
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

