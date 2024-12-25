#pragma once
#include "utils/path.hpp"
#include <queue>
#include <algorithm>
class TaskSystem {
    ComponentManager& component_manager_;
    EntityManager& entity_manager_;
    Router& router_;
    std::vector<Task> task_queue_;
    std::vector<Task> task_in_progress_;
    int map_size_;
    int max_distance_ = map_size_ * map_size_;
public:
    TaskSystem();
    TaskSystem(ComponentManager& component_manager, EntityManager& entity_manager, Router& router) : 
        component_manager_(component_manager),
        entity_manager_(entity_manager),
        router_(router) {
        map_size_ = router_.get_map_size();
        task_queue_.clear();
        std::cout << "TaskSystem initialized with empty task queue" << std::endl;
        id = 0;
    }

    void update() {
        std::cout << "TaskSystem updating" << std::endl;
        update_storage();
        update_task_queue();
        assign_task();
    }

    //update queue based on existed tasks, which are given by world
    void update_task_queue() {
        std::cout << "TaskSystem updating task queue" << std::endl;
        
        //try add back all task that are unfeasible
        for(auto& entity : router_.get_entities_with_components<TaskComponent>()) {
            //std::cout << "current entity type: " << router_.printer(entity).first << " ,location: (" << router_.printer(entity).second.x << ", " << router_.printer(entity).second.y << ")" << std::endl;
            auto& cur_task = component_manager_.get_component<TaskComponent>(entity).current_task;

            //idle task won't be added back to queue
            if (cur_task.type == TaskType::IDLE) {
                continue;
            }

            //obtain task won't be added back to queue, because it is bound to an allocate task
            if (cur_task.type == TaskType::OBTAIN && !cur_task.feasible) {
                std::cout << "can not obtain!" << std::endl;

                auto& next_task = component_manager_.get_component<TaskComponent>(entity).next_task;
                //delete obtain task from this character
                cur_task = idle_task();

                //add back allocate task(next) to queue
                task_queue_.emplace_back(next_task);
                remove_task_from_progress(next_task);
                next_task = idle_task();
                
                continue;
            }

            //add back all tasks that are not feasible
            if (!cur_task.feasible) {
                std::cout << "a task is not feasible" << std::endl;
                if (cur_task.type != TaskType::IDLE) {
                    task_queue_.emplace_back(cur_task);
                    remove_task_from_progress(cur_task);
                    std::cout << "this task is added back to queue" << std::endl;
                }
                cur_task = idle_task();
            }
            
            //delete task if target entity is no longer a target
            if (cur_task.target_action.target_entity != -1) {
                auto& target = cur_task.target_action.target_entity;
                if (!component_manager_.has_component<TargetComponent>(target) || 
                    !component_manager_.get_component<TargetComponent>(target).is_target) {
                    std::cout << "target entity is no longer a target" << std::endl;
                    cur_task = idle_task();
                    remove_task_from_progress(cur_task);
                }
            }

            //Print target entity info of character's current task
            auto& target = component_manager_.get_component<TaskComponent>(entity).current_task.target_action.target_entity;
            auto tmp_type = router_.printer(target).first;
            auto tmp_loc = router_.printer(target).second;
            std::cout << "target type: " << tmp_type << ", target location: (" << tmp_loc.x << ", " << tmp_loc.y << ")" << std::endl;
            if (!component_manager_.has_component<TargetComponent>(target)) {
                std::cout << "current entity with task has no target component" << std::endl;
                continue;
            }
        }//space: entity with task component(character or animal)
        
        //remove all completed tasks
        std::cout << "try: remove finished target/task" << std::endl;
        remove_finished_task();

        //add new tasks into queue
        for (auto& target_entity : router_.get_entities_with_components<TargetComponent>()) {
            if (target_in_queue(target_entity)) {
                std::cout << "target is already in queue" << std::endl;
                continue;
            }

            if (is_target_in_progress(target_entity)) {
                std::cout << "target is already in progress" << std::endl;
                continue;
            }

            if (component_manager_.get_component<TargetComponent>(target_entity).to_be_deleted) {
                std::cout << "target entity: " << target_entity << " is to be deleted" << std::endl;
                continue;
            }

            auto& track = component_manager_.get_component<TargetComponent>(target_entity);
            if (!track.is_target || track.is_finished) {
                //std::cout << "target entity: " << target_entity << " is not a target" << std::endl;
                continue;
            }

            auto tmp_type = router_.printer(target_entity).first;
            auto tmp_loc = router_.printer(target_entity).second;
            Task new_task = empty_task();

            if (!component_manager_.has_component<RenderComponent>(target_entity)) {
                std::cout << "wrong! current entity has no render component" << std::endl;
                continue;
            }

            if(component_manager_.has_component<ResourceComponent>(target_entity)) {
                auto& resource = component_manager_.get_component<ResourceComponent>(target_entity);
                auto& holder = resource.holder;
                if (holder != -1 && component_manager_.has_component<RenderComponent>(holder)) {
                    auto& hold_type = component_manager_.get_component<RenderComponent>(holder).entityType;
                    //if woods are held by character, door or wall, it is not a target
                    if (hold_type == EntityType::CHARACTER || hold_type == EntityType::DOOR || 
                        hold_type == EntityType::WALL || hold_type == EntityType::STORAGE) {
                        continue;
                    }
                }
            }
            
            auto target_type = component_manager_.get_component<RenderComponent>(target_entity).entityType;
            std::cout << "new task, target: " << target_entity;
            std::cout << " Target type: " << target_type << ", Location: (" << tmp_loc.x << ", " << tmp_loc.y << ")" << std::endl;
            
            bool valid_task = false;
            //chop tree
            if (target_type == EntityType::TREE) {
                std::cout << "find new task: chop tree" << std::endl;
                new_task = chop_task(target_entity);
                valid_task = true;
            }
            //collect resource from tree
            else if (target_type == EntityType::WOODPACK) {
                auto& resource = component_manager_.get_component<ResourceComponent>(target_entity);
                if (resource.holder == -1) {
                    std::cout << "find new task: collect resource" << std::endl;
                    new_task = collect_task(target_entity);
                    valid_task = true;
                }
            }
            else if (target_type == EntityType::WALL || target_type == EntityType::DOOR) {

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
            else if (target_type == EntityType::STORAGE) {
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
        std::cout << "task wait for assign: " << task_queue_.size() << std::endl;

        if (!task_queue_.empty()) {
            auto& head = task_queue_.front();
            head.print_task_info();
        }

        std::cout << "task in progress: " << task_in_progress_.size() << std::endl;
        if (!task_in_progress_.empty()) {
            auto& head = task_in_progress_.front();
            head.print_task_info();
        }
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
            
            //if character is moving, skip this round(avoid flash)
            if (!router_.is_move_finished(character)) {
                std::cout << "entity is moving, do not assign task this round" << std::endl;
                continue;
            }

            /*
            //ATTTENTION to this: could lead IDLE overload real task
            if(component_manager_.has_component<ActionComponent>(character)) {
                auto& action = component_manager_.get_component<ActionComponent>(character);
                if (action.action_finished) {
                    character_task.current_task = idle_task();
                    character_task.next_task = idle_task();
                }
            }
            */

            //keep task at first
            //attention: while character is moving, assign a new move task would lead to flash
            if (character_task.current_task.type == TaskType::IDLE 
                && character_task.next_task.type != TaskType::IDLE) {
                character_task.current_task = character_task.next_task;
                character_task.next_task = idle_task();
            }

            if(character_task.current_task.type != TaskType::IDLE) {
                continue;
            }

            //only assign task if current task is available
            std::cout << "task queue has tasks: " << task_queue_.size() << std::endl;
            for(auto task = task_queue_.begin(); task != task_queue_.end();) {

                //attention! target_pos is a set of locations
                auto targets = task->target_locations; 
                auto character_pos = component_manager_.get_component<LocationComponent>(character).loc;
                auto& character_tasks = component_manager_.get_component<TaskComponent>(character);
                std::cout << "character is at (" << character_pos.x << ", " << character_pos.y << ")" << std::endl;
                std::cout << "task is at (" << targets[0].x << ", " << targets[0].y << ")" << std::endl;
                
                if (router_.is_reachable(character_pos, targets)) {
                    //std::cout << "reachable!" << std::endl;
                    if ( (task->type == TaskType::ALLOCATE || task->type == TaskType::STORE)
                    && character_carries_resource(character)) {
                        task->actor = character;
                        character_tasks.current_task = *task;
                        task_in_progress_.emplace_back(*task);
                        task = task_queue_.erase(task);
                        is_assigned_task = true;
                        std::cout << "character has resource in hand, allocate or store resource" << std::endl;
                        break;
                    } else if (Entity dst; 
                    task->type == TaskType::ALLOCATE 
                    && !character_carries_resource(character)
                    && find_resource_destination(character, dst, 5)) {
                        Task obtain = obtain_task(dst);
                        task->actor = character;
                        character_tasks.current_task = obtain;
                        task_in_progress_.emplace_back(obtain);
                        is_assigned_task = true;
                        std::cout << "character has no resource, but find a storage area to obtain resources" << std::endl;
                        break;
                    }
                    else if (task->type != TaskType::ALLOCATE && task->type != TaskType::STORE) {
                        // chop/construct/collect
                        task->actor = character;
                        character_tasks.current_task = *task;
                        task_in_progress_.emplace_back(*task);
                        task = task_queue_.erase(task);
                        is_assigned_task = true;
                        std::cout << "found task, start to do it" << std::endl;
                        break;
                    } else {//this if includes that task is allocate but no resource to allocate
                        std::cout << "current task not assigned, go next" << std::endl;
                        ++task;
                        continue;
                    }
                } else {
                    std::cout << "unreachable task!" << std::endl;
                    ++task;
                    continue;
                }
            }

            //if no task is assigned, let character idle
            if (!is_assigned_task) {
                character_task.current_task = idle_task();
                character_task.next_task = idle_task();
            }

            router_.print_character_current_task(character);
        }
        std::cout << "After assign: task wait for assign: " << task_queue_.size() << ", task in progress: " << task_in_progress_.size() << std::endl;
        //animals are always idling
        for(auto& animal : router_.get_animals()) {
            if (!component_manager_.has_component<TaskComponent>(animal)) {
                std::cout << "add task component to animal" << std::endl;
                component_manager_.add_component(animal, TaskComponent{idle_task()});
            }
            if (!router_.is_move_finished(animal)) {
                continue;
            }
            component_manager_.get_component<TaskComponent>(animal).current_task = idle_task();
            component_manager_.get_component<TaskComponent>(animal).next_task = idle_task();
        }
    }


    void remove_corresponding_task_from_queue(Entity entity) {
        for(auto it = task_in_progress_.begin(); it != task_in_progress_.end();) {
            if (it->target_action.target_entity == entity) {
                it = task_in_progress_.erase(it);
            } else {
                ++it;
            }
        }

        for(auto it = task_queue_.begin(); it != task_queue_.end();) {
            if (it->target_action.target_entity == entity) {
                it = task_queue_.erase(it);
            } else {
                ++it;
            }
        }
    }

    void update_task_by_target(Entity entity) {
        for(auto& entity : router_.get_entities_with_components<TaskComponent>()) {
            auto& task = component_manager_.get_component<TaskComponent>(entity).current_task;
            if (task.target_action.target_entity == entity) {
                task = idle_task();
            }
        }
    }

    void remove_finished_task() {
        //search all entities with TARGET and TARGET is finished
        //(tree, woodpack just dropped, construction just built)
        //this search function will mark TARGET deleted if it is a TREE
        for(auto& entity : finished_target_entities_()) {
            //if entity appears as a target in the queues
            //remove the corresponding task
            std::cout << "ATTENTION: update finished target/task" << std::endl;
            std::cout << "this entity: " << entity << " is finished, removed from queue" << std::endl;
            remove_corresponding_task_from_queue(entity);
        }

        //isolately deal with entity: storage, blueprint
        //TASKS: STORE(PLACE in storage)
        //TASKS: OBTAIN(PICK from storage)
        //TASKS: ALLOCATE(ALLOCATE to blueprint)
        //for they are targets, but won't be marked as finished
        //traverse characters doing these tasks
        for(auto& character : router_.get_characters()) {
            if (!component_manager_.has_component<TaskComponent>(character)) {
                continue;
            }
            auto& task = component_manager_.get_component<TaskComponent>(character);
            auto& curTask = task.current_task;
            auto& nextTask = task.next_task;
           
            if(curTask.finished) {
                std::cout << "current task type: " << static_cast<int>(curTask.type) << " is finished" << std::endl;  
                
                //consider idle task
                if (curTask.type == TaskType::IDLE) {
                    continue;
                }

                //remove task from 2 queues by its id
                remove_task_from_progress(curTask);

                //make character idle after finish current task
                curTask = idle_task();

                //put next task into current
                if (nextTask.type != TaskType::IDLE && !nextTask.finished) {
                    curTask = nextTask;
                    nextTask = idle_task();
                } 
            }

            //ATTENION: it is a batch for multi characters
            //if blueprint is allocated or built, a character still carries resource on the way
            //remove this allocate task for him
            if (curTask.type == TaskType::ALLOCATE && curTask.feasible) {
                auto& blueprint = curTask.target_action.target_entity;
                if (component_manager_.has_component<ConstructionComponent>(blueprint)) {
                    auto& construction = component_manager_.get_component<ConstructionComponent>(blueprint);
                    if (construction.allocated || construction.is_built) {
                        remove_task_from_progress(curTask);
                        curTask = idle_task();
                    }
                }
            }

            //extra logic avoid repeated collect
            if (curTask.type == TaskType::COLLECT && curTask.feasible) {
                auto& woodpack = curTask.target_action.target_entity;
                if (component_manager_.has_component<ResourceComponent>(woodpack)) {
                    auto& resource = component_manager_.get_component<ResourceComponent>(woodpack);
                    if (resource.holder != -1) {
                        remove_task_from_progress(curTask);
                        curTask = idle_task();
                    }
                }
            }
        }
    }

    Entities finished_target_entities_() {
        Entities entities;
        for(auto& entity : router_.get_all_entities()) {
            if (component_manager_.has_component<TargetComponent>(entity)) {
                auto& target = component_manager_.get_component<TargetComponent>(entity);
                if (target.is_finished && target.is_target) {
                    std::cout << "find finished target: " << entity << std::endl;
                    entities.emplace_back(entity);

                    //if target is a tree, it is finished
                    //it will be deleted
                    auto& type = component_manager_.get_component<RenderComponent>(entity).entityType;
                    if (type == EntityType::TREE) {
                        std::cout << "tree " << entity << " will be deleted" << std::endl;
                        component_manager_.get_component<TargetComponent>(entity).to_be_deleted = true;
                    } else if (type == EntityType::WOODPACK) {
                        //woods are no longer target once they are picked up
                        //it will be transferred as sub-resource of entity
                        //like storage, character, construction
                        component_manager_.get_component<TargetComponent>(entity).is_target = false;
                    }
                }
            }
        }
        return entities;
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
    //every task has a unique id except for idle task
    int id;

    void update_storage() {
        bool any_to_store = false;
        for(auto& character : router_.get_characters()) {
            //ensure character carries some woods and not on the way to allocate
            //or a stora task maybe wrongly generated
            bool to_store = character_carries_resource(character) && !character_is_allocating(character);
            any_to_store |= to_store;
            if (to_store) {
                Entity dst_storage;
                if (find_resource_destination(character, dst_storage, 0)) {

                    //storage area is already a target
                    if (component_manager_.has_component<TargetComponent>(dst_storage)) {
                        auto& target = component_manager_.get_component<TargetComponent>(dst_storage);
                        target.is_target = true;
                        continue;
                    }

                    //add target component
                    TargetComponent target{
                        .progress = 0,
                        .timer = 0,
                        .is_target = true,
                        .is_finished = false,
                        .to_be_deleted = false,
                        .hold_by = -1
                    };
                    component_manager_.add_component(dst_storage, target);
                }
                //if no character has resource to store, storage areas are no long targets
                //thus there will not generate STORAGE task
            } else if (!any_to_store) {
                for(auto& storage : router_.get_storage_areas()) {
                    if (component_manager_.has_component<TargetComponent>(storage)) {
                        auto& target = component_manager_.get_component<TargetComponent>(storage);
                        target.is_target = false;
                    }
                }
            }
        }
    }

    //here entity is the task's target entity
    bool is_target_in_progress(Entity entity) {
        for (auto& task : task_in_progress_) {
            auto& target = task.target_action.target_entity;
            if (target == entity) {
                return true;
            }
        }
        return false;
    }   

    //remove task from task_in_progress_ by its id
    //FOR those tasks that could not be recognized FINISHED by target entity
    //like allocate/store/obtain
    void remove_task_from_progress(Task task) {
        for(auto task = task_in_progress_.begin(); task != task_in_progress_.end();) {
            if (task->id == task -> id) {
                std::cout << "task id: " << task->id << " is removed from task_in_progress_" << std::endl;
                task = task_in_progress_.erase(task);
            } else {
                ++task;
            }
        }
    }

    void remove_task_from_queue(Task task) {
        for(auto task = task_queue_.begin(); task != task_queue_.end();) {
            if (task->id == task -> id) {
                task = task_queue_.erase(task);
            } else {
                ++task;
            }
        }
    }

    //entity is a character
    bool character_carries_resource(Entity entity) {
        assert(component_manager_.has_component<StorageComponent>(entity));
        //chracter always has a bag
        auto& bag = component_manager_.get_component<StorageComponent>(entity);
        return bag.current_storage > 0;
    }

    bool character_is_allocating(Entity entity) {
        if (!component_manager_.has_component<TaskComponent>(entity)) {
            return false;
        }
        auto& task = component_manager_.get_component<TaskComponent>(entity).current_task;
        return task.type == TaskType::ALLOCATE;
    }

    bool find_resource_destination(Entity target, Entity& dst, int resource_amount) {
        //std::cout << "try find resource destination" << std::endl;
        bool path_exists = false;
        int distance = max_distance_;
        Location final_dst;
        for (auto& entity : router_.get_storage_areas()) {
            //find storage or blueprint
            auto src_pos = component_manager_.get_component<LocationComponent>(target).loc;
            auto dst_pos = component_manager_.get_component<LocationComponent>(entity).loc;
            auto& storage = component_manager_.get_component<StorageComponent>(entity);
            if ( router_.find_path(src_pos, dst_pos).size() != 0 
            && storage.current_storage < storage.storage_capacity
            && ( resource_amount == 0 || resource_amount > 0 && storage.current_storage > 0 ) ) {
                if (!path_exists) {
                    path_exists = true;
                    distance = router_.calculate_distance(src_pos, dst_pos);
                    dst = entity;
                } else if (router_.calculate_distance(src_pos, dst_pos) < distance) {
                    distance = router_.calculate_distance(src_pos, dst_pos);
                    dst = entity;
                    final_dst = dst_pos;
                }
            }
        }
        if (path_exists) {
            std::cout << "find storage " << dst << " at (" << final_dst.x << ", " << final_dst.y << ")" << std::endl;
        }else{
            std::cout << "no resource destination found" << std::endl;
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
        .feasible = true,
        .id = -1
    };
}


Task TaskSystem::chop_task(Entity entity) {
    auto& pos = component_manager_.get_component<LocationComponent>(entity).loc;

    return Task{
        .type = TaskType::CHOP_WOOD, 
        .target_locations = router_.get_locations_around(pos),
        .priority = 3, 
        .target_action = Action{
            ActionType::CHOP, pos, 2.0, entity
        },
        .feasible = true,
        .id = ++id
    };
}

Task TaskSystem::collect_task(Entity entity) {
    auto& pos = component_manager_.get_component<LocationComponent>(entity).loc;
    
    return Task{
        .type = TaskType::COLLECT, 
        .target_locations = {pos},
        .priority = 99, 
        .target_action = Action{
            ActionType::PICK, pos, 1.0, entity
        },
        .feasible = true,
        .id = ++id
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
        .feasible = true,
        .id = ++id
    };
}
    
Task TaskSystem::store_task(Entity entity) {
    auto& pos = component_manager_.get_component<LocationComponent>(entity).loc;

    return Task{
        .type = TaskType::STORE, 
        .target_locations = router_.get_locations_around(pos),
        .priority = 50,//always consider storing first 
        .target_action = Action{
            ActionType::PLACE, pos, 1.0, entity
        },
        .feasible = true,
        .id = ++id
    };
}
    
Task TaskSystem::allocate_task(Entity entity) { 
    auto& pos = component_manager_.get_component<LocationComponent>(entity).loc;
    
    return Task{
        .type = TaskType::ALLOCATE, 
        .target_locations = router_.get_locations_around(pos),
        .priority = 5, 
        .target_action = Action{
            ActionType::PLACE, pos, 1.0, entity
        },
        .feasible = true,
        .id = ++id
    };
}

Task TaskSystem::construct_task(Entity entity) {
    auto& pos = component_manager_.get_component<LocationComponent>(entity).loc;
    
    return Task{
        .type = TaskType::CONSTRUCT, 
        .target_locations = router_.get_locations_around(pos),
        .priority = 7, 
        .target_action = Action{
            ActionType::BUILD, pos, 5.0, entity
        },
        .feasible = true,
        .id = ++id
    };
}