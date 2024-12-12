#pragma once
#include <cstdint>
#include <utility>
#include <list>
#include <array>
#include <vector>
#include <functional>
#include <deque>
#define MAX_COMPONENTS 32
 
using ComponentType = std::uint8_t;
using Entity = int;
using Entities = std::vector<Entity>;

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
    ALLOCATE, //near storage
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
    int id;
};

struct LocationComponent {
    Location loc;
};

struct FieldComponent {
    Field field;
};

struct CollisionComponent {
    bool collidable;
};

struct MovementComponent {
    Location start_pos;
    Location end_pos;
    float progress; // 0.0 到 1.0
    float speed;    // 每秒移动的格子数
};

struct ResourceComponent {
    int amount;
    Entity holder;
};

struct RenderComponent {
    EntityType entityType;
    bool is_selected;
    RenderPos render_pos;
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

struct followComponent {
    Entity target;
};
