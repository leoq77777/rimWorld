#pragma once

#include "utils/path.hpp"

class CreateSystem {
    ComponentManager& component_manager_;
    EntityManager& entity_manager_;
    Router& router_;
    int map_size_;
    int tile_size_;
    Entities characters_;
    Entities animals_;
    Entities storages_;
    Entities doors_;
public:
    CreateSystem();
    CreateSystem(ComponentManager& component_manager, EntityManager& entity_manager, Router& router, int tile_size) 
        : component_manager_(component_manager), 
          entity_manager_(entity_manager), 
          router_(router), 
          tile_size_(tile_size) {
        std::cout << "CreateSystem initialized" << std::endl;
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
    Entities get_storages() {
        return storages_;
    }
    Entities get_doors() {
        return doors_;
    }
};

void CreateSystem::update() {
    //std::cout << "CreateSystem: updating" << std::endl;
    destroy_entities();
    create_entities();
}

void CreateSystem::create_entities() {
    std::cout << "CreateSystem: creating entities" << std::endl;
    for(auto& entity : router_.get_entities_with_components<CreateComponent>()) {
        if (!component_manager_.has_component<CreateComponent>(entity)) {
            continue;
        }

        auto& create = component_manager_.get_component<CreateComponent>(entity);
        if (create.to_be_created) {
            std::cout << "CreateSystem: creating entity " << entity << std::endl;
            auto& type = create.entity_type;
            Location pos = component_manager_.get_component<LocationComponent>(entity).loc;
            if (!router_.is_valid_position(pos)) {
                std::cout << "CreateSystem: invalid position to create" << std::endl;
                //wail until it is valid?
                //entity_manager_.destroy_entity(entity);
                continue;
            }
            auto& amount = create.amount;
            std::cout << "CreateSystem: creating " << amount << " entities" << std::endl;
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
    std::cout << "CreateSystem: destroying entities" << std::endl;
    for(auto& entity : router_.get_all_entities()) {
        if (!component_manager_.has_component<TargetComponent>(entity)) {
            continue;
        }

        auto& track = component_manager_.get_component<TargetComponent>(entity);
        auto& pos = component_manager_.get_component<LocationComponent>(entity).loc;
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
    MovementComponent move{loc, loc, 0.0f, 0};
    RenderComponent render{
        .entityType = EntityType::CHARACTER, 
        .is_selected = false, 
        .collidable = false, 
    };
    StorageComponent storage{
        .storage_capacity = 1100,
        .current_storage = 0,
        .stored_resources = {}
    };

    component_manager_.add_component(entity, location);
    component_manager_.add_component(entity, move);
    component_manager_.add_component(entity, render);
    component_manager_.add_component(entity, storage);
    characters_.push_back(entity);
    return entity;
}

Entity CreateSystem::create_tree(Location loc) {
    Entity entity = entity_manager_.create_entity();
    std::cout << "Entity " << entity << ": tree created at (" << loc.x << ", " << loc.y << ")" << std::endl;
    LocationComponent location{loc};
    ResourceComponent resource{1, false};
    RenderComponent render{
        .entityType = EntityType::TREE, 
        .is_selected = false, 
        .collidable = true};
    component_manager_.add_component(entity, location);
    component_manager_.add_component(entity, resource);
    component_manager_.add_component(entity, render);   
    return entity;
}

Entity CreateSystem::create_wood(Location loc) {
    Entity entity = entity_manager_.create_entity();
    std::cout << "Entity " << entity << ": wood created" << std::endl;
    LocationComponent location{loc};
    //one pack has 5 woods without holder
    ResourceComponent resource{5, -1};
    RenderComponent render{
        .entityType = EntityType::WOODPACK, 
        .is_selected = false, 
        .collidable = false};
    TargetComponent target{
        .progress = 0,
        .timer = 0,
        .is_target = true,
        .is_finished = false,
        .to_be_deleted = false,
        .hold_by = -1
    };

    component_manager_.add_component(entity, location);
    component_manager_.add_component(entity, resource);
    component_manager_.add_component(entity, render);
    component_manager_.add_component(entity, target);

    return entity;
}

Entity CreateSystem::create_wall(Location loc) {
    Entity entity = entity_manager_.create_entity();
    std::cout << "Entity " << entity << ": wall blueprint created" << std::endl;
    LocationComponent location{loc};
    ConstructionComponent construction{
        .allocated = false,
        .is_built = false
    };
    StorageComponent storage{5, 0};
    RenderComponent render{
        .entityType = EntityType::WALL, 
        .is_selected = false, 
        .collidable = true};
    TargetComponent target{
        .progress = 0,
        .timer = 0,
        .is_target = true,
        .is_finished = false,
        .to_be_deleted = false,
        .hold_by = -1
    };

    component_manager_.add_component(entity, location);
    component_manager_.add_component(entity, construction);
    component_manager_.add_component(entity, storage);
    component_manager_.add_component(entity, render);
    component_manager_.add_component(entity, target);
    return entity;
}

Entity CreateSystem::create_door(Location loc) {
    Entity entity = entity_manager_.create_entity();
    std::cout << "Entity " << entity << ": door blueprint created" << std::endl;
    LocationComponent location{loc};
    ConstructionComponent construction{
        .allocated = false,
        .is_built = false
    };
    StorageComponent storage{25, 0};
    RenderComponent render{
        .entityType = EntityType::DOOR, 
        .is_selected = false, 
        .collidable = false};
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
    component_manager_.add_component(entity, storage);
    component_manager_.add_component(entity, render);
    component_manager_.add_component(entity, target);

    return entity;
}

Entity CreateSystem::create_dog(Location loc) {
    Entity entity = entity_manager_.create_entity();
    std::cout << "Entity " << entity << ": dog created" << std::endl;
    LocationComponent location{loc};
    RenderComponent render{
        .entityType = EntityType::DOG, 
        .is_selected = false, 
        .collidable = true};
    MovementComponent move{loc, loc, 0.0f, 0};
    component_manager_.add_component(entity, location);
    component_manager_.add_component(entity, render);
    component_manager_.add_component(entity, move);
    animals_.push_back(entity);
    return entity;
}

Entity CreateSystem::create_storage(Location loc) {
    Entity entity = entity_manager_.create_entity();
    std::cout << "Entity " << entity << ": storage created" << std::endl;
    LocationComponent location{loc};
    StorageComponent storage{1100, 0};
    RenderComponent render{
        .entityType = EntityType::STORAGE, 
        .is_selected = false, 
        .collidable = false};
    component_manager_.add_component(entity, location);
    component_manager_.add_component(entity, storage);
    component_manager_.add_component(entity, render);
    storages_.emplace_back(entity);
    return entity;
}
