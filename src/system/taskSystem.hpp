// tasksystem.hpp
#ifndef TASK_SYSTEM_HPP
#define TASK_SYSTEM_HPP

#include "world/world.h"
#include <queue>
#include <algorithm>

enum class TaskType {
    CHOP_WOOD,
    BUILD,
    IDLE,
    WANDER,
    COLLECT
};

struct Task {
    TaskType type;
    Entity target_entity;
    location target_location;
    int priority;
    std::queue<Action*> actions;

    Action* get_next_action() {
        if (actions.empty()) {
            return nullptr;
        }
        Action* next = actions.front();
        actions.pop();
        return next;
    }

    void generate_actions(location start_pos) {
        switch (type) {
            case TaskType::CHOP_WOOD: {
                Action* move = new Action{
                    ActionType::MOVE,
                    target_location,
                    1.0f,
                    target_entity
                };
                Action* chop = new Action{
                    ActionType::CHOP,
                    target_location,
                    3.0f,
                    target_entity
                };
                actions.push(move);
                actions.push(chop);
                break;
            }
            case TaskType::COLLECT: {
                Action* move = new Action{
                    ActionType::MOVE,
                    target_location,
                    1.0f,
                    target_entity
                };
                Action* collect = new Action{
                    ActionType::COLLECT,
                    target_location,
                    1.0f,
                    target_entity
                };
                actions.push(move);
                actions.push(collect);
                break;
            }
            case TaskType::BUILD: {
                Action* move = new Action{
                    ActionType::MOVE,
                    target_location,
                    1.0f,
                    target_entity
                };
                Action* build = new Action{
                    ActionType::BUILD,
                    target_location,
                    5.0f,
                    target_entity
                };
                actions.push(move);
                actions.push(build);
                break;
            }
            case TaskType::WANDER: {
                location random_target = {
                    start_pos.x + (rand() % 20 - 10),
                    start_pos.y + (rand() % 20 - 10)
                };
                Action* wander = new Action{
                    ActionType::MOVE,
                    random_target,
                    2.0f + (rand() % 30) / 10.0f,
                    Entity(-1)
                };
                actions.push(wander);
                break;
            }
            case TaskType::IDLE: {
                Action* idle = new Action{
                    ActionType::MOVE,
                    start_pos,
                    2.0f,
                    Entity(-1)
                };
                actions.push(idle);
                break;
            }
        }
    }
};

class TaskSystem {
public:
    TaskSystem(World& world) : world_(world) {}

    void update(float dt) {
        auto entities = world_.get_active_entities();
        for (auto entity : entities) {
            if (!world_.component_manager_.has_component<TaskComponent>(entity)) {
                continue;
            }

            auto& task_comp = world_.component_manager_.get_component<TaskComponent>(entity);
            
            // 如果没有当前任务，从队列中获取新任务
            if (!task_comp.current_task && !task_comp.task_queue.empty()) {
                task_comp.current_task = &task_comp.task_queue.front();
                task_comp.task_queue.pop();
            }

            // 搜索新任务
            if (task_comp.task_queue.empty()) {
                search_available_tasks(entity);
            }
        }
    }

private:
    float calculate_distance(const location& a, const location& b) {
        return std::sqrt(std::pow(a.x - b.x, 2) + std::pow(a.y - b.y, 2));
    }

    std::vector<Entity> find_entities_in_range(location center, int range) {
        std::vector<Entity> entities_in_range;
        for (int x = std::max(0, center.x - range); x < std::min(MAP_SIZE, center.x + range); ++x) {
            for (int y = std::max(0, center.y - range); y < std::min(MAP_SIZE, center.y + range); ++y) {
                for (Entity entity : world_.entity_map_[x][y]) {
                    if (calculate_distance(center, {x, y}) <= range) {
                        entities_in_range.push_back(entity);
                    }
                }
            }
        }
        return entities_in_range;
    }

