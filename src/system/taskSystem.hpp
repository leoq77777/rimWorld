#pragma once
#include "world/world.hpp"
#include "components/componentManager.hpp"
#include "entities/entity.hpp"
#include <queue>
#include <algorithm>
class TaskSystem {
public:
    TaskSystem() = default;
    TaskSystem(World& new_world) {
        world_ = new_world;
        entity_manager_ = world_.entity_manager_;
        component_manager_ = world_.component_manager_;
        std::cout << "TaskSystem initialized" << std::endl;
    }

    Task empty_task() {
        return Task{TaskType::EMPTY, {-1, -1}, -1, ActionType::NONE};
    }

    //entity: 
    Task idle_task() {
        auto& pos = component_manager.get_component<locationComponent>(entity).loc;
        return Task{
            .type = TaskType::IDLE, 
            .target_location = {pos, pos},
            .priority = 0, 
            .target_action = Action{
                ActionType::WANDER, pos, 0, entity
            }
        };
    }

    //entity: tree to be chopped(marked)
    Task chop_task(Entity entity) {
        auto& pos = component_manager.get_component<locationComponent>(entity).loc;
        
        return Task{
            .type = TaskType::CHOP, 
            .target_location = get_locations_around(pos),
            .priority = 6, 
            .target_action = Action{
                ActionType::CHOP, pos, 5.0, entity
            }
        };
    }

    //entity: resouces to be collected
    Task collect_task(Entity entity) {
        auto& pos = component_manager.get_component<locationComponent>(entity).loc;
        
        return Task{
            .type = TaskType::COLLECT, 
            .target_location = std::vector<location>{pos},
            .priority = 5, 
            .target_action = Action{
                ActionType::PICK, pos, 1.0, entity
            }
        };
    }

    //entity: storage area(units) to obtain resources
    Task obtain_task(Entity entity) {
        auto& pos = component_manager.get_component<locationComponent>(entity).loc;
        return Task{
            .type = TaskType::OBTAIN, 
            .target_location = std::vector<location>{pos},
            .priority = 3, 
            .target_action = Action{
                ActionType::PICK, pos, 1.0, entity
            }
        };
    }
    
    //entity: storage area(units) to store resources
    Task store_task(Entity entity) {
        auto& pos = component_manager.get_component<locationComponent>(entity).loc;
        
        return Task{
            .type = TaskType::PLACE, 
            .target_location = get_locations_around(pos),
            .priority = 3, 
            .target_action = Action{
                ActionType::PLACE, pos, 1.0, entity
            }
        };
    }
    
    //entity: blueprint to be allocated resource
    Task allocate_task(Entity entity) { 
        auto& pos = component_manager.get_component<locationComponent>(entity).loc;
        
        return Task{
            .type = TaskType::ALLOCATE, 
            .target_location = get_locations_around(pos),
            .priority = 4, 
            .target_action = Action{
                ActionType::PLACE, pos, 1.0, entity
            }
        };
    }

    //entity: blueprint to be built
    Task construct_task(Entity entity) {
        auto& pos = component_manager.get_component<locationComponent>(entity).loc;
        
        return Task{
            .type = TaskType::COLLECT, 
            .target_location = get_locations_around(pos),
            .priority = 6, 
            .target_action = Action{
                ActionType::BUILD, pos, 5.0, entity
            }
        };
    }

    //update queue based on existed tasks, which are given by world
    void update_task_queue(float dt) {
         //add back all tasks that are not feasible
        for(auto& entity : world_.get_entities_with_components<TaskComponent>()) {
            auto& cur_task = component_manager.get_component<TaskComponent>(entity).current_task;
            if (!cur_task.feasible) {
                task_queue_.emplace_back(cur_task);
                cur_task = idle_task();
            }

            //remove all completed tasks
            auto& target = component_manager.get_component<TaskComponent>(entity).current_task.target_action.target_entity;
            auto& track = component_manager.get_component<TargetComponent>(target);
            if (track.is_finished) {
                component_manager.remove_component<TargetComponent>(target);
                cur_task = idle_task();
            }
        }

        //add new tasks into queue
        for (auto& target_entity : world_.get_entities_with_components<TargetComponent>()) {
            if (target_in_queue(target_entity)) {
                continue;
            }
            auto& track = component_manager.get_component<TargetComponent>(target_entity);
            if (!track.is_target) {
                continue;
            }
            Task new_task = empty_task();
            auto render = component_manager.get_component<RenderComponent>(target_entity);
            //chop tree
            if (render.entityType == EntityType::TREE) {
                new_task = chop_task(target_entity);
            }
            //collect resource from tree
            else if (auto& resource = component_manager.get_component<ResourceComponent>(target_entity);
                render.entityType == EntityType::WOODPACK && resource.holder == -1) {
                new_task = collect_task(target_entity);
            }
            //allocate resource to blueprint
            else if ((render.entityType == EntityType::WALL || render.entityType == EntityType::DOOR)
                    && !component_manager.get_component<ConstructionComponent>(target_entity).allocated
                    && !component_manager.get_component<ConstructionComponent>(target_entity).is_built) {
                new_task = allocate_task(target_entity);
            }
            //construct blueprint allocated
            else if ((render.entityType == EntityType::WALL || render.entityType == EntityType::DOOR)
                    && component_manager.get_component<ConstructionComponent>(target_entity).allocated
                    && !component_manager.get_component<ConstructionComponent>(target_entity).is_built) {
                new_task = construct_task(target_entity);
            }
            //store resource to storage area
            else if (render.entityType == EntityType::STORAGE) {
                new_task = store_task(target_entity);
            }
            //obtain task is only called when there is a allocate task
            task_queue_.emplace_back(new_task);
        }

        //if no task, add an idle task
        if (task_queue_.empty()) {
            task_queue_.emplace_back(idle_task());
        }

        //sort tasks by priority
        std::sort(task_queue_.begin(), task_queue_.end(), [](const Task& a, const Task& b) {
            return a.priority > b.priority;
        });
    }
    
