#pragma once
#include "../entities/entity.hpp"
#include "utils/path.hpp"
#include <memory>
#include <vector>
#include <iostream>

#define MAP_SIZE 50
#define MAX_DIST MAP_SIZE*MAP_SIZE
#define DEFAULT_SPEED 32
#define FRAMERATE 60
#define TILE_SIZE 32
class World {
public:  
    World() {
        entity_map_.resize(MAP_SIZE, std::vector<Entities>(MAP_SIZE));
        mark_map_.resize(MAP_SIZE, std::vector<int>(MAP_SIZE, -1));
        register_all_components();
        init_world(); 
    }

    // managers
    EntityManager entity_manager_;
    ComponentManager component_manager_;

    // Entity creations
    Entity create_character(Location pos);
    Entity create_tree(Location pos);
    Entity create_wood(Location pos);
    Entity create_wall(Location pos);
    Entity create_door(Location pos);
    Entity create_dog(Location pos);
    Entity create_storage(Location pos);
    Entity create_blueprint(EntityType entityType, int x, int y);
    
    // Core logic
    void update();

    // Utility maybe moved to other files
    bool is_valid_position(Location pos);
    
    // Functions operated by USER (maybe by UI)
    void mark_tree();
    void make_storage_area(Location start, Location end);
    void set_door_blueprint(Location pos);
    void set_wall_blueprint(Location pos);

    // More methods related to entity
    std::vector<Entity> get_all_entities();
    std::vector<std::vector<Entities>> load_entity_map();
    void load_characters();
    void load_animals();
    void update_entities();
    void generate_random_trees(int count);
    void set_speed(Entity entity, int speed);
    Entities get_entity_at_location(Location loc);
    template<typename... ComponentTypes>
    Entities get_entities_with_components() {
        Entities entities;
        for (auto entity : get_all_entities()) {
            if (component_manager_.has_component<ComponentTypes...>(entity)) {
                entities.push_back(entity);
            }
        }
        return entities;
    }

    // About world save and load
    void save_world();
    void load_world();

private:
    // Starter function run every time
    void register_all_components();
    void init_world();

    // Some containers, managers, systems
    

    std::vector<Entity> characters_;
    std::vector<Entity> animals_;
    std::vector<std::vector<Entities>> entity_map_;
    std::vector<std::vector<int>> mark_map_;
};

// 方法实现
Entity World::create_character(Location loc) {
    Entity entity = entity_manager_.create_entity();
    std::cout << "Entity " << entity << ": character created at (" << loc.x << ", " << loc.y << ")" << std::endl;
    LocationComponent location{loc};
    CollisionComponent collision{true};
    MovementComponent move{
        .start_pos = loc,
        .end_pos = loc,
        .progress = 0.0f,
        .speed = 0
    };
    RenderComponent render{EntityType::CHARACTER, false, sf::Vector2f(loc.x * TILE_SIZE, loc.y * TILE_SIZE)};
    component_manager_.add_component(entity, location);
    component_manager_.add_component(entity, collision);
    component_manager_.add_component(entity, move);
    component_manager_.add_component(entity, render);
    entity_map_[loc.x][loc.y].push_back(entity);
    characters_.push_back(entity);
    return entity;
}

Entity World::create_tree(Location loc) {
    Entity entity = entity_manager_.create_entity();
    std::cout << "Entity " << entity << ": tree created at (" << loc.x << ", " << loc.y << ")" << std::endl;
    LocationComponent location{loc};
    CollisionComponent collision{true}; 
    ResourceComponent resource{1, false};
    RenderComponent render{EntityType::TREE, false, sf::Vector2f(loc.x * TILE_SIZE, loc.y * TILE_SIZE)};
    component_manager_.add_component(entity, location);
    component_manager_.add_component(entity, collision);
    component_manager_.add_component(entity, resource);
    component_manager_.add_component(entity, render);   
    entity_map_[loc.x][loc.y].push_back(entity);
    return entity;
}

Entity World::create_wood(Location loc) {
    Entity entity = entity_manager_.create_entity();
    std::cout << "Entity " << entity << ": wood created" << std::endl;
    LocationComponent location{loc};
    CollisionComponent collision{false}; 
    //one pack has 5 woods without holder
    ResourceComponent resource{5, -1};
    RenderComponent render{EntityType::WOODPACK};
    component_manager_.add_component(entity, location);
    component_manager_.add_component(entity, collision);
    component_manager_.add_component(entity, resource);
    component_manager_.add_component(entity, render);
    entity_map_[loc.x][loc.y].push_back(entity);
    return entity;
}

Entity World::create_wall(Location loc) {
    Entity entity = entity_manager_.create_entity();
    std::cout << "Entity " << entity << ": wall blueprint created" << std::endl;
    LocationComponent location{loc};
    CollisionComponent collision{false};
    ConstructionComponent construction{true, true};
    StorageComponent storage{5, 0, Entities()};
    RenderComponent render{EntityType::WALL};
    component_manager_.add_component(entity, location);
    component_manager_.add_component(entity, collision);
    component_manager_.add_component(entity, construction);
    component_manager_.add_component(entity, storage);
    component_manager_.add_component(entity, render);
    entity_map_[loc.x][loc.y].push_back(entity);
    return entity;
}

Entity World::create_door(Location loc) {
    Entity entity = entity_manager_.create_entity();
    std::cout << "Entity " << entity << ": door blueprint created" << std::endl;
    LocationComponent location{loc};
    ConstructionComponent construction{true, true};
    CollisionComponent collision{false};
    StorageComponent storage{25, 0, Entities()};
    RenderComponent render{EntityType::DOOR};
    //减速实现未完成
    component_manager_.add_component(entity, location);
    component_manager_.add_component(entity, construction);
    component_manager_.add_component(entity, collision);
    component_manager_.add_component(entity, storage);
    component_manager_.add_component(entity, render);
    entity_map_[loc.x][loc.y].push_back(entity);
    return entity;
}