    int calculate_priority(Entity worker, Entity target) {
        auto& worker_loc = world_.component_manager_.get_component<locationComponent>(worker);
        auto& target_loc = world_.component_manager_.get_component<locationComponent>(target);
        
        float distance = calculate_distance(worker_loc.loc, target_loc.loc);
        return static_cast<int>(1000.0f / (1.0f + distance));
    }

    void search_available_tasks(Entity entity) {
        auto& loc_comp = world_.component_manager_.get_component<locationComponent>(entity);
        auto& task_comp = world_.component_manager_.get_component<TaskComponent>(entity);
        auto& render_comp = world_.component_manager_.get_component<RenderComponent>(entity);
        
        // 处理狗的漫游任务
        if (render_comp.entityType == EntityType::DOG) {
            if (task_comp.task_queue.empty() && !task_comp.current_task) {
                Task new_task;
                new_task.type = TaskType::WANDER;
                new_task.priority = 1;
                new_task.generate_actions(loc_comp.loc);
                task_comp.task_queue.push(new_task);
            }
            return;
        }

        // 处理角色的任务
        if (render_comp.entityType == EntityType::CHARACTER) {
            std::vector<Task> available_tasks;
            
            // 搜索可收集的资源
            for (auto target : find_entities_in_range(loc_comp.loc, SEARCH_RANGE)) {
                if (world_.component_manager_.has_component<ResourceComponent>(target)) {
                    auto& resource = world_.component_manager_.get_component<ResourceComponent>(target);
                    if (resource.collectable) {
                        Task new_task;
                        new_task.type = TaskType::COLLECT;
                        new_task.target_entity = target;
                        new_task.target_location = world_.component_manager_.get_component<locationComponent>(target).loc;
                        new_task.priority = calculate_priority(entity, target) + 100;
                        new_task.generate_actions(loc_comp.loc);
                        available_tasks.push_back(new_task);
                    }
                }
            }

            // 搜索可伐木的树
            for (auto target : find_entities_in_range(loc_comp.loc, SEARCH_RANGE)) {
                if (world_.component_manager_.has_component<RenderComponent>(target)) {
                    auto& render = world_.component_manager_.get_component<RenderComponent>(target);
                    if (render.entityType == EntityType::TREE) {
                        Task new_task;
                        new_task.type = TaskType::CHOP_WOOD;
                        new_task.target_entity = target;
                        new_task.target_location = world_.component_manager_.get_component<locationComponent>(target).loc;
                        new_task.priority = calculate_priority(entity, target);
                        new_task.generate_actions(loc_comp.loc);
                        available_tasks.push_back(new_task);
                    }
                }
            }

            // 搜索需要建造的蓝图
            for (auto target : find_entities_in_range(loc_comp.loc, SEARCH_RANGE)) {
                if (world_.component_manager_.has_component<ConstructionComponent>(target)) {
                    auto& construction = world_.component_manager_.get_component<ConstructionComponent>(target);
                    if (construction.is_blueprint && construction.to_build) {
                        Task new_task;
                        new_task.type = TaskType::BUILD;
                        new_task.target_entity = target;
                        new_task.target_location = world_.component_manager_.get_component<locationComponent>(target).loc;
                        new_task.priority = calculate_priority(entity, target) + 50;
                        new_task.generate_actions(loc_comp.loc);
                        available_tasks.push_back(new_task);
                    }
                }
            }

            // 按优先级排序任务
            std::sort(available_tasks.begin(), available_tasks.end(), 
                [](const Task& a, const Task& b) { return a.priority > b.priority; });
            
            // 将排序后的任务添加到任务队列
            for (const auto& task : available_tasks) {
                task_comp.task_queue.push(task);
            }

            // 如果没有找到任何任务，添加一个空闲任务
            if (task_comp.task_queue.empty() && !task_comp.current_task) {
                Task idle_task;
                idle_task.type = TaskType::IDLE;
                idle_task.priority = 0;
                idle_task.generate_actions(loc_comp.loc);
                task_comp.task_queue.push(idle_task);
            }
        }
    }

    World& world_;
};

#endif // TASK_SYSTEM_HPP