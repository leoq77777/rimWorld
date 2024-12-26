#pragma once
#include "utils/path.hpp"
#include <cassert>
class ActionSystem {
    static constexpr float BASE_MOVE_SPEED = 2.0f; 
    static constexpr float DOG_SPEED_MULTIPLIER = 1.5f;  
    static constexpr float HAS_TASK_BOOSTER = 2.0f;
    ComponentManager& component_manager_;
    EntityManager& entity_manager_;
    Router& router_;
    int map_size_;
    int framerate_;
public:
    ActionSystem();
    ActionSystem(ComponentManager& component_manager, EntityManager& entity_manager, Router& router, int map_size, int framerate) 
        : component_manager_(component_manager), 
          entity_manager_(entity_manager), 
          router_(router), 
          map_size_(map_size),
          framerate_(framerate) {
        std::cout << "ActionSystem initialized" << std::endl;
    }

    void update()  {
        bool print = true;
        std::cout << "try update action" << std::endl;
        auto entities = router_.get_entities_with_components<TaskComponent>();
        
        for (auto entity : entities) {
            auto& type = component_manager_.get_component<RenderComponent>(entity).entityType;
            if (type == EntityType::DOG) 
                print = false;

            if (!component_manager_.has_component<ActionComponent>(entity)) {
                if (print) std::cout << "add action component to entity " << entity << std::endl;
                component_manager_.add_component(entity, ActionComponent{
                    .current_action = none_action(entity),
                    .action_finished = false
                });
            }

            auto& action = component_manager_.get_component<ActionComponent>(entity);
            auto& task = component_manager_.get_component<TaskComponent>(entity);

            if (print) router_.print_character_current_task(entity);

            assign_action(entity, print);

            execute_current_action(entity, print);
        }
    }

private:
    

    void assign_action(Entity entity, bool print) {
        if (print) std::cout << "try assign action to entity " << entity << std::endl;
        auto& action = component_manager_.get_component<ActionComponent>(entity);
        //consider only current task
        auto& task = component_manager_.get_component<TaskComponent>(entity).current_task;
        auto& cur_pos = component_manager_.get_component<LocationComponent>(entity).loc;
        auto& target_pos = task.target_locations;
        

        //isolately deal with idle task
        if (task.type == TaskType::IDLE && router_.is_move_finished(entity)) {
            if (print) std::cout << "assign idle action to this entity " << entity << std::endl;
            action.current_action = wander(entity);
            action.action_finished = false;
            return;
        } else if (task.type == TaskType::IDLE) {
            return;
        }

        if (!is_character_available(entity)) {
            if (print) std::cout << "entity is in action, skip. Action Target: " << action.current_action.target_entity << std::endl;
            return;
        }

        //arrive at target location, take some action if needed
        if (router_.is_in_locations(cur_pos, target_pos)) {
            if (print) std::cout << "entity arrives at target location" << std::endl;
            if (is_character_available(entity) || 
                (!action.in_progress && !action.action_finished)) {
                action.current_action = task.target_action;
                action.in_progress = true;
                std::cout << "entity arrives at target location, start action" << std::endl;
                
                //IMPORTANT, if one arrives at task location, remember to delete his path
                if (component_manager_.has_component<MovementComponent>(entity)) {
                    auto& path = component_manager_.get_component<MovementComponent>(entity).path;
                    path = {};
                }
            } else {
                if (print) std::cout << "entity is in action, skip. Action Target: " << action.current_action.target_entity << std::endl;
            }
        } else {
            if (print) std::cout << "entity does not arrive at target location, update path and move" << std::endl;
            //update path, and move
            
            /*THIS is for that entity is doing task when the target loctaion becomes unreachable
            thus will set this task unfeasible
            (Because this costs a lot, and unreachable situation is rare, cancel it)
            if (!router_.is_reachable(cur_pos, target_pos)) {
                if (print) std::cout << "entity cannot reach target location, set task unfeasible" << std::endl;
                task.feasible = false;
                return;
            }
            */

            //move
            if (print) std::cout << "entity " << entity << " current action is move" << std::endl;
            //Here use a trick, do not update path if the next position is always valid
            //thus should store Path in the move action
            assert( component_manager_.has_component<MovementComponent>(entity) );
            auto& path = component_manager_.get_component<MovementComponent>(entity).path;
            //if don't has a path, add one
            if (path.size() == 0) {
                path = router_.find_path_to_locations(cur_pos, target_pos, "Ax");
                std::cout << "set a new path" << std::endl;
            }
            
            std::cout << "current pos: " << cur_pos.x << ", " << cur_pos.y << std::endl;

            //check if next step is valid
            auto& next_step = path.front();            
            std::cout << "next step at ( " << next_step.x << ", " << next_step.y << " )" << std::endl;

            if (next_step == cur_pos) {
                std::cout << "already in!" << std::endl;
                path.pop_front();
                return;
            }

            if (!router_.is_valid_position(next_step)) {
                std::cout << "next step invalid, re-calculate path" << std::endl;
                path = router_.find_path_to_locations(cur_pos, target_pos, "Ax");
            }

            //still invalid next step, set this task unfeasible
            if (!router_.is_valid_position(next_step)){
                std::cout << "still, next step is invalid, set task unfeasible" << std::endl;
                path = {};
                task.feasible = false;
                return;
            }

            //i do this to decrease the calculation time of routing which costs a lot
            action.current_action = move_action(entity, path);
            action.action_finished = false;
        }
    }
    
