#include "world.h"
//#include "system/actionSystem.hpp"
//#include "system/taskSystem.hpp"

World::World() {
    //action_system_ = ActionSystem(*this);
    //task_system_ = TaskSystem(*this);
    entity_map_.resize(MAP_SIZE, std::vector<Entities>(MAP_SIZE));
    mark_map_.resize(MAP_SIZE, std::vector<int>(MAP_SIZE, -1));
    register_all_components();
    init_world(); 
}

void World::register_all_components() {
    std::cout << "registering components: \nlocationComponent\ncollisionComponent\nmoveComponent\nResourceComponent\nRenderComponent\nConstructionComponent" << std::endl;
    component_manager_.register_component<locationComponent>();
    component_manager_.register_component<collisionComponent>();
    component_manager_.register_component<moveComponent>();
    component_manager_.register_component<ResourceComponent>();
    component_manager_.register_component<RenderComponent>();
    component_manager_.register_component<ConstructionComponent>();
    component_manager_.register_component<StorageComponent>();
}

void World::init_world() {
    std::cout << "initializing world" << std::endl;
    int spawn_x = rand()%MAP_SIZE / 2;
    int spawn_y = rand()%MAP_SIZE / 2;
    create_character(Location{spawn_x, spawn_y});
    create_dog(Location{spawn_x + 1, spawn_y + 1});
    generate_random_trees(10);
}

Entity World::create_character(Location loc) {
    Entity entity = entity_manager_.create_entity();
    std::cout << "Entity " << entity << ": character created at (" << loc.x << ", " << loc.y << ")" << std::endl;
    locationComponent location{loc};
    collisionComponent collision{true};
    moveComponent move{0};
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
    locationComponent location{loc};
    collisionComponent collision{true}; 
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
    locationComponent location{loc};
    collisionComponent collision{false}; 
    //one pack has 5 woods
    ResourceComponent resource{5, true};
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
    locationComponent location{loc};
    collisionComponent collision{false};
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
    locationComponent location{loc};
    ConstructionComponent construction{true, true};
    collisionComponent collision{false};
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
    locationComponent location{loc};
    collisionComponent collision{true};
    RenderComponent render{EntityType::DOG, false, sf::Vector2f(loc.x * TILE_SIZE, loc.y * TILE_SIZE)};
    moveComponent move{0}; 
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
    locationComponent location{loc};
    collisionComponent collision{false};
    StorageComponent storage{1100, 0, Entities()};
    RenderComponent render{EntityType::STORAGE};
    component_manager_.add_component(entity, location);
    component_manager_.add_component(entity, collision);
    component_manager_.add_component(entity, storage);
    component_manager_.add_component(entity, render);
    entity_map_[loc.x][loc.y].push_back(entity);
    return entity;
}

bool World::is_valid_position(Location loc) {
    Entities entities = entity_map_[loc.x][loc.y];
    for(auto& entity : entities) {
        collisionComponent collision = component_manager_.get_component<collisionComponent>(entity);
        if(collision.collidable) {
            return false;
        }
    }
    return true;
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

std::vector<Entity> World::get_all_entities() {
    std::vector<Entity> all_entities;
    for(int i = 0; i < MAX_ENTITIES; ++i) {
        if(entity_manager_.is_entity_alive(i)) {
            all_entities.push_back(i);
        }
    }
    return all_entities;
}

std::vector<std::vector<Entities>> World::load_entity_map() {
    std::vector<std::vector<Entities>> entity_map;
    auto entities = get_all_entities();
    for(auto& entity : entities) {
        auto loc = component_manager_.get_component<locationComponent>(entity).loc;
        entity_map[loc.x][loc.y].push_back(entity);
        auto type = component_manager_.get_component<RenderComponent>(entity).entityType;
        if (type == EntityType::CHARACTER) {
            characters_.push_back(entity);
        }
    }
    return entity_map;
}

void World::set_speed(Entity entity, int speed) {
    assert(component_manager_.has_component<moveComponent>(entity) && "Entity does not have moveComponent");
    component_manager_.get_component<moveComponent>(entity).speed = speed;
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

void World::mark_tree() {
    for(auto& entity : get_all_entities()) {
        auto& render = component_manager_.get_component<RenderComponent>(entity);
        if (render.entityType == EntityType::TREE
            && render.is_selected) {
            component_manager_.add_component(entity, TargetComponent{0.0, 0.0, true, false, false, Entity()});
        }
    }
}


void World::make_storage_area(sf::Vector2i start, sf::Vector2i end) {
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
