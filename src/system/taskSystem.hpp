#pragma once
#include "utils/path.hpp"
#include <queue>
#include <algorithm>
class TaskSystem {
    ComponentManager& component_manager_;
    EntityManager& entity_manager_;
    std::vector<std::vector<Entities>>& entity_map_;
    Router& router_;
    std::vector<Task> task_queue_;
    std::vector<Task> task_in_progress_;
    int max_distance_ = entity_map_.size() * entity_map_.size();
public:
    TaskSystem();
    TaskSystem(ComponentManager& component_manager, EntityManager& entity_manager, Router& router, std::vector<std::vector<Entities>>& entity_map) : 
        component_manager_(component_manager),
        entity_manager_(entity_manager),
        entity_map_(entity_map),
        router_(router) {
        task_queue_.clear();
        std::cout << "TaskSystem initialized with empty task queue" << std::endl;
       
    }

    void update() {
        std::cout << "TaskSystem updating" << std::endl;
        update_task_queue();
        assign_task();
    }

    //update queue based on existed tasks, which are given by world
    void update_task_queue() {
        std::cout << "TaskSystem updating task queue" << std::endl;

         //add back all tasks that are not feasible
        for(auto& entity : router_.get_entities_with_components<TaskComponent>()) {
            
            if (!component_manager_.has_component<TaskComponent>(entity)) {
                std::cout << "wrong! current entity has no task component" << std::endl;
                continue;
            }

            std::cout << "current entity type: " << router_.printer(entity).first << " ,location: (" << router_.printer(entity).second.x << ", " << router_.printer(entity).second.y << ")" << std::endl;
            auto& cur_task = component_manager_.get_component<TaskComponent>(entity).current_task;

            if (cur_task.type == TaskType::IDLE) {
                std::cout << "this entity is idling currently" << std::endl;
                continue;
            }

            if (!cur_task.feasible) {
                std::cout << "a task is not feasible" << std::endl;
                if (cur_task.type != TaskType::IDLE) {
                    task_queue_.emplace_back(cur_task);
                }
                cur_task = idle_task();
            }
            std::cout << "checkpoint: try remove completed tasks" << std::endl;
            //remove all completed tasks
            auto& target = component_manager_.get_component<TaskComponent>(entity).current_task.target_action.target_entity;
            router_.printer(target);
            if (!component_manager_.has_component<TargetComponent>(target)) {
                std::cout << "current entity with task has no target component" << std::endl;
                continue;
            }

            auto& track = component_manager_.get_component<TargetComponent>(target);
            if (track.is_finished) {
                std::cout << "a task is completed" << std::endl;
                component_manager_.remove_component<TargetComponent>(target);
                cur_task = idle_task();
            }
        }//space: entity with task component(character or animal)

        //std::cout << "checkpoint1: add new tasks into queue" << std::endl;
        
        //add new tasks into queue
        for (auto& target_entity : router_.get_entities_with_components<TargetComponent>()) {
            if (target_in_queue(target_entity)) {
                std::cout << "target is already in queue" << std::endl;
                continue;
            }

            auto& track = component_manager_.get_component<TargetComponent>(target_entity);
            if (!track.is_target) {
                continue;
            }
            auto tmp_type = router_.printer(target_entity).first;
            auto tmp_loc = router_.printer(target_entity).second;
            Task new_task = empty_task();

            if (!component_manager_.has_component<RenderComponent>(target_entity)) {
                std::cout << "wrong! current entity has no render component" << std::endl;
                continue;
            }

            
            auto render = component_manager_.get_component<RenderComponent>(target_entity);
            std::cout << "new task, target: " << target_entity;
            std::cout << " Target type: " << tmp_type << ", Location: (" << tmp_loc.x << ", " << tmp_loc.y << ")" << std::endl;
            
            bool valid_task = false;
            //chop tree
            if (render.entityType == EntityType::TREE) {
                std::cout << "find new task: chop tree" << std::endl;
                new_task = chop_task(target_entity);
                valid_task = true;
            }
            //collect resource from tree
            else if (auto& resource = component_manager_.get_component<ResourceComponent>(target_entity);
                render.entityType == EntityType::WOODPACK && resource.holder == -1) {
                std::cout << "find new task: collect resource" << std::endl;
                new_task = collect_task(target_entity);
                valid_task = true;
            }
            else if ((render.entityType == EntityType::WALL || render.entityType == EntityType::DOOR)) {
                if(!component_manager_.get_component<ConstructionComponent>(target_entity).allocated
                    && !component_manager_.get_component<ConstructionComponent>(target_entity).is_built) {
                    //allocate resource to blueprint
                    std::cout << "find new task: allocate resource" << std::endl;
                    new_task = allocate_task(target_entity);
                    valid_task = true;
                } else if (component_manager_.get_component<ConstructionComponent>(target_entity).allocated
                    && !component_manager_.get_component<ConstructionComponent>(target_entity).is_built) {
                    //construct blueprint
                    std::cout << "find new task: construct blueprint" << std::endl;
                    new_task = construct_task(target_entity);
                    valid_task = true;
                }
            }
            //store resource to storage area
            else if (render.entityType == EntityType::STORAGE) {
                std::cout << "find new task: store resource" << std::endl;
                new_task = store_task(target_entity);
                valid_task = true;
            }
            //obtain task is only called when there is a allocate task
            
            if (valid_task) {
                task_queue_.emplace_back(new_task);
            } else {
                std::cout << "invalid task!" << std::endl;
            }
        }//space: entity with target component

        //std::cout << "checkpoint2: add idle task into queue" << std::endl;

        //std::cout << "checkpoint3: sort tasks by priority" << std::endl;
        //sort tasks by priority
        std::sort(task_queue_.begin(), task_queue_.end(), [](const Task& a, const Task& b) {
            return a.priority > b.priority;
        });
    }
    