    void execute_current_action(Entity entity, bool print) {
        if (print) std::cout << "entity " << entity << " execute current action" << std::endl;
        auto& action = component_manager_.get_component<ActionComponent>(entity);
        auto& current_action = action.current_action;
        action.in_progress = true;
        action.action_finished = false;
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

    //doing nothing(different from idle task)
    bool is_character_available(Entity entity) {
        if (!component_manager_.has_component<ActionComponent>(entity)) {
            return true;
        }
        assert(component_manager_.has_component<TaskComponent>(entity));
        auto& task = component_manager_.get_component<TaskComponent>(entity);
        auto& action = component_manager_.get_component<ActionComponent>(entity);
        return action.current_action.type == ActionType::NONE || task.current_task.type == TaskType::IDLE;
    }

    void finish_current_action(Entity entity) {
        bool process_move = false;
        std::cout << "entity " << entity << " finishes action" << std::endl;
        assert(component_manager_.has_component<ActionComponent>(entity));
        auto& action = component_manager_.get_component<ActionComponent>(entity);
        
        //how can an entity without task component finish an action?
        assert(component_manager_.has_component<TaskComponent>(entity));
        auto& task = component_manager_.get_component<TaskComponent>(entity);
         //if during a task, entity finishes move action,
        //current task won't be replaced by idle task
        if (task.current_task.type != TaskType::IDLE && action.current_action.type == ActionType::MOVE
            || task.current_task.type == TaskType::IDLE) {
            process_move = true;
        }

        //set current action to none
        action.current_action = none_action(entity);
        //action.action_finished = true;
        //action.in_progress = false;
       
        
        //this guarantees that current task won't be replaced by idle task
        //if it is just moving to target location of task
        if(!process_move) {
            task.current_task.finished = true;
        }
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
            .target_location = Location{next_pos.x, next_pos.y}, 
            .duration = 1.0f / BASE_MOVE_SPEED,  // 使用基础移动速度计算duration
            .target_entity = entity
        };
    }
    
    Action wander(Entity entity) {
        int direction = rand() % 4;
        Location next_pos;
        Location cur_pos = component_manager_.get_component<LocationComponent>(entity).loc;
        
        if (direction == 0) {//move right
            next_pos = {std::min(cur_pos.x + 1, map_size_ - 1), cur_pos.y};
        } else if (direction == 1) {//move left
            next_pos = {std::max(cur_pos.x - 1, 0), cur_pos.y};
        } else if (direction == 2) {//move up
            next_pos = {cur_pos.x, std::min(cur_pos.y + 1, map_size_ - 1)};
        } else if (direction == 3) {//move down
            next_pos = {cur_pos.x, std::max(cur_pos.y - 1, 0)};
        }

        return Action{
            .type = ActionType::MOVE, 
            .target_location = next_pos, 
            .duration = 1.0f / BASE_MOVE_SPEED,
            .target_entity = entity
        };
    }

