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
    std::unordered_map<Entity, Entity> resource_bind;
    int map_size_;
    int max_distance_ = map_size_ * map_size_;
    int id_;
    void remove_task_from_progress_by_id(int id);
    void remove_task_from_queue_by_id(int id);
    void remove_allocate_task_assigned_to_character(Entity entity);
    bool character_carries_resource(Entity entity);
    bool target_entity_in_task_queue(Entity entity);
    bool is_target_in_progress(Entity entity);
    void remove_task_from_queue_by_target(Entity entity);
    void remove_finished_task();
    Entities get_finished_target_entities();
    Entity find_exist_obtain_target(Entity blueprint);
    void bind_resource_to_blueprint(Entity reso, Entity blueprint);
    bool is_resoure_bound_to_blueprint(Entity reso);
    void update_storage();
public:
    TaskSystem();
    TaskSystem(ComponentManager& component_manager, EntityManager& entity_manager, Router& router) : 
        component_manager_(component_manager),
        entity_manager_(entity_manager),
        router_(router) {
        map_size_ = router_.get_map_size();
        task_queue_.clear();
        std::cout << "TaskSystem initialized with empty task queue" << std::endl;
        id_ = 0;
    }

    void start_target_timer(Entity entity, float tick_time) {
        assert(component_manager_.has_component<TargetComponent>(entity));
        auto& target = component_manager_.get_component<TargetComponent>(entity);
        target.timer = tick_time;
    }

    bool is_target_available_at_moment(Entity entity) {
        assert(component_manager_.has_component<TargetComponent>(entity));
        auto& target = component_manager_.get_component<TargetComponent>(entity);
        return target.timer == 0.0;
    }

    void target_timer_tick(Entity entity) {
        assert(component_manager_.has_component<TargetComponent>(entity));
        auto& target = component_manager_.get_component<TargetComponent>(entity);
        target.timer--;
        if (target.timer < 0.0) target.timer = 0.0;
    }

    void update() {
        std::cout << "TaskSystem updating" << std::endl;
        remove_finished_task();
        update_task_queue();
        assign_task();
        update_storage();
    }

    //update queue based on existed tasks, which are given by world
    void update_task_queue() {
        std::cout << "TaskSystem updating task queue" << std::endl;

        //try add back all task that are unfeasible
        //and delete those tasks that target at non-target
        for(auto& entity : router_.get_entities_with_components<TaskComponent>()) {
            //std::cout << "current entity type: " << router_.printer(entity).first << " ,location: (" << router_.printer(entity).second.x << ", " << router_.printer(entity).second.y << ")" << std::endl;
            auto& cur_task = component_manager_.get_component<TaskComponent>(entity).current_task;

            //idle task won't be added back to queue
            if (cur_task.type == TaskType::IDLE)
                continue;

            //obtain task won't be added back to queue, because it is bound to an allocate task
            if (cur_task.type == TaskType::OBTAIN && !cur_task.feasible) {
                std::cout << "can not obtain!" << std::endl;
                //delete obtain task from this character
                cur_task = idle_task();
                remove_allocate_task_assigned_to_character(entity);
                continue;
            }

            //add back all tasks that are not feasible
            if (!cur_task.feasible) {
                std::cout << "a task is not feasible, will be added back to queue" << std::endl;
                remove_task_from_progress_by_id(cur_task.id);
                Task old = cur_task;
                old.feasible = true;
                old.actor = -1;
                cur_task = idle_task();
                task_queue_.emplace_back(old);
            }
            
            //delete task if target entity is no longer a target
            //here deletion should be considered both in task queue and character's task list
            if (cur_task.target_action.target_entity != -1) {
                auto& target = cur_task.target_action.target_entity;
                if (!component_manager_.has_component<TargetComponent>(target) || 
                    !component_manager_.get_component<TargetComponent>(target).is_target) {
                    std::cout << "target entity " << target <<" is no longer a target" << std::endl;
                    remove_task_from_progress_by_id(cur_task.id);
                    remove_task_from_queue_by_id(cur_task.id);
                    cur_task = idle_task();
                    continue;
                }
            }
        }//space: entity with task component(character or animal)
        
        //add new tasks into queue
        for (auto& target_entity : router_.get_entities_with_components<TargetComponent>()) {
            if (target_entity_in_task_queue(target_entity)) {
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
                std::cout << "target entity: " << target_entity << " is not a target" << std::endl;
                continue;
            }

            auto tmp_type = router_.printer(target_entity).first;
            auto tmp_loc = router_.printer(target_entity).second;
            Task new_task = empty_task();

            if (!component_manager_.has_component<RenderComponent>(target_entity)) {
                std::cout << "wrong! current entity has no render component" << std::endl;
                continue;
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
                assert( component_manager_.has_component<StorageComponent>(target_entity) );
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

        std::cout << "after update queue, task wait for assign: " << task_queue_.size() << std::endl;

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
            std::cout << "assign task to character " << character << std::endl;
            bool is_assigned_task = false;
            if (!component_manager_.has_component<TaskComponent>(character)) 
                component_manager_.add_component(character, TaskComponent{.current_task = idle_task()});

            auto& character_task = component_manager_.get_component<TaskComponent>(character);
            if(character_task.current_task.type != TaskType::IDLE || !router_.is_move_finished(character))
                continue;

            std::cout << "task queue has tasks: " << task_queue_.size() << std::endl;
            if (task_queue_.empty())
                continue;

            //sort task queue according to how far each task is from character
            //also consider task priority
            auto char_loc = component_manager_.get_component<LocationComponent>(character).loc;
            std::sort(task_queue_.begin(), task_queue_.end(), [&char_loc](const Task& a, const Task& b) {
                auto distance_squared = [](const Task& task, const Location& loc) -> double {
                    Location tar = task.target_locations[0];
                    double dx = tar.x - loc.x;
                    double dy = tar.y - loc.y;
                    return dx * dx + dy * dy; 
                };

                double score_a = (a.priority > 0 ? 1.0 / a.priority : std::numeric_limits<double>::max()) *
                     distance_squared(a, char_loc);
                double score_b = (b.priority > 0 ? 1.0 / b.priority : std::numeric_limits<double>::max()) *
                     distance_squared(b, char_loc);
                return score_a < score_b;
            });

            for(auto task = task_queue_.begin(); task != task_queue_.end();) {
                if (task -> actor != -1 && task -> actor != character) {
                    std::cout << "this task is owned by entity " << task -> actor << ", while current entity is " << character << std::endl; 
                    ++task;
                    continue;
                }

                //attention! target_pos is a set of locations
                auto targets = task->target_locations; 
                auto character_pos = component_manager_.get_component<LocationComponent>(character).loc;
                auto& character_tasks = component_manager_.get_component<TaskComponent>(character);
                //std::cout << "character is at (" << character_pos.x << ", " << character_pos.y << ")" << std::endl;
                //std::cout << "task is at (" << targets[0].x << ", " << targets[0].y << ")" << std::endl;
                
                if (true || router_.is_reachable(character_pos, targets)) {
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
                    } else if (Entity dst_storage; 
                    task->type == TaskType::ALLOCATE 
                    && !character_carries_resource(character)) {
                        auto blueprint = task->target_action.target_entity;

                        if (!is_target_available_at_moment(blueprint)) {
                            target_timer_tick(blueprint);
                            ++task;
                            continue;
                        } 

                        dst_storage = find_exist_obtain_target(blueprint);
                        if (dst_storage == -1) {
                            auto temp_storage = component_manager_.get_component<StorageComponent>(blueprint);
                            int needed_amount = temp_storage.storage_capacity - temp_storage.current_storage;
                            if (find_resource_destination(character, dst_storage, needed_amount))
                                bind_resource_to_blueprint(dst_storage, blueprint);
                        } 
                        if (dst_storage != -1) {
                            Task obtain = obtain_task(dst_storage, character);

                            //here mark the ownership of ALLOCATE task and OBTAIN task
                            obtain.actor = character;
                            task->actor = character;

                            character_tasks.current_task = obtain;
                            task_in_progress_.emplace_back(obtain);
                            is_assigned_task = true;
                            std::cout << "character has no resource, but find a storage area to obtain resources" << std::endl;
                            break;
                        } else {
                            start_target_timer(blueprint, 100); //wait 100 rounds
                        }
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
                    std::cout << "unreachable task for character " << character << std::endl;
                    ++task;
                    continue;
                }
            }

            //if no task is assigned, let character idle
            if (!is_assigned_task) {
                character_task.current_task = idle_task();
            }

            router_.print_character_current_task(character);
        }
        std::cout << "After assign: task wait for assign: " << task_queue_.size() << ", task in progress: " << task_in_progress_.size() << std::endl;
        
        //animals are always idling
        for(auto& animal : router_.get_animals()) {
            if (!component_manager_.has_component<TaskComponent>(animal))
                component_manager_.add_component(animal, TaskComponent{idle_task()});
            if (!router_.is_move_finished(animal)) 
                continue;
            component_manager_.get_component<TaskComponent>(animal).current_task = idle_task();
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
    Task obtain_task(Entity entity, Entity actor);
    
    //entity: storage area(units) to store resources
    Task store_task(Entity entity);
    
    //entity: blueprint to be allocated resource
    Task allocate_task(Entity entity);

    //entity: blueprint to be built
    Task construct_task(Entity entity);

    

private:
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
            if ( src_pos == dst_pos || 
                (router_.find_path_Ax(src_pos, dst_pos).size() != 0 
                && storage.current_storage < storage.storage_capacity
                && ( storage.current_storage >= resource_amount ) ) ) {
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
        if (path_exists) 
            std::cout << "find storage " << dst << " at (" << final_dst.x << ", " << final_dst.y << ")" << std::endl;
        else 
            std::cout << "no resource destination found" << std::endl;
        return path_exists;
    }
};

bool TaskSystem::target_entity_in_task_queue(Entity entity) {
    for (auto& task : task_queue_) {
        if (task.target_action.target_entity == entity)
            return true;
    }
    return false;
}

bool TaskSystem::is_target_in_progress(Entity entity) {
    for (auto& task : task_in_progress_) {
        auto& target = task.target_action.target_entity;
        if (target == entity)
                return true;
    }
    return false;
} 

void TaskSystem::remove_task_from_progress_by_id(int id) {
    for(auto task = task_in_progress_.begin(); task != task_in_progress_.end();) {
        if (task->id == id) task = task_in_progress_.erase(task);
        else ++task;
    }
}

void TaskSystem::remove_task_from_queue_by_id(int id) {
    for(auto task = task_queue_.begin(); task != task_queue_.end();) {
        if (task->id == id) task = task_queue_.erase(task);
        else ++task;
    }
}

void TaskSystem::remove_allocate_task_assigned_to_character(Entity entity) {
    for(auto task = task_queue_.begin(); task != task_queue_.end();) {
        if (task -> type == TaskType::ALLOCATE && task -> actor == entity) task = task_queue_.erase(task);
        else ++task;
    }
    for(auto task = task_queue_.begin(); task != task_queue_.end();) {
        if (task -> type == TaskType::ALLOCATE && task -> actor == entity) task = task_queue_.erase(task);
        else ++task;
    }
}

bool TaskSystem::character_carries_resource(Entity entity) {
    assert(component_manager_.has_component<StorageComponent>(entity));
    auto& bag = component_manager_.get_component<StorageComponent>(entity);
    return bag.current_storage > 0;
}

void TaskSystem::remove_task_from_queue_by_target(Entity entity) {
    for(auto it = task_in_progress_.begin(); it != task_in_progress_.end();) {
        if (it->target_action.target_entity == entity) it = task_in_progress_.erase(it);
        else ++it;
    }

    for(auto it = task_queue_.begin(); it != task_queue_.end();) {
        if (it->target_action.target_entity == entity) it = task_queue_.erase(it);
        else ++it;
    }
}

void TaskSystem::remove_finished_task() {

    for(auto& entity : get_finished_target_entities()) {
        std::cout << "target entity: " << entity << " is finished, removed from queue" << std::endl;
        remove_task_from_queue_by_target(entity);
    }

    for(auto& character : router_.get_characters()) {
        if (!component_manager_.has_component<TaskComponent>(character))
            continue;

        auto& task = component_manager_.get_component<TaskComponent>(character);
        auto& curTask = task.current_task;
           
        if(curTask.finished) {
            std::cout << "character " << character << " 's current task type: " << static_cast<int>(curTask.type) << " is finished" << std::endl;  
            if (curTask.type == TaskType::IDLE) 
                continue;
            remove_task_from_progress_by_id(curTask.id);
            curTask = idle_task();
        }

        if (curTask.type == TaskType::ALLOCATE && curTask.feasible) {
            auto& blueprint = curTask.target_action.target_entity;
            if (component_manager_.has_component<ConstructionComponent>(blueprint)) {
                auto& construction = component_manager_.get_component<ConstructionComponent>(blueprint);
                if (construction.allocated || construction.is_built) {
                    remove_task_from_progress_by_id(curTask.id);
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
                    remove_task_from_progress_by_id(curTask.id);
                    curTask = idle_task();
                }
            }
        }
    }
}

Entities TaskSystem::get_finished_target_entities() {
    Entities entities;
    for(auto& entity : router_.get_all_entities()) {
        if (component_manager_.has_component<TargetComponent>(entity)) {
            auto& target = component_manager_.get_component<TargetComponent>(entity);
            if (target.is_finished && target.is_target) {
                std::cout << "find finished target: " << entity << std::endl;
                entities.emplace_back(entity);
                target.is_target = false;
                //if target is a tree, it is finished
                //it will be deleted
                auto& type = component_manager_.get_component<RenderComponent>(entity).entityType;
                if (type == EntityType::TREE) {
                    std::cout << "tree " << entity << " will be deleted" << std::endl;
                    component_manager_.get_component<TargetComponent>(entity).to_be_deleted = true;
                } else if (type == EntityType::WOODPACK) {
                    component_manager_.remove_component<TargetComponent>(entity);
                }
            }
        }
    }
    return entities;
}

void TaskSystem::bind_resource_to_blueprint(Entity reso, Entity blueprint) {
    resource_bind[blueprint] = reso;
}

Entity TaskSystem::find_exist_obtain_target(Entity blueprint) {
    if (resource_bind.find(blueprint) != resource_bind.end())
        return resource_bind[blueprint];
    else
        return -1;
}

bool TaskSystem::is_resoure_bound_to_blueprint(Entity reso) {
    for(auto pair : resource_bind) {
        if (pair.second == reso)
            return true;
    }
    return false;
}

void TaskSystem::update_storage() {
    for(auto pair : resource_bind) {
        auto blueprint = pair.first;
        auto& construction = component_manager_.get_component<ConstructionComponent>(blueprint);
        if (construction.allocated) 
            resource_bind.erase(blueprint);
    }

    for(auto& character : router_.get_characters()) {
        assert(component_manager_.has_component<TaskComponent>(character));
        auto& task = component_manager_.get_component<TaskComponent>(character).current_task;
        if (character_carries_resource(character) 
            && task.type != TaskType::ALLOCATE
            && task.type != TaskType::OBTAIN) {
            Entity storage_to_store;
            bool any_to_store = find_resource_destination(character, storage_to_store, 0);
            if (is_resoure_bound_to_blueprint(storage_to_store)) {
                std::cout << "current storage " << storage_to_store << " is locked!" << std::endl;
                any_to_store = false;
            }
            if (any_to_store) {
                std::cout << "character " << character << " to store at " << storage_to_store << std::endl;
                if (!component_manager_.has_component<TargetComponent>(storage_to_store)) {
                    component_manager_.add_component(storage_to_store, TargetComponent{
                        .progress = 0,
                        .is_target = true,
                        .is_finished = false
                    });
                } else {
                    component_manager_.get_component<TargetComponent>(storage_to_store).is_target = true;
                    component_manager_.get_component<TargetComponent>(storage_to_store).is_finished = false;
                }
            }
        }
    }
}

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
        .priority = 10, 
        .target_action = Action{
            ActionType::CHOP, pos, 2.0, entity
        },
        .feasible = true,
        .id = ++id_
    };
}