Entity World::create_dog(Location loc) {
    Entity entity = entity_manager_.create_entity();
    std::cout << "Entity " << entity << ": dog created" << std::endl;
    LocationComponent location{loc};
    CollisionComponent collision{true};
    RenderComponent render{EntityType::DOG, false, sf::Vector2f(loc.x * TILE_SIZE, loc.y * TILE_SIZE)};
    MovementComponent move{
        .start_pos = loc,
        .end_pos = loc,
        .progress = 0.0f,
        .speed = 0
    };
    component_manager_.add_component(entity, location);
    component_manager_.add_component(entity, collision);
    component_manager_.add_component(entity, render);
    component_manager_.add_component(entity, move);
    entity_map_[loc.x][loc.y].push_back(entity);
    animals_.push_back(entity);
    return entity;
}

Entity World::create_storage(Location loc) {
    Entity entity = entity_manager_.create_entity();
    std::cout << "Entity " << entity << ": storage created" << std::endl;
    LocationComponent location{loc};
    CollisionComponent collision{false};
    StorageComponent storage{1100, 0, Entities()};
    RenderComponent render{EntityType::STORAGE};
    component_manager_.add_component(entity, location);
    component_manager_.add_component(entity, collision);
    component_manager_.add_component(entity, storage);
    component_manager_.add_component(entity, render);
    entity_map_[loc.x][loc.y].push_back(entity);
    return entity;
}

void World::update() {
    // 实现代码...
}

bool World::is_valid_position(Location loc) {
    Entities entities = entity_map_[loc.x][loc.y];
    for(auto& entity : entities) {
        CollisionComponent collision = component_manager_.get_component<CollisionComponent>(entity);
        if(collision.collidable) {
            return false;
        }
    }
    return true;
}

void World::mark_tree() {
    for(auto& entity : get_all_entities()) {
        auto& render = component_manager_.get_component<RenderComponent>(entity);
        if (render.entityType == EntityType::TREE
            && render.is_selected) {
            component_manager_.add_component(entity, TargetComponent{0.0, 0.0, true, false, false, Entity()});
        }
    }
}

void World::make_storage_area(Location start, Location end) {
    int start_x = std::min(start.x, end.x);
    int start_y = std::min(start.y, end.y);
    int end_x = std::max(start.x, end.x);
    int end_y = std::max(start.y, end.y);
    std::cout << "try create storage area from (" << start_x << ", " << start_y << ") to (" << end_x << ", " << end_y << ")" << std::endl;
    for(int i = start_x; i <= end_x; ++i) {
        for(int j = start_y; j <= end_y; ++j) {
            create_storage(Location{i, j});
        }
    }
}

Entities World::get_all_entities() {
    Entities all_entities;
    for(int i = 0; i < MAX_ENTITIES; ++i) {
        if(entity_manager_.is_entity_alive(i)) {
            all_entities.push_back(i);
        }
    }
    return all_entities;
}

Entities World::get_entity_at_location(Location loc) {
    Entities entities = entity_map_[loc.x][loc.y];
    return entities;
}

std::vector<std::vector<Entities>> World::load_entity_map() {
    std::vector<std::vector<Entities>> entity_map;
    auto entities = get_all_entities();
    for(auto& entity : entities) {
        auto loc = component_manager_.get_component<LocationComponent>(entity).loc;
        entity_map[loc.x][loc.y].push_back(entity);
        auto type = component_manager_.get_component<RenderComponent>(entity).entityType;
        if (type == EntityType::CHARACTER) {
            characters_.push_back(entity);
        }
    }
    return entity_map;
}

void World::load_characters() {
    // 实现代码...
}

void World::load_animals() {
    // 实现代码...
}

void World::update_entities() {
    for(auto& entity : get_all_entities()) {
        auto& track = component_manager_.get_component<TargetComponent>(entity);
        if (track.to_be_deleted) {
            entity_manager_.destroy_entity(entity);
            std::cout << "Entity " << entity << " deleted" << std::endl;
        }
    }
}

void World::generate_random_trees(int count) {
    while (count > 0) {
        int x = rand() % MAP_SIZE;
        int y = rand() % MAP_SIZE;
        Location loc{x, y};
        if (is_valid_position(loc)) {
            create_tree(loc);
            --count;
        }
    }
}

void World::set_speed(Entity entity, int speed) {
    assert(component_manager_.has_component<MovementComponent>(entity) && "Entity does not have moveComponent");
    component_manager_.get_component<MovementComponent>(entity).speed = speed;
}

void World::save_world() {
    // 实现代码...
}

void World::load_world() {
    // 实现代码...
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
}

void World::init_world() {
    std::cout << "initializing world" << std::endl;
    int spawn_x = rand()%MAP_SIZE / 2;
    int spawn_y = rand()%MAP_SIZE / 2;
    std::cout << "try create character at (" << spawn_x << ", " << spawn_y << ")" << std::endl;
    create_character(Location{spawn_x, spawn_y});
    std::cout << "try create dog at (" << spawn_x + 1 << ", " << spawn_y + 1 << ")" << std::endl;
    create_dog(Location{spawn_x + 1, spawn_y + 1});
    std::cout << "try generate 10 trees" << std::endl;
    generate_random_trees(10);
}

void World::set_door_blueprint(Location pos) {
    create_door(pos);
}

void World::set_wall_blueprint(Location pos) {
    create_wall(pos);
}