    //entity is character or animal
    void move(Entity entity) {
        std::cout << "entity " << entity << " move" << std::endl;
        auto& action = component_manager_.get_component<ActionComponent>(entity);
        auto& cur_pos = component_manager_.get_component<LocationComponent>(entity).loc;
        auto& target_pos = action.current_action.target_location;
        std::cout << "is moving from " << cur_pos.x << ", " << cur_pos.y << " to " << target_pos.x << ", " << target_pos.y << std::endl;
        if (cur_pos == target_pos) {
            finish_current_action(entity);
            return;
        }

        if (!router_.is_valid_position(target_pos)) {
            std::cout << "invalid next move target" << std::endl;
            finish_current_action(entity);
            return;
        }

        float speed = BASE_MOVE_SPEED;
        auto& type = component_manager_.get_component<RenderComponent>(entity).entityType;
        
        if (type == EntityType::DOG)
            speed = BASE_MOVE_SPEED * 1.5f;

        if (type == EntityType::CHARACTER && !is_character_available(entity))
            speed *= HAS_TASK_BOOSTER;
    
        if (router_.to_be_slow(cur_pos))
            speed *= 0.25;

        if (!component_manager_.has_component<MovementComponent>(entity)) {
            component_manager_.add_component(entity, MovementComponent{
                .start_pos = cur_pos,
                .end_pos = cur_pos, //don't worry, this will be updated when executing move action
                .progress = 0.0f,
                .speed = 0, //same to end_pos
                .path = {}
            });
        } 

        assert(component_manager_.has_component<MovementComponent>(entity));

        auto& movement = component_manager_.get_component<MovementComponent>(entity);
        //update movement component
        movement.speed = speed;
        movement.start_pos = cur_pos;
        movement.end_pos = target_pos;
        movement.progress += movement.speed * (1.0f / framerate_);

        if (movement.progress >= 1.0f) {
            cur_pos = target_pos;
            //reset movement
            movement = MovementComponent{
                .start_pos = cur_pos,
                .end_pos = cur_pos,
                .progress = 0.0f,
                .speed = BASE_MOVE_SPEED,
                .move_finished = true
            };
            
        }
    }

    //entity is character
    void chop(Entity entity) {
        std::cout << "entity " << entity << " chop" << std::endl;
        auto& action = component_manager_.get_component<ActionComponent>(entity).current_action;
        auto& tree = action.target_entity;
        if (!component_manager_.has_component<TargetComponent>(tree) || 
            !component_manager_.get_component<TargetComponent>(tree).is_target) {
            std::cout << "chop target: " << tree << "is not a target" << std::endl;
            return;
        }
        std::cout << "chop target: " << tree << std::endl;
        auto& tree_pos = component_manager_.get_component<LocationComponent>(tree).loc;
        auto& entity_pos = component_manager_.get_component<LocationComponent>(entity).loc;
        //every update is 1s / framerate(s), duration is 5s
        auto& track = component_manager_.get_component<TargetComponent>(tree);
        track.progress += 100 / (action.duration * framerate_);
        std::cout << "chop progress:" << track.progress << std::endl;
        if (track.progress >= 100) {
            //mark this target so that TaskSystem knows it's finished
            track.is_finished = true;
            //in Tasksystem, if a target of tree is finished, it will be marked 'to delete'
            //thus createsystem knows this tree should be deleted
            
            finish_current_action(entity);
            
            //temporary entity wait for create system to create actual entity(woodpack)
            Entity temp_entity = entity_manager_.create_entity();
            Location loc = tree_pos;
            component_manager_.add_component(temp_entity, LocationComponent{loc});
            component_manager_.add_component(temp_entity, CreateComponent{true, 11, EntityType::WOODPACK});
        }
    }

