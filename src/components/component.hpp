#pragma once
#include <cstdint>
#include <utility>
#include <list>
#include <array>
#include <vector>
#include <functional>
#include <deque>
#include "../lib/nlohmann/json.hpp"
#define MAX_COMPONENTS 32
 
using ComponentType = std::uint8_t;
using Entity = int;
using Entities = std::vector<Entity>;

using json = nlohmann::json;

struct Location {
    int x;
    int y;
    Location() : x(0), y(0) {}
    Location(int x, int y) : x(x), y(y) {}
    bool operator==(const Location& other) const {
        return x == other.x && y == other.y;
    }
    bool operator!=(const Location& other) const {
        return x != other.x || y != other.y;
    }

};

namespace std {  
    template<>
        struct hash<Location> {
        size_t operator()(const Location& loc) const noexcept {
            return hash<int>()(loc.x) ^ (hash<int>()(loc.y) << 1);
        }
    };
}

using Locations = std::vector<Location>;
using RenderPos = std::pair<float, float>;
using Field = std::pair<Location, Location>;

enum EntityType {
    CHARACTER,
    TREE,
    WOODPACK,
    WALL,
    DOOR,
    DOG,
    STORAGE,
    TEMP
};

//different tasks have different location requirements
enum TaskType {
    CHOP_WOOD, //near tree
    CONSTRUCT, //near construction
    ALLOCATE, //near construction
    IDLE, //entity itself
    COLLECT, //in unit of resource
    STORE, //near storage
    OBTAIN, //near storage
    EMPTY
};

enum ActionType {
    CHOP,   //chop tree marked(has target component)
    BUILD, //build if any blueprint is allocated
    MOVE,   //move to target location using BFS
    PICK, //pick from storage or tree left
    PLACE, //place to storage or blueprints
    NONE
};

struct Action {
    ActionType type;
    Location target_location; //accurate one point
    float duration;
    Entity target_entity; //entity to be acted on
};

//first reach target location, then do target action
struct Task {
    TaskType type;
    Locations target_locations; //a field able to do tasks
    int priority;
    Action target_action;
    bool feasible;
    bool finished;
    int id;
    Entity actor;
    void print_task_info() {
        std::cout << "--------------------------------" << std::endl;
        std::cout << "task id: " << id << ", type: " << static_cast<int>(type) << ", target locations: " << target_locations[0].x << ", " << target_locations[0].y << std::endl;
        std::cout << "target action: " << static_cast<int>(target_action.type) << ", act on: " << target_action.target_entity << std::endl;
        std::cout << "actor: " << actor << ", priority: " << priority << ", feasible: " << feasible << std::endl;
        std::cout << "--------------------------------" << std::endl;
    }
};

struct LocationComponent {
    Location loc;
};

struct MovementComponent {
    Location start_pos;
    Location end_pos;
    float progress; 
    float speed;    
    bool move_finished;
};

struct ResourceComponent {
    int amount;
    Entity holder;
};

struct RenderComponent {
    EntityType entityType;
    bool is_selected;
    bool collidable;
};

struct ConstructionComponent {
    bool allocated;
    bool is_built;
};

struct CreateComponent {
    bool to_be_created;
    int amount;
    EntityType entity_type;
};

//5 woods in one pack
struct StorageComponent {
    int storage_capacity;
    int current_storage;
    std::deque<Entity> stored_resources;
};

//character will have this
struct TaskComponent {
    Task current_task;
    Task next_task;
};
//whole procedure: assign a task to a character, 
//character move to target location, 
//perform action, 
//update task progress,
//if task is completed, assign a new/empty task
struct ActionComponent {  
    Action current_action;
    bool action_finished;
    bool in_progress;
};
    
//used to track task and progress
struct TargetComponent {
    float progress;
    float timer;
    bool is_target;
    bool is_finished;
    bool to_be_deleted;//in world, it will be deleted
    Entity hold_by;
};

void to_json(json& j, const EntityType& et) {
    j = static_cast<int>(et);
}

void from_json(const json& j, EntityType& et) {
    et = static_cast<EntityType>(j.get<int>());
}

void to_json(json& j, const TaskType& tt) {
    j = static_cast<int>(tt);
}

void from_json(const json& j, TaskType& tt) {
    tt = static_cast<TaskType>(j.get<int>());
}

void to_json(json& j, const ActionType& at) {
    j = static_cast<int>(at);
}

void from_json(const json& j, ActionType& at) {
    at = static_cast<ActionType>(j.get<int>());
}

void to_json(json& j, const Location& loc) {
    j = json{{"x", loc.x}, {"y", loc.y}};
}

void from_json(const json& j, Location& loc) {
    j.at("x").get_to(loc.x);
    j.at("y").get_to(loc.y);
}

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Action, type, target_location, duration, target_entity)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Task, type, target_locations, priority, target_action, 
    feasible, finished, id, actor)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(LocationComponent, loc)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MovementComponent, start_pos, end_pos, progress, speed, move_finished)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ResourceComponent, amount, holder)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(RenderComponent, entityType, is_selected, collidable)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ConstructionComponent, allocated, is_built)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CreateComponent, to_be_created, amount, entity_type)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(StorageComponent, storage_capacity, current_storage, stored_resources)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(TaskComponent, current_task, next_task)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ActionComponent, current_action, action_finished, in_progress)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(TargetComponent, progress, timer, is_target, is_finished, to_be_deleted, hold_by)