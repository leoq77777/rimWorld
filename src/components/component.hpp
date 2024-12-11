#pragma once
#include <cstdint>
#include <utility>
#include <SFML/Graphics.hpp>
#include <list>
#include <array>
#define MAX_COMPONENTS 32
#define MAX_ENTITIES 10000
 
using ComponentType = std::uint8_t;
using Entity = int;
using Entities = std::vector<Entity>;
using Field = std::pair<sf::Vector2i, sf::Vector2i>;
using Location = sf::Vector2i;
using Locations = std::vector<Location>;
enum EntityType {
    CHARACTER,
    TREE,
    WOODPACK,
    WALL,
    DOOR,
    DOG,
    STORAGE
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
    Entity target_entity;
};

//first reach target location, then do target action
struct Task {
    TaskType type;
    Locations target_locations; //a field able to do tasks
    int priority;
    Action target_action;
    bool feasible;
};

struct locationComponent {
    Location loc;
};

struct fieldComponent {
    Field field;
};

struct collisionComponent {
    bool collidable;
};

struct moveComponent {
    int speed;
};

struct ResourceComponent {
    int amount;
    Entity holder;
};

struct RenderComponent {
    EntityType entityType;
    bool is_selected;
    sf::Vector2f render_pos;
};

struct ConstructionComponent {
    bool allocated;
    bool is_built;
};

//5 woods in one pack
struct StorageComponent {
    int storage_capacity;
    int current_storage;
    Entities stored_resources;
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