    //entity is character
    void pick(Entity entity) {
        std::cout << "entity " << entity << " pick" << std::endl;
        auto& action = component_manager_.get_component<ActionComponent>(entity).current_action;
        auto& target = action.target_entity;
        std::cout << "pick target: " << target << std::endl;
        auto& target_pos = component_manager_.get_component<LocationComponent>(target).loc;
        auto& target_type = component_manager_.get_component<RenderComponent>(target).entityType;
        auto& bag = component_manager_.get_component<StorageComponent>(entity);
        std::cout << "bag current storage: " << bag.current_storage << std::endl;
        std::cout << "bag storage capacity: " << bag.storage_capacity << std::endl;
        //PICK ACTION has target entity of RESOURCE or STORAGE
        //TASK: COLLECT
        if (target_type == EntityType::WOODPACK) {
            auto pack = target;//one unit has 5 woodpacks
            if (component_manager_.get_component<RenderComponent>(pack).entityType != EntityType::WOODPACK) {
                std::cout << "pick target is not woodpack" << std::endl;
                return;
            }
            auto& resource = component_manager_.get_component<ResourceComponent>(pack);
            auto& amount = resource.amount;
            auto& holder = resource.holder;
            //why if? because function is called only on 1 pack of woods
            if (bag.storage_capacity - bag.current_storage >= 5) {
                bag.current_storage += amount;
                bag.stored_resources.emplace_back(target);
                holder = entity;
                //once a pack of woods is picked/collected, it is not a target anymore
                auto& track = component_manager_.get_component<TargetComponent>(target);
                track.is_finished = true;
                std::cout << "target pack collection " << pack << " is finished" << std::endl;
            }
            finish_current_action(entity);
            //TASK: OBTAIN
        } else if (target_type == EntityType::STORAGE) {
            assert(component_manager_.has_component<StorageComponent>(target));
            auto& storage = component_manager_.get_component<StorageComponent>(target);
            
            int amount = 5;
            //obtain only 5 woods at one time
            if (bag.storage_capacity - bag.current_storage >= amount) {
                if (storage.stored_resources.empty()) {
                    std::cout << "storage is empty" << std::endl;
                    finish_current_action(entity);
                    return;
                }
                auto one_pack = storage.stored_resources.front();

                //transfer resource from storage to bag
                auto& resource = component_manager_.get_component<ResourceComponent>(one_pack);
                auto& holder = resource.holder;

                //update amount
                bag.current_storage += amount;
                storage.current_storage -= amount;

                //transfer ownership
                bag.stored_resources.emplace_back(one_pack);
                holder = entity;
                storage.stored_resources.pop_front();

                std::cout << "pick " << amount << " woods from " << target << " to " << entity << std::endl;
                std::cout << "now bag carries " << bag.current_storage << " woods" << std::endl;
                //auto& track = component_manager_.get_component<TargetComponent>(one_pack);
                //track.is_finished = true;
            }
            finish_current_action(entity);

            /* to delete
            auto& task = component_manager_.get_component<TaskComponent>(entity).current_task;
            task.finished = true;
            //THIS IS IMPORTANT, or tasksystem doesn't know this task is finished
            */
        }
        std::cout << "after pick, bag current storage: " << bag.current_storage << std::endl;
    }

    //entity is character
    void place(Entity entity) {
        std::cout << "entity " << entity << " place" << std::endl;
        auto& action = component_manager_.get_component<ActionComponent>(entity).current_action;
        auto& site = action.target_entity;
        std::cout << "place target: " << site << std::endl;
        auto& site_pos = component_manager_.get_component<LocationComponent>(site).loc;
        auto& entity_pos = component_manager_.get_component<LocationComponent>(entity).loc;
        auto& carriage = component_manager_.get_component<StorageComponent>(entity);
        auto& storage = component_manager_.get_component<StorageComponent>(site);
        
        auto& render = component_manager_.get_component<RenderComponent>(site);
        //TASK: STORE
        if (render.entityType == EntityType::STORAGE) {
            //put all woodpacks into storage
            storage.current_storage += carriage.current_storage;
            carriage.current_storage = 0;

            //transfer ownership (entity by entity)
            while(!carriage.stored_resources.empty()) {
                auto woodpack = carriage.stored_resources.front();
                component_manager_.get_component<ResourceComponent>(woodpack).holder = site;
                storage.stored_resources.emplace_back(woodpack);
                carriage.stored_resources.pop_front();

                //update location of woodpack
                auto& wood_loc = component_manager_.get_component<LocationComponent>(woodpack).loc;
                wood_loc = site_pos;
            }

            std::cout << "current storage unit has " << storage.current_storage << " woodpacks" << std::endl;
            assert(component_manager_.has_component<TargetComponent>(site));
            auto& track = component_manager_.get_component<TargetComponent>(site);
            track.is_finished = true;
            finish_current_action(entity);

            /*to delete
            auto& task = component_manager_.get_component<TaskComponent>(entity).current_task;
            task.finished = true;
            //THIS IS IMPORTANT, or tasksystem doesn't know this task is finished
            */

            //TASK: ALLOCATE
        } else if (render.entityType == EntityType::WALL || render.entityType == EntityType::DOOR) {
            while(storage.current_storage < storage.storage_capacity && carriage.current_storage > 0) {
                std::cout << "try allocate woodpack to blueprint" << std::endl;
                auto& cur_resource = carriage.stored_resources.back();
                //transfer ownership of current woodpack(entity)
                component_manager_.get_component<ResourceComponent>(cur_resource).holder = site;
                
                //update storage
                storage.current_storage += 5;
                carriage.current_storage -= 5;
                //transfer ownership of current woodpack(entity)
                component_manager_.get_component<ResourceComponent>(cur_resource).holder = site;
                storage.stored_resources.emplace_back(cur_resource);
                carriage.stored_resources.pop_back();

                //check if enough woodpacks to build
                if (storage.current_storage >= storage.storage_capacity) {
                    std::cout << "enough woodpacks to build" << std::endl;
                    component_manager_.get_component<ConstructionComponent>(site).allocated = true;
                    finish_current_action(entity);
                    return;
                    /*to delete
                    auto& task = component_manager_.get_component<TaskComponent>(entity).current_task;
                    task.finished = true;
                    //THIS IS IMPORTANT, or tasksystem doesn't know this task is finished
                    */
                //if character carries resource, then he just carries
                    //until allocate/store task is assigned to him
                    //check if woodpacks in bag are used up
                } else if (carriage.current_storage == 0) {
                    std::cout << "all woodpacks in bag are placed" << std::endl;
                    finish_current_action(entity);

                    /* to delete
                    auto& task = component_manager_.get_component<TaskComponent>(entity).current_task;
                    task.finished = true;
                    //THIS IS IMPORTANT, or tasksystem doesn't know this task is finished
                    */
                    return;
                }
            }

            if (carriage.current_storage == 0) {
                std::cout << "character has no resource in hand" << std::endl;
            }
            if (storage.current_storage >= storage.storage_capacity) {
                std::cout << "already allocated!" << std::endl;
            }
            
        }
        //draw resource! based on current_storage displaying amount
    }