    void assign_task() {
        for(auto character : world_.characters_) {
            bool is_assigned_task = false;
            auto& character_task = component_manager.get_component<TaskComponent>(character);
            
            //keep task at first
            if (character_task.current_task.type == TaskType::IDLE 
                && character_task.next_task.type != TaskType::IDLE) {
                character_task.current_task = character_task.next_task;
                character_task.next_task = idle_task();
            }

            if(character_task.current_task.type != TaskType::IDLE) {
                continue;
            }

            //only assign task if current task is available
            for(auto task = task_queue_.begin(); task != task_queue_.end();) {
                //attention! target_pos is a field, not location
                auto targets = task->target_locations; 
                auto character_pos = component_manager.get_component<locationComponent>(character).loc;
                auto& character_tasks = component_manager.get_component<TaskComponent>(character);
                

                if (find_path_to_locations(character_pos, targets)) {
                    if (Entity dst;
                        task->type == TaskType::ALLOCATE && find_resource_destination(character, dst)) {
                        //attention: allocate requeires an extra task: collect, which is not from task_queue_
                        //only if character can find a storage area to obtain resources
                        //will he carry out the allocate task
                        character_tasks.current_task = obtain_task(dst); //obtain resources from dst(storage area)
                        character_tasks.next_task = task;
                        task = task_queue_.erase(task);
                        is_assigned_task = true;
                        break;
                    } else if (task->type != TaskType::ALLOCATE) {
                        character_tasks.current_task = task;
                        task = task_queue_.erase(task);
                        is_assigned_task = true;
                        break;
                        //character can allocate resource if he carries resource
                    } else if (task->type == TaskType::ALLOCATE && character_carries_resource(character)) {
                        character_tasks.current_task = task;
                        task = task_queue_.erase(task);
                        is_assigned_task = true;
                        break;
                    } else {
                        ++task;
                        continue;
                    }
                } else {
                    ++task;
                    continue;
                }
            }

            if (!is_assigned_task) {//nothing to do, just idle
                character_task.current_task = idle_task();
                character_task.next_task = idle_task();
            }

            for(auto& animal : world_.animals_) {
                //animals are always idling
                component_manager.get_component<TaskComponent>(animal).current_task = idle_task();
                component_manager.get_component<TaskComponent>(animal).next_task = idle_task();
            }
        }
    }

private:
    bool character_carries_resource(Entity entity) {
        auto& bag = component_manager.get_component<StorageComponent>(entity);
        return bag.current_storage > 0;
    }

    bool find_resource_destination(Entity target, Entity& dst) {
        bool path_exists = false;
        int distance = MAX_DISTANCE;
        for(auto& entity : world_.get_entities_with_components<StorageComponent>()) {
            //find storage or blueprint
            if (auto render_type = component_manager.get_component<RenderComponent>(entity).entityType; 
                ( render_type == EntityType::STORAGE ) || 
                ((render_type == EntityType::WALL || render_type == EntityType::DOOR) && !entity_manager.get_component<ConstructionComponent>(entity).allocated) 
                && find_path(target, entity) {
                if (!path_exists) {
                    path_exists = true;
                    distance = calculate_distance(target, entity);
                    dst = entity;
                } else if (calculate_distance(target, entity) < distance) {
                    distance = calculate_distance(target, entity);
                    dst = entity;
                }
        }
        return path_exists;
    }

    bool target_in_queue(Entity entity) {
        for (auto task : task_queue_) {
            if (task.target_action.target_entity == entity) {
                return true;
            }
        }
        return false;
    }

    World& world_;
    EntityManager& entity_manager_;
    ComponentManager& component_manager_;
    std::vector<Task> task_queue_;
};