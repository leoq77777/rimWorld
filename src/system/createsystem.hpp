#pragma once

#include "utils/path.hpp"

class CreateSystem {
    ComponentManager& component_manager_;
    EntityManager& entity_manager_;
    Router& router_;
    std::vector<std::vector<Entities>>& entity_map_;
    int map_size_;
    int tile_size_;
    Entities characters_;
    Entities animals_;
public:
    CreateSystem();
    CreateSystem(ComponentManager& component_manager, EntityManager& entity_manager, Router& router, std::vector<std::vector<Entities>>& entity_map, int tile_size) 
        : component_manager_(component_manager), 
          entity_manager_(entity_manager), 
          router_(router), 
          entity_map_(entity_map),
          tile_size_(tile_size) {
        std::cout << "CreateSystem initialized" << std::endl;
        map_size_ = entity_map_.size();
    }
    void create_entities();
    void destroy_entities();
    void update();
    Entity create_character(Location pos);
    Entity create_tree(Location pos);
    Entity create_wood(Location pos);
    Entity create_wall(Location pos);
    Entity create_door(Location pos);
    Entity create_dog(Location pos);
    Entity create_storage(Location pos);
    Entities get_characters() {
        return characters_;
    }
    Entities get_animals() {
        return animals_;
    }
};

void CreateSystem::update() {
    //std::cout << "CreateSystem: updating" << std::endl;
    destroy_entities();
    create_entities();
}

void CreateSystem::create_entities() {
    //std::cout << "CreateSystem: creating entities" << std::endl;
    for(auto& entity : entity_manager_.get_all_entities()) {
        if (!component_manager_.has_component<CreateComponent>(entity)) {
            continue;
        }

        auto& create = component_manager_.get_component<CreateComponent>(entity);
        if (create.to_be_created) {
            std::cout << "CreateSystem: creating entity " << entity << std::endl;
            auto& type = create.entity_type;
            auto& pos = component_manager_.get_component<LocationComponent>(entity).loc;
            if (!router_.is_valid_position(pos)) {
                std::cout << "CreateSystem: invalid position to create" << std::endl;
                //wail until it is valid?
                //entity_manager_.destroy_entity(entity);
                continue;
            }
            auto& amount = create.amount;
            for(int i = 0; i < amount; i++) {
                switch(type) {
                    case EntityType::CHARACTER:
                        create_character(pos);
                    break;
                    case EntityType::TREE:
                        create_tree(pos);
                        break;
                    case EntityType::WOODPACK:
                        create_wood(pos);
                        break;
                    case EntityType::WALL:
                        create_wall(pos);
                        break;
                    case EntityType::DOOR:
                        create_door(pos);
                        break;
                    case EntityType::DOG:
                        create_dog(pos);
                        break;
                    case EntityType::STORAGE:
                        create_storage(pos);
                        break;
                }
            }
            entity_manager_.destroy_entity(entity);
        }
    }
}

void CreateSystem::destroy_entities() {
    //std::cout << "CreateSystem: destroying entities" << std::endl;
    for(auto& entity : router_.get_all_entities()) {
        if (!component_manager_.has_component<TargetComponent>(entity)) {
            continue;
        }

        auto& track = component_manager_.get_component<TargetComponent>(entity);
        if (track.to_be_deleted) {
            std::cout << "Entity " << entity << " deleted" << std::endl;
            entity_manager_.destroy_entity(entity);
        }   
    }
}

Entity CreateSystem::create_character(Location loc) {
    Entity entity = entity_manager_.create_entity();
    std::cout << "Entity " << entity << ": character created at (" << loc.x << ", " << loc.y << ")" << std::endl;
    LocationComponent location{loc};
    CollisionComponent collision{true};
    MovementComponent move{loc, loc, 0.0f, 0};
    RenderComponent render{EntityType::CHARACTER, false, RenderPos{loc.x * tile_size_, loc.y * tile_size_}};
    
    component_manager_.add_component(entity, location);
    component_manager_.add_component(entity, collision);
    component_manager_.add_component(entity, move);
    component_manager_.add_component(entity, render);
    
    entity_map_[loc.x][loc.y].push_back(entity);
    characters_.push_back(entity);
    return entity;
}