Task TaskSystem::collect_task(Entity entity) {
    auto& pos = component_manager_.get_component<LocationComponent>(entity).loc;
    
    return Task{
        .type = TaskType::COLLECT, 
        .target_locations = {pos},
        .priority = 15, 
        .target_action = Action{
            ActionType::PICK, pos, 1.0, entity
        },
        .feasible = true,
        .id = ++id_
    };
}

Task TaskSystem::obtain_task(Entity entity, Entity actor) {
    auto& pos = component_manager_.get_component<LocationComponent>(entity).loc;
    return Task{
        .type = TaskType::OBTAIN, 
        .target_locations = {pos},
        .priority = 3, 
        .target_action = Action{
            ActionType::PICK, pos, 1.0, entity
        },
        .feasible = true,
        .id = ++id_,
        .actor = actor
    };
}
    
Task TaskSystem::store_task(Entity entity) {
    auto& pos = component_manager_.get_component<LocationComponent>(entity).loc;

    return Task{
        .type = TaskType::STORE, 
        .target_locations = router_.get_locations_around(pos),
        .priority = 8,//always consider storing first 
        .target_action = Action{
            ActionType::PLACE, pos, 1.0, entity
        },
        .feasible = true,
        .id = ++id_
    };
}
    
Task TaskSystem::allocate_task(Entity entity) { 
    auto& pos = component_manager_.get_component<LocationComponent>(entity).loc;
    
    return Task{
        .type = TaskType::ALLOCATE, 
        .target_locations = router_.get_locations_around(pos),
        .priority = 10, 
        .target_action = Action{
            ActionType::PLACE, pos, 1.0, entity
        },
        .feasible = true,
        .id = ++id_
    };
}

Task TaskSystem::construct_task(Entity entity) {
    auto& pos = component_manager_.get_component<LocationComponent>(entity).loc;
    
    return Task{
        .type = TaskType::CONSTRUCT, 
        .target_locations = router_.get_locations_around(pos),
        .priority = 5, 
        .target_action = Action{
            ActionType::BUILD, pos, 5.0, entity
        },
        .feasible = true,
        .id = ++id_
    };
}