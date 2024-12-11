#pragma once

#include "Component.hpp"
#include "utils/json.hpp"

// 辅助宏来简化序列化和反序列化函数的编写
#define SERIALIZE_COMPONENT(component, ...) \
    friend void to_json(nlohmann::json& j, const component& c) { j = nlohmann::json{{__VA_ARGS__}}; } \
    friend void from_json(const nlohmann::json& j, component& c) { using expander = int[]; (void)expander{0, (j.at(#__VA_ARGS__).get_to(c.__VA_ARGS__), 0)...}; }

// 为每个组件添加序列化和反序列化支持

// locationComponent
SERIALIZE_COMPONENT(locationComponent, "loc", loc)

// fieldComponent
SERIALIZE_COMPONENT(fieldComponent, "field", field)

// collisionComponent
SERIALIZE_COMPONENT(collisionComponent, "collidable", collidable)

// moveComponent
SERIALIZE_COMPONENT(moveComponent, "veloX", veloX, "veloY", veloY)

// ResourceComponent
SERIALIZE_COMPONENT(ResourceComponent, "amount", amount, "collectable", collectable)

// RenderComponent
SERIALIZE_COMPONENT(RenderComponent, "entityType", entityType, "is_selected", is_selected, "render_pos", render_pos)

// ConstructionComponent
SERIALIZE_COMPONENT(ConstructionComponent, "destoryable", destoryable, "buildable", buildable, "is_blueprint", is_blueprint, "to_build", to_build)

// StorageComponent
SERIALIZE_COMPONENT(StorageComponent, "storage_capacity", storage_capacity)

// Action 结构
friend void to_json(nlohmann::json& j, const Action& a) {
    j = nlohmann::json{
        {"type", static_cast<int>(a.type)},
        {"target_location", a.target_location},
        {"duration", a.duration},
        {"target_entity", a.target_entity}
    };
}

friend void from_json(const nlohmann::json& j, Action& a) {
    a.type = static_cast<ActionType>(j.at("type").get<int>());
    j.at("target_location").get_to(a.target_location);
    j.at("duration").get_to(a.duration);
    j.at("target_entity").get_to(a.target_entity);
}

// Task 结构
friend void to_json(nlohmann::json& j, const Task& t) {
    // Convert queue to vector for serialization
    std::vector<Action> actions_vec;
    std::queue<Action*> temp_queue(t.actions);
    while (!temp_queue.empty()) {
        actions_vec.push_back(*temp_queue.front());
        temp_queue.pop();
    }
    j = nlohmann::json{
        {"type", static_cast<int>(t.type)},
        {"target_entity", t.target_entity},
        {"target_location", t.target_location},
        {"priority", t.priority},
        {"actions", actions_vec}
    };
}

friend void from_json(const nlohmann::json& j, Task& t) {
    t.type = static_cast<TaskType>(j.at("type").get<int>());
    j.at("target_entity").get_to(t.target_entity);
    j.at("target_location").get_to(t.target_location);
    j.at("priority").get_to(t.priority);
    // Deserialize into vector and then back into queue
    std::vector<Action> actions_vec = j.at("actions").get<std::vector<Action>>();
    for (auto& action : actions_vec) {
        t.actions.push(new Action(action));
    }
}

// TaskComponent
friend void to_json(nlohmann::json& j, const TaskComponent& tc) {
    // Convert queue to vector for serialization
    std::vector<Task> tasks;
    std::queue<Task> temp_queue(tc.task_queue);
    while (!temp_queue.empty()) {
        tasks.push_back(temp_queue.front());
        temp_queue.pop();
    }
    j = nlohmann::json{
        {"task_queue", tasks},
        {"current_task", tc.current_task != nullptr ? *tc.current_task : nullptr},
        {"task_progress", tc.task_progress}
    };
}

friend void from_json(const nlohmann::json& j, TaskComponent& tc) {
    // Deserialize into vector and then back into queue
    std::vector<Task> tasks = j.at("task_queue").get<std::vector<Task>>();
    for (const auto& task : tasks) {
        tc.task_queue.push(task);
    }
    if (!j["current_task"].is_null()) {
        tc.current_task = new Task(j.at("current_task"));
    } else {
        tc.current_task = nullptr;
    }
    tc.task_progress = j.at("task_progress");
}

// ActionComponent
friend void to_json(nlohmann::json& j, const ActionComponent& ac) {
    j = nlohmann::json{
        {"current_action", ac.current_action != nullptr ? *ac.current_action : nullptr},
        {"action_progress", ac.action_progress},
        {"action_timer", ac.action_timer}
    };
}

friend void from_json(const nlohmann::json& j, ActionComponent& ac) {
    if (!j["current_action"].is_null()) {
        ac.current_action = new Action(j.at("current_action"));
    } else {
        ac.current_action = nullptr;
    }
    ac.action_progress = j.at("action_progress");
    ac.action_timer = j.at("action_timer");
}