Entity CreateSystem::create_tree(Location loc) {
    Entity entity = entity_manager_.create_entity();
    std::cout << "Entity " << entity << ": tree created at (" << loc.x << ", " << loc.y << ")" << std::endl;
    LocationComponent location{loc};
    CollisionComponent collision{true}; 
    ResourceComponent resource{1, false};
    RenderComponent render{EntityType::TREE, false, RenderPos{loc.x * tile_size_, loc.y * tile_size_}};
    component_manager_.add_component(entity, location);
    component_manager_.add_component(entity, collision);
    component_manager_.add_component(entity, resource);
    component_manager_.add_component(entity, render);   
    entity_map_[loc.x][loc.y].push_back(entity);
    return entity;
}

Entity CreateSystem::create_wood(Location loc) {
    Entity entity = entity_manager_.create_entity();
    std::cout << "Entity " << entity << ": wood created" << std::endl;
    LocationComponent location{loc};
    CollisionComponent collision{false}; 
    //one pack has 5 woods without holder
    ResourceComponent resource{5, -1};
    RenderComponent render{EntityType::WOODPACK, false, RenderPos{loc.x * tile_size_, loc.y * tile_size_}};
    component_manager_.add_component(entity, location);
    component_manager_.add_component(entity, collision);
    component_manager_.add_component(entity, resource);
    component_manager_.add_component(entity, render);
    entity_map_[loc.x][loc.y].push_back(entity);
    return entity;
}

Entity CreateSystem::create_wall(Location loc) {
    Entity entity = entity_manager_.create_entity();
    std::cout << "Entity " << entity << ": wall blueprint created" << std::endl;
    LocationComponent location{loc};
    CollisionComponent collision{false};
    ConstructionComponent construction{false, false};
    StorageComponent storage{5, 0};
    RenderComponent render{EntityType::WALL, false, RenderPos{loc.x * tile_size_, loc.y * tile_size_}};
    TargetComponent target{
        .progress = 0,
        .timer = 0,
        .is_target = true,
        .is_finished = false,
        .to_be_deleted = false,
        .hold_by = -1
    };

    component_manager_.add_component(entity, location);
    component_manager_.add_component(entity, collision);
    component_manager_.add_component(entity, construction);
    component_manager_.add_component(entity, storage);
    component_manager_.add_component(entity, render);
    component_manager_.add_component(entity, target);
    entity_map_[loc.x][loc.y].push_back(entity);
    return entity;
}

Entity CreateSystem::create_door(Location loc) {
    Entity entity = entity_manager_.create_entity();
    std::cout << "Entity " << entity << ": door blueprint created" << std::endl;
    LocationComponent location{loc};
    ConstructionComponent construction{false, false};
    CollisionComponent collision{false};
    StorageComponent storage{25, 0};
    RenderComponent render{EntityType::DOOR, false, RenderPos{loc.x * tile_size_, loc.y * tile_size_}};
    TargetComponent target{
        .progress = 0,
        .timer = 0,
        .is_target = true,
        .is_finished = false,
        .to_be_deleted = false,
        .hold_by = -1
    };

    //减速实现未完成
    component_manager_.add_component(entity, location);
    component_manager_.add_component(entity, construction);
    component_manager_.add_component(entity, collision);
    component_manager_.add_component(entity, storage);
    component_manager_.add_component(entity, render);
    component_manager_.add_component(entity, target);

    entity_map_[loc.x][loc.y].push_back(entity);
    return entity;
}

Entity CreateSystem::create_dog(Location loc) {
    Entity entity = entity_manager_.create_entity();
    std::cout << "Entity " << entity << ": dog created" << std::endl;
    LocationComponent location{loc};
    CollisionComponent collision{true};
    RenderComponent render{EntityType::DOG, false, RenderPos{loc.x * tile_size_, loc.y * tile_size_}};
    MovementComponent move{loc, loc, 0.0f, 0};
    component_manager_.add_component(entity, location);
    component_manager_.add_component(entity, collision);
    component_manager_.add_component(entity, render);
    component_manager_.add_component(entity, move);
    entity_map_[loc.x][loc.y].push_back(entity);
    animals_.push_back(entity);
    return entity;
}

Entity CreateSystem::create_storage(Location loc) {
    Entity entity = entity_manager_.create_entity();
    std::cout << "Entity " << entity << ": storage created" << std::endl;
    LocationComponent location{loc};
    CollisionComponent collision{false};
    StorageComponent storage{1100, 0};
    RenderComponent render{EntityType::STORAGE};
    component_manager_.add_component(entity, location);
    component_manager_.add_component(entity, collision);
    component_manager_.add_component(entity, storage);
    component_manager_.add_component(entity, render);
    entity_map_[loc.x][loc.y].push_back(entity);
    return entity;
}
