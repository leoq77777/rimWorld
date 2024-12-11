#pragma once
#include "world/world.hpp"
#include "components/componentManager.hpp"
#include "entities/entity.hpp"
#include "utils/path.hpp"
class ActionSystem {
    static constexpr float BASE_MOVE_SPEED = 2.0f;  // 基础移动速度（格/秒）
    static constexpr float DOG_SPEED_MULTIPLIER = 1.5f;  // 狗的速度倍率
public:
    ActionSystem(World& new_world) 
        : world_(new_world), 
          entity_manager_(world_.entity_manager_), 
          component_manager_(world_.component_manager_) {
        std::cout << "ActionSystem initialized" << std::endl;
    }

    void update(float dt) {
        auto entities = world_.get_entities_with_components<TaskComponent>();
        for (auto entity : entities) {
            if (!component_manager_.has_component<ActionComponent>(entity)) {
                continue;
            }

            auto& action = component_manager_.get_component<ActionComponent>(entity);
            auto& task = component_manager_.get_component<TaskComponent>(entity);
            
            //assign action based on current task
            assign_action(entity);

            //execute
            execute_current_action(entity);
        }
    }

private:
    Action wander(Entity entity) {
        int direction = rand() % 4;
        Location next_pos;
        Location cur_pos = component_manager_.get_component<LocationComponent>(entity).loc;
        
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
            .duration = 1.0f / BASE_MOVE_SPEED,  // 使用基础移动速度计算duration
            .target_entity = entity
        };
    }

    Action none_action(Entity entity) {
        return Action{
            .type = ActionType::NONE, 
            .target_location = component_manager_.get_component<LocationComponent>(entity).loc, 
            .duration = 0, 
            .target_entity = entity};
    }

    Action move_action(Entity entity, Path path) {
        auto& next_pos = path.front();
        return Action{
            .type = ActionType::MOVE, 
            .target_location = next_pos, 
            .duration = 1.0f / BASE_MOVE_SPEED,  // 使用基础移动速度计算duration
            .target_entity = entity
        };
    }
    
    void assign_action(Entity entity) {
        auto& action = component_manager_.get_component<ActionComponent>(entity);
        //consider only current task
        auto& task = component_manager_.get_component<TaskComponent>(entity).current_task;
        auto& cur_pos = component_manager_.get_component<LocationComponent>(entity).loc;
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
        auto& current_action = component_manager_.get_component<ActionComponent>(entity).current_action;
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
        auto& action = component_manager_.get_component<ActionComponent>(entity);
        action.current_action = none_action(entity);
    }

    //entity is character or animal
    void move(Entity entity) {
        auto& action = component_manager_.get_component<ActionComponent>(entity);
        auto& cur_pos = component_manager_.get_component<LocationComponent>(entity).loc;
        auto& target_pos = action.current_action.target_location;
        
        if (cur_pos == target_pos) {
            finish_action(entity);
            return;
        }

        if (!component_manager_.has_component<MovementComponent>(entity)) {
            float speed = BASE_MOVE_SPEED;
            // 根据实体类型设置不同速度
            auto& type = component_manager_.get_component<RenderComponent>(entity).entityType;
            if (type == EntityType::DOG) {
                speed = BASE_MOVE_SPEED * 1.5f;
            }
            
            component_manager_.add_component(entity, MovementComponent{
                .start_pos = cur_pos,
                .end_pos = target_pos,
                .progress = 0.0f,
                .speed = speed
            });
        }

        auto& movement = component_manager_.get_component<MovementComponent>(entity);
        movement.progress += movement.speed * (1.0f/FRAMERATE);

        if (movement.progress >= 1.0f) {
            cur_pos = target_pos;
            component_manager_.remove_component<MovementComponent>(entity);
        }
    }

    //entity is character
    void chop(Entity entity) {
        auto& action = component_manager_.get_component<ActionComponent>(entity).current_action;
        auto& tree = action.target_entity;
        auto& tree_pos = component_manager_.get_component<LocationComponent>(tree).loc;
        auto& entity_pos = component_manager_.get_component<LocationComponent>(entity).loc;
        //every update is 1s / framerate(s), duration is 5s
        auto& track = component_manager_.get_component<TargetComponent>(tree);
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
        auto& action = component_manager_.get_component<ActionComponent>(entity).current_action;
        auto& target = action.target_entity;
        auto& target_pos = component_manager_.get_component<LocationComponent>(target).loc;
        auto& render = component_manager_.get_component<RenderComponent>(entity);
        auto& bag = component_manager_.get_component<StorageComponent>(entity);
        //PICK ACTION has target entity of RESOURCE or STORAGE
        if (render.type == EntityType::WOODPACK) {
            for(auto& pack : world_.get_entity_at_location(target_pos)) {//one unit has 5 woodpacks
                if (component_manager_.has_component<RenderComponent>(pack).entityType != EntityType::WOODPACK) {
                    continue;
                }
                auto& resource = component_manager_.get_component<ResourceComponent>(pack);
                auto& amount = resource.amount;
                auto& holder = resource.holder;
                if (bag.storage_capacity - bag.current_storage >= 5) {
                    bag.current_storage += amount;
                    bag.stored_resources.emplace_back(target);
                    holder = entity;
                }
            }
            finish_action(entity);
        } else if (render.type == EntityType::STORAGE) {
            for(auto& pack : world_.get_entity_at_location(target_pos)) {//one unit has ? packs
                if (component_manager_.has_component<RenderComponent>(pack).entityType != EntityType::WOODPACK) {
                    continue;
                }
                auto& resource = component_manager_.get_component<ResourceComponent>(pack);
                auto& amount = resource.amount;
                auto& holder = resource.holder;
                if (bag.storage_capacity - bag.current_storage >= 5) {
                    bag.current_storage += amount;
                    bag.stored_resources.emplace_back(target);
                    holder = entity;
                }
            }
            finish_action(entity);
        }
    }

    //entity is character
    void place(Entity entity) {
        auto& action = component_manager_.get_component<ActionComponent>(entity);
        auto& site = action.target_entity;
        auto& site_pos = component_manager_.get_component<LocationComponent>(site).loc;
        auto& entity_pos = component_manager_.get_component<LocationComponent>(entity).loc;

        auto& carriage = component_manager_.get_component<StorageComponent>(entity);
        auto& storage = component_manager_.get_component<StorageComponent>(site);
        
        auto& render = component_manager_.get_component<RenderComponent>(site);
        if (render.type == EntityType::STORAGE) {
            //put all woodpacks into storage
            storage += carriage.current_storage;
            carriage.current_storage = 0;
            //transfer ownership
            for (auto& resource : storage.stored_resources) {
                component_manager_.get_component<ResourceComponent>(resource).holder = site;
            }
            storage.stored_resources.clear();
        } else if (render.type == EntityType::WALL || render.type == EntityType::DOOR) {
            while(storage.current_storage < storage.storage_capacity && carriage.current_storage > 0) {
                auto& cur_resource = carriage.stored_resources.front();
                //transfer ownership of current woodpack(entity)
                component_manager_.get_component<ResourceComponent>(cur_resource).holder = site;
                
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
        auto& action = component_manager_.get_component<ActionComponent>(entity);
        auto& blueprint = action.target_entity;
        auto& track = component_manager_.get_component<trackComponent>(blueprint);
        track.progress += 100 / action.duration * FRAMERATE;
        if (track.progress >= 100) {
            track.is_finished = true;
            finish_action(entity);
            component_manager_.add_component(blueprint, CollisionComponent{true});
            component_manager_.remove_component<TargetComponent>(blueprint);
            component_manager_.remove_component<TaskComponent>(blueprint);
            auto& construction = component_manager_.get_component<ConstructionComponent>(blueprint);
            construction.is_built = true;
        }
    }

    World& world_;
    ComponentManager& component_manager_;
    EntityManager& entity_manager_;
};