#include "world.h"

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
    create_character(spawn_x, spawn_y);
    create_dog(spawn_x + 1, spawn_y + 1);
    generate_random_trees(10);
}

Entity World::create_character(int x, int y) {
    Entity entity = entity_manager_.create_entity();
    std::cout << "Entity " << entity << ": character created at (" << x << ", " << y << ")" << std::endl;
    locationComponent loc{sf::Vector2i(x, y)};
    collisionComponent col{true};
    moveComponent move{0.0f, 0.0f};
    RenderComponent render{EntityType::CHARACTER, false, sf::Vector2f(x * TILE_SIZE, y * TILE_SIZE)};
    component_manager_.add_component(entity, loc);
    component_manager_.add_component(entity, col);
    component_manager_.add_component(entity, move);
    component_manager_.add_component(entity, render);
    entity_map_[x][y].push_back(entity);
    characters_.push_back(entity);
    return entity;
}

Entity World::create_tree(int x, int y) {
    Entity entity = entity_manager_.create_entity();
    std::cout << "Entity " << entity << ": tree created at (" << x << ", " << y << ")" << std::endl;
    locationComponent loc{sf::Vector2i(x, y)};
    collisionComponent col{true}; 
    ResourceComponent res{1, false};
    RenderComponent render{EntityType::TREE, false, sf::Vector2f(x * TILE_SIZE, y * TILE_SIZE)};
    component_manager_.add_component(entity, loc);
    component_manager_.add_component(entity, col);
    component_manager_.add_component(entity, res);
    component_manager_.add_component(entity, render);   
    entity_map_[x][y].push_back(entity);
    return entity;
}

Entity World::create_wood(int x, int y) {
    Entity entity = entity_manager_.create_entity();
    std::cout << "Entity " << entity << ": wood created" << std::endl;
    locationComponent loc{sf::Vector2i(x, y)};
    collisionComponent col{false}; 
    ResourceComponent res{55, true};
    RenderComponent render{EntityType::WOOD};
    component_manager_.add_component(entity, loc);
    component_manager_.add_component(entity, col);
    component_manager_.add_component(entity, res);
    component_manager_.add_component(entity, render);
    entity_map_[x][y].push_back(entity);
    return entity;
}

Entity World::create_wall(int x, int y) {
    Entity entity = entity_manager_.create_entity();
    std::cout << "Entity " << entity << ": wall blueprint created" << std::endl;
    locationComponent location{sf::Vector2i(x, y)};
    collisionComponent collision{false};
    ConstructionComponent construction{true, true, true};
    RenderComponent render{EntityType::WALL};
    component_manager_.add_component(entity, location);
    component_manager_.add_component(entity, collision);
    component_manager_.add_component(entity, construction);
    component_manager_.add_component(entity, render);
    entity_map_[x][y].push_back(entity);
    return entity;
}

Entity World::create_door(int x, int y) {
    Entity entity = entity_manager_.create_entity();
    std::cout << "Entity " << entity << ": door blueprint created" << std::endl;
    locationComponent location{sf::Vector2i(x, y)};
    ConstructionComponent construction{true, true, true};
    collisionComponent collision{false};
    RenderComponent render{EntityType::DOOR};
    //减速实现未完成
    component_manager_.add_component(entity, location);
    component_manager_.add_component(entity, construction);
    component_manager_.add_component(entity, collision);
    component_manager_.add_component(entity, render);
    entity_map_[x][y].push_back(entity);
    return entity;
}

Entity World::create_dog(int x, int y) {
    Entity entity = entity_manager_.create_entity();
    std::cout << "Entity " << entity << ": dog created" << std::endl;
    locationComponent location{sf::Vector2i(x, y)};
    collisionComponent collision{true};
    RenderComponent render{EntityType::DOG, false, sf::Vector2f(x * TILE_SIZE, y * TILE_SIZE)};
    moveComponent move{0.0f, 0.0f}; 
    component_manager_.add_component(entity, location);
    component_manager_.add_component(entity, collision);
    component_manager_.add_component(entity, render);
    component_manager_.add_component(entity, move);
    entity_map_[x][y].push_back(entity);
    return entity;
}

Entity World::create_storage(int x, int y) {
    Entity entity = entity_manager_.create_entity();
    std::cout << "Entity " << entity << ": storage created" << std::endl;
    locationComponent location{sf::Vector2i(x, y)};
    collisionComponent collision{false};
    StorageComponent storage{110};
    RenderComponent render{EntityType::STORAGE, false, sf::Vector2f(x * TILE_SIZE, y * TILE_SIZE)};
    component_manager_.add_component(entity, location);
    component_manager_.add_component(entity, collision);
    component_manager_.add_component(entity, storage);
    component_manager_.add_component(entity, render);
    entity_map_[x][y].push_back(entity);
    return entity;
}

bool World::is_valid_position(int x, int y) {
    Entities entities = entity_map_[x][y];
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
        if (is_valid_position(x, y)) {
            create_tree(x, y);
            --count;
        }
    }
}

std::vector<Entity> World::get_active_entities() {
    std::vector<Entity> active_entities;
    for(int i = 0; i < MAX_ENTITIES; ++i) {
        if(entity_manager_.is_entity_alive(i)) {
            active_entities.push_back(i);
        }
    }
    return active_entities;
}


void World::make_storage_area(sf::Vector2i start, sf::Vector2i end) {
    int start_x = std::min(start.x, end.x);
    int start_y = std::min(start.y, end.y);
    int end_x = std::max(start.x, end.x);
    int end_y = std::max(start.y, end.y);
    std::cout << "try create storage area from (" << start_x << ", " << start_y << ") to (" << end_x << ", " << end_y << ")" << std::endl;
    for(int i = start_x; i <= end_x; ++i) {
        for(int j = start.y; j <= end.y; ++j) {
            create_storage(i, j);
        }
    }
}

