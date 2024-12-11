#pragma once
#include "world/world.hpp"
#include "components/componentManager.hpp"
#include "entities/entity.hpp"
#include <queue>
#include <algorithm>
class TaskSystem : public ISystem {
public:
    TaskSystem() = default;
    TaskSystem(World& new_world) : 
        world_(new_world),
        entity_manager_(new_world.entity_manager_),
        component_manager_(new_world.component_manager_) {
        std::cout << "TaskSystem initialized" << std::endl;
    }

    void update() override {
        update_task_queue();
        assign_task();
    }

private:
    // 任务创建辅助函数
    Task create_task(TaskType type, Location pos, int priority, ActionType action, 
                    float duration, Entity target, bool use_surrounding = false) {
        auto locations = use_surrounding ? get_locations_around(pos) : std::vector<Location>{pos};
        return Task{
            .type = type,
            .target_locations = locations,
            .priority = priority,
            .target_action = Action{action, pos, duration, target}
        };
    }

    // 简化后的任务生成函数
    Task empty_task() {
        return create_task(TaskType::EMPTY, {-1, -1}, -1, ActionType::NONE, 0, -1);
    }

    Task idle_task(Entity entity) {
        auto& pos = component_manager_.get_component<LocationComponent>(entity).loc;
        return create_task(TaskType::IDLE, pos, 0, ActionType::WANDER, 0, entity);
    }

    Task chop_task(Entity entity) {
        auto& pos = component_manager_.get_component<LocationComponent>(entity).loc;
        return create_task(TaskType::CHOP, pos, 6, ActionType::CHOP, 5.0, entity, true);
    }

    // ... 其他任务生成函数类似简化 ...

    void update_task_queue() {
        update_existing_tasks();
        add_new_tasks();
        sort_task_queue();
    }

    void update_existing_tasks() {
        for(auto& entity : world_.get_entities_with_components<TaskComponent>()) {
            auto& task = component_manager_.get_component<TaskComponent>(entity);
            update_task_status(entity, task);
        }
    }

    void update_task_status(Entity entity, TaskComponent& task) {
        if (!task.current_task.feasible) {
            task_queue_.push_back(task.current_task);
            task.current_task = idle_task(entity);
        }

        auto& target = task.current_task.target_action.target_entity;
        if (component_manager_.has_component<TargetComponent>(target)) {
            auto& track = component_manager_.get_component<TargetComponent>(target);
            if (track.is_finished) {
                component_manager_.remove_component<TargetComponent>(target);
                task.current_task = idle_task(entity);
            }
        }
    }

    void add_new_tasks() {
        for (auto& target : world_.get_entities_with_components<TargetComponent>()) {
            if (target_in_queue(target)) continue;
            
            auto& track = component_manager_.get_component<TargetComponent>(target);
            if (!track.is_target) continue;

            if (auto task = create_task_for_entity(target)) {
                task_queue_.push_back(*task);
            }
        }

        if (task_queue_.empty()) {
            task_queue_.push_back(idle_task(-1));
        }
    }

    std::optional<Task> create_task_for_entity(Entity entity) {
        auto& render = component_manager_.get_component<RenderComponent>(entity);
        
        switch(render.entityType) {
            case EntityType::TREE:
                return chop_task(entity);
            case EntityType::WOODPACK:
                if (component_manager_.get_component<ResourceComponent>(entity).holder == -1) {
                    return collect_task(entity);
                }
                break;
            case EntityType::WALL:
            case EntityType::DOOR:
                return handle_construction_task(entity);
            case EntityType::STORAGE:
                return store_task(entity);
        }
        return std::nullopt;
    }

    // ... 其他辅助函数保持不变 ...

    World& world_;
    EntityManager& entity_manager_;
    ComponentManager& component_manager_;
    std::vector<Task> task_queue_;
};