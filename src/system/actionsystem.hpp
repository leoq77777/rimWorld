#pragma once
#include "world/world.h"

class ActionSystem {
public:
    ActionSystem(World& new_world) : world_(new_world) {
        entity_manager_ = world_.entity_manager_;
        component_manager_ = world_.component_manager_;
        std::cout << "ActionSystem initialized" << std::endl;
    }

    void update(float dt) {
        auto entities = world_.get_entities_with_components<TaskComponent>();
        for (auto entity : entities) {
            if (!component_manager.has_component<ActionComponent>(entity)) {
                continue;
            }

            auto& action = component_manager.get_component<ActionComponent>(entity);
            auto& task = component_manager.get_component<TaskComponent>(entity);
            
            //assign action based on current task
            assign_action(entity);

            //execute
            execute_current_action(entity);
        }
    }

private:
    Action wander(Entity entity) {
        int direction = rand() % 4;
        float speed = rand() % DEFAULT_SPEED;
        Location next_pos;
        Location cur_pos = component_manager.get_component<locationComponent>(entity).loc;
        if (direction == 0) {
            next_pos = {std::min(cur_pos.x + 1, MAP_SIZE - 1), cur_pos.y};
        } else if (direction == 1) {
            next_pos = {std::max(cur_pos.x - 1, 0), cur_pos.y};
        } else if (direction == 2) {
            next_pos = {cur_pos.x, std::min(cur_pos.y + 1, MAP_SIZE - 1)};
        } else if (direction == 3) {
            next_pos = {cur_pos.x, std::max(cur_pos.y - 1, 0)};
        }
        return Action{
            .type = ActionType::MOVE, 
            .target_location = next_pos, 
            .duration = 0, 
            .target_entity = entity};
    }

    Action none_action(Entity entity) {
        return Action{
            .type = ActionType::NONE, 
            .target_location = component_manager.get_component<locationComponent>(entity).loc, 
            .duration = 0, 
            .target_entity = entity};
    }

    Action move_action(Entity entity, Path path) {
        auto& next_pos = path.front();
        return Action{
            .type = ActionType::MOVE, 
            .target_location = next_pos, 
            .duration = static_cast<float>(path.size() * 0.1), 
            .target_entity = entity};
    }
    
    void assign_action(Entity entity) {
        auto& action = component_manager.get_component<ActionComponent>(entity);
        //consider only current task
        auto& task = component_manager.get_component<TaskComponent>(entity).current_task;
        auto& cur_pos = component_manager.get_component<locationComponent>(entity).loc;
        auto& target_pos = task.target_locations;

        //isolately deal with idle task
        if (task.type == TaskType::IDLE) {
            action.current_action = wander(entity);
            return;
        }

        //arrive at target location, take some action if needed
        if (is_in_field(cur_pos, target_pos)) {
            action.current_action = task.target_action;
        } else {
            //update path, and move
            Path path = find_path(cur_pos, target_pos, world_);
            //could not reach target location, set task unfeasible
            if (path.empty()) {
                task.feasible = false;
                task.target_action = Action{ActionType::NONE, location(), 0, Entity()};
                return;
            }   
            //move
            action.current_action = move_action(entity, path);
        }
    }
    
    void execute_current_action(Entity entity) {
        auto& current_action = component_manager.get_component<ActionComponent>(entity);
        auto& task = component_manager.get_component<TaskComponent>(entity);
        auto& cur_pos = component_manager.get_component<locationComponent>(entity).loc;
        auto& target_pos = task.target_location;
        if (current_action.type == ActionType::MOVE) {
            move(entity);
        }
        else if (current_action.type == ActionType::CHOP) {
            chop(entity);
        }
        else if (current_action.type == ActionType::PICK) {
            pick(entity);
        }
        else if (current_action.type == ActionType::PLACE) {
            place(entity);
        }
        else if (current_action.type == ActionType::BUILD) {
            build(entity);
        }
    }

    void finish_action(Entity entity) {
        auto& action = component_manager.get_component<ActionComponent>(entity);
        action.current_action = none_action(entity);
    }

    //entity is character or animal
    void move(Entity entity) {
        auto& action = component_manager.get_component<ActionComponent>(entity);
        auto& cur_pos = component_manager.get_component<locationComponent>(entity).loc;
        auto& target_pos = action.target_location;
        if (cur_pos == target_pos) {
            return;
        }
    }