    //entity is character
    void build(Entity entity) {
        std::cout << "entity " << entity << " build" << std::endl;
        auto& action = component_manager_.get_component<ActionComponent>(entity).current_action;
        auto blueprint = action.target_entity;
        std::cout << "build target: " << blueprint << std::endl;
        auto& track = component_manager_.get_component<TargetComponent>(blueprint);
        track.progress += 100 / (action.duration * framerate_);
        std::cout << "build progress:" << track.progress << std::endl;
        if (track.progress >= 100) {
            //blueprint not target, won't be added to task queue
            track.is_finished = true;

            finish_current_action(entity);

            auto entity_info = router_.printer(entity);
            std::cout << "entity info:\ntype: " << entity_info.first << "\npos: " << entity_info.second.x << ", " << entity_info.second.y << std::endl;
            auto info = router_.printer(blueprint);
            std::cout << "blueprint info:\ntype: " << info.first << "\npos: " << info.second.x << ", " << info.second.y << std::endl;
            /*
            if (component_manager_.has_component<CollisionComponent>(blueprint)) {
                component_manager_.get_component<CollisionComponent>(blueprint).collidable = true;
            } else {
                component_manager_.add_component(blueprint, CollisionComponent{true});
            }

            if (component_manager_.has_component<TaskComponent>(blueprint)) {
                component_manager_.remove_component<TaskComponent>(blueprint);
            }//not sure if this is necessary
            */

            if(component_manager_.has_component<ConstructionComponent>(blueprint)) {
                auto& construction = component_manager_.get_component<ConstructionComponent>(blueprint);
                construction.is_built = true;
                std::cout << "blueprint " << blueprint << " is built" << std::endl;
            } else {
                std::cout << "WRONG blueprint doesn't have construction component" << std::endl;
                component_manager_.add_component(blueprint, ConstructionComponent{.allocated = true, .is_built = true});
            }

            //remove woods on the site
            auto& storage = component_manager_.get_component<StorageComponent>(blueprint);
            while(!storage.stored_resources.empty()) {
                //delete woodpack from site
                auto& woodpack = storage.stored_resources.front();
                storage.stored_resources.pop_front();

                //delete this woodpack entity
                if (component_manager_.has_component<TargetComponent>(woodpack)) {
                    component_manager_.get_component<TargetComponent>(woodpack).to_be_deleted = true;
                }
            }
            std::cout << "all woodpacks on site are deleted" << std::endl;
        }
    }

};  