    void assign_task() {
        std::cout << "assigning tasks" << std::endl;
        for(auto character : router_.get_characters()) {
            bool is_assigned_task = false;
            if (!component_manager_.has_component<TaskComponent>(character)) {
                std::cout << "add task component to character" << std::endl;
                component_manager_.add_component(character, TaskComponent{
                    .current_task = idle_task(),
                    .next_task = idle_task()
                });
            }

            auto& character_task = component_manager_.get_component<TaskComponent>(character);
            
            //keep task at first
            if (character_task.current_task.type == TaskType::IDLE 
                && character_task.next_task.type != TaskType::IDLE) {
                character_task.current_task = character_task.next_task;
                character_task.next_task = idle_task();
            }

            if(character_task.current_task.type != TaskType::IDLE) {
                continue;
            }

            if (!task_queue_.empty()) {
                std::cout << "checkpoint: task queue is not empty" << std::endl;
            }

            //only assign task if current task is available
            std::cout << "task queue has tasks: " << task_queue_.size() << std::endl;
            for(auto task = task_queue_.begin(); task != task_queue_.end();) {
                if (task->type == TaskType::IDLE) {
                    std::cout << "it is an idle task" << std::endl;
                    continue;
                }

                //attention! target_pos is a field, not location
                auto targets = task->target_locations; 
                auto character_pos = component_manager_.get_component<LocationComponent>(character).loc;
                auto& character_tasks = component_manager_.get_component<TaskComponent>(character);
                std::cout << "character is at (" << character_pos.x << ", " << character_pos.y << ")" << std::endl;
                std::cout << "task is at (" << targets[0].x << ", " << targets[0].y << ")" << std::endl;
                
                if (router_.find_path_to_locations(character_pos, targets).size() != 0) {
                    std::cout << "reachable!" << std::endl;
                    if (Entity dst;
                        task->type == TaskType::ALLOCATE && find_resource_destination(character, dst)) {
                        //attention: allocate requeires an extra task: collect, which is not from task_queue_
                        //only if character can find a storage area to obtain resources
                        //will he carry out the allocate task
                        character_tasks.current_task = obtain_task(dst); //obtain resources from dst(storage area)
                        character_tasks.next_task = *task;
                        task = task_queue_.erase(task);
                        is_assigned_task = true;
                        break;
                    } else if (task->type != TaskType::ALLOCATE) {
                        character_tasks.current_task = *task;
                        task = task_queue_.erase(task);
                        is_assigned_task = true;
                        break;
                        //character can allocate resource if he carries resource
                    } else if (task->type == TaskType::ALLOCATE && character_carries_resource(character)) {
                        character_tasks.current_task = *task;
                        task = task_queue_.erase(task);
                        is_assigned_task = true;
                        break;
                    } else {
                        ++task;
                        continue;
                    }
                } else {
                    std::cout << "unreachable!" << std::endl;
                    ++task;
                    continue;
                }
            }

            if (!is_assigned_task) {//nothing to do, just idle
                character_task.current_task = idle_task();
                character_task.next_task = idle_task();
            }

            //animals are always idling
            for(auto& animal : router_.get_animals()) {
                if (!component_manager_.has_component<TaskComponent>(animal)) {
                    std::cout << "add task component to animal" << std::endl;
                    component_manager_.add_component(animal, TaskComponent{idle_task()});
                }
                component_manager_.get_component<TaskComponent>(animal).current_task = idle_task();
                component_manager_.get_component<TaskComponent>(animal).next_task = idle_task();
            }
        }
    }