    //entity is character
    void chop(Entity entity) {
        auto& action = component_manager.get_component<ActionComponent>(entity);
        auto& tree = action.target_entity;
        auto& tree_pos = component_manager.get_component<locationComponent>(tree).loc;
        auto& entity_pos = component_manager.get_component<locationComponent>(entity).loc;
        //every update is 1s / framerate(s), duration is 5s
        auto& track = component_manager.get_component<trackComponent>(tree);
        track.progress += 100 / action.duration * FRAMERATE;
        if (track.progress >= action.duration) {
            track.is_finished = true;
            track.to_be_deleted = true;
            finish_action(entity);
            for(int i = 0; i < 11; i++) {
                world_.create_wood(tree_pos);
            }
        }
    }

    //entity is character
    void pick(Entity entity) {
        //add a storage if entity does not have one
        if (!component_manager.has_component<StorageComponent>(entity)) {
            component_manager.add_component(entity, StorageComponent{999, 0});
        }

        auto& action = component_manager.get_component<ActionComponent>(entity);
        auto& target = action.target_entity;
        auto& render = component_manager.get_component<RenderComponent>(entity);
        //PICK ACTION has target entity of RESOURCE or STORAGE
        if (render.type == EntityType::WOODPACK) {
            auto& resource = component_manager.get_component<ResourceComponent>(target);
            auto& amount = resource.amount;
            auto& holder = resource.holder;
            if (bag.storage_capacity - bag.current_storage >= 5) {
                bag.current_storage += amount;
                bag.stored_resources.emplace_back(resource_entity);
                holder = entity;
            } else {
                std::cout << "bag is full" << std::endl;
            }
        } else if (render.type == EntityType::STORAGE) {
            
        }
    }

    //entity is character
    void place(Entity entity) {
        auto& action = component_manager.get_component<ActionComponent>(entity);
        auto& site = action.target_entity;
        auto& site_pos = component_manager.get_component<locationComponent>(site).loc;
        auto& entity_pos = component_manager.get_component<locationComponent>(entity).loc;

        auto& carriage = component_manager.get_component<StorageComponent>(entity);
        auto& storage = component_manager.get_component<StorageComponent>(site);
        
        auto& render = component_manager.get_component<RenderComponent>(site);
        if (render.type == EntityType::STORAGE) {
            //put all woodpacks into storage
            storage += carriage.current_storage;
            carriage.current_storage = 0;
            //transfer ownership
            for (auto& resource : storage.stored_resources) {
                component_manager.get_component<ResourceComponent>(resource).holder = site;
            }
            storage.stored_resources.clear();
        } else if (render.type == EntityType::WALL || render.type == EntityType::DOOR) {
            while(storage.current_storage < storage.storage_capacity && carriage.current_storage > 0) {
                auto& cur_resource = carriage.stored_resources.front();
                //transfer ownership of current woodpack(entity)
                component_manager.get_component<ResourceComponent>(cur_resource).holder = site;
                
                //update storage
                storage.current_storage += 5;
                carriage.current_storage -= 5;
                storage.stored_resources.emplace_back(cur_resource);
                carriage.stored_resources.pop_front();
            }
            //check if enough woodpacks to build
            if (storage.current_storage >= storage.storage_capacity) {
                std::cout << "enough woodpacks to build" << std::endl;
                site.allocated = true;
                //if character carries resource, then he just carries
                //until allocate/store task is assigned to him
            //check if woodpacks in bag are used up
            } else if (carriage.current_storage == 0) {
                std::cout << "all woodpacks in bag are placed" << std::endl;
                return;
            }
        }
        //draw resource! based on current_storage displaying amount
    }

    //entity is character
    void build(Entity entity) {
        auto& action = component_manager.get_component<ActionComponent>(entity);
        auto& blueprint = action.target_entity;
        auto& track = component_manager.get_component<trackComponent>(blueprint);
        track.progress += 100 / action.duration * FRAMERATE;
        if (track.progress >= 100) {
            track.is_finished = true;
            finish_action(entity);
            component_manager.add_component(blueprint, CollisionComponent{true});
            component_manager.remove_component<TargetComponent>(blueprint);
            component_manager.remove_component<TaskComponent>(blueprint);
            auto& construction = component_manager.get_component<ConstructionComponent>(blueprint);
            construction.is_built = true;
        }
    }

    World& world_;
    ComponentManager& component_manager;
    EntityManager& entity_manager;
};