    Task empty_task();

    //entity: 
    Task idle_task();

    //entity: tree to be chopped(marked)
    Task chop_task(Entity entity);

    //entity: resouces to be collected
    Task collect_task(Entity entity);

    //entity: storage area(units) to obtain resources
    Task obtain_task(Entity entity);
    
    //entity: storage area(units) to store resources
    Task store_task(Entity entity);
    
    //entity: blueprint to be allocated resource
    Task allocate_task(Entity entity);

    //entity: blueprint to be built
    Task construct_task(Entity entity);

    

private:
    bool character_carries_resource(Entity entity) {
        auto& bag = component_manager_.get_component<StorageComponent>(entity);
        return bag.current_storage > 0;
    }

    bool find_resource_destination(Entity target, Entity& dst) {
        bool path_exists = false;
        int distance = max_distance_;
        for (auto& entity : router_.get_placeable_areas()) {
            //find storage or blueprint
            auto& src_pos = component_manager_.get_component<LocationComponent>(target).loc;
            auto& dst_pos = component_manager_.get_component<LocationComponent>(entity).loc;
            if (router_.find_path(src_pos, dst_pos).size() != 0) {
                if (!path_exists) {
                    path_exists = true;
                    distance = router_.calculate_distance(src_pos, dst_pos);
                    dst = entity;
                } else if (router_.calculate_distance(src_pos, dst_pos) < distance) {
                    distance = router_.calculate_distance(src_pos, dst_pos);
                    dst = entity;
                }
            }
        }
        return path_exists;
    }

    bool target_in_queue(Entity entity) {
        for (auto& task : task_queue_) {
            if (task.target_action.target_entity == entity) {
                return true;
            }
        }
        return false;
    }
};

Task TaskSystem::empty_task() {
    return Task{
        .type = TaskType::EMPTY, 
        .target_locations = {Location{-1,-1}}, 
        .priority = -1, 
        .target_action = ActionType::NONE};
}

Task TaskSystem::idle_task() {
    auto pos = Location{-1, -1};
    return Task{
        .type = TaskType::IDLE, 
        .target_locations = {pos},
        .priority = 0, 
        .target_action = Action{
            ActionType::MOVE, pos, 0, -1
        },
        .feasible = true
    };
}


Task TaskSystem::chop_task(Entity entity) {
    auto& pos = component_manager_.get_component<LocationComponent>(entity).loc;

    return Task{
        .type = TaskType::CHOP_WOOD, 
        .target_locations = router_.get_locations_around(pos),
        .priority = 6, 
        .target_action = Action{
            ActionType::CHOP, pos, 5.0, entity
        },
        .feasible = true
    };
}

Task TaskSystem::collect_task(Entity entity) {
    auto& pos = component_manager_.get_component<LocationComponent>(entity).loc;
    
    return Task{
        .type = TaskType::COLLECT, 
        .target_locations = {pos},
        .priority = 5, 
        .target_action = Action{
            ActionType::PICK, pos, 1.0, entity
        },
        .feasible = true
    };
}

Task TaskSystem::obtain_task(Entity entity) {
    auto& pos = component_manager_.get_component<LocationComponent>(entity).loc;
    return Task{
        .type = TaskType::OBTAIN, 
        .target_locations = {pos},
        .priority = 3, 
        .target_action = Action{
            ActionType::PICK, pos, 1.0, entity
        },
        .feasible = true
    };
}
    
Task TaskSystem::store_task(Entity entity) {
    auto& pos = component_manager_.get_component<LocationComponent>(entity).loc;

    return Task{
        .type = TaskType::STORE, 
        .target_locations = router_.get_locations_around(pos),
        .priority = 3, 
        .target_action = Action{
            ActionType::PLACE, pos, 1.0, entity
        },
        .feasible = true
    };
}
    
Task TaskSystem::allocate_task(Entity entity) { 
    auto& pos = component_manager_.get_component<LocationComponent>(entity).loc;
    
    return Task{
        .type = TaskType::ALLOCATE, 
        .target_locations = router_.get_locations_around(pos),
        .priority = 4, 
        .target_action = Action{
            ActionType::PLACE, pos, 1.0, entity
        },
        .feasible = true
    };
}

Task TaskSystem::construct_task(Entity entity) {
    auto& pos = component_manager_.get_component<LocationComponent>(entity).loc;
    
    return Task{
        .type = TaskType::CONSTRUCT, 
        .target_locations = router_.get_locations_around(pos),
        .priority = 6, 
        .target_action = Action{
            ActionType::BUILD, pos, 5.0, entity
        },
        .feasible = true
    };
}