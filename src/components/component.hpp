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
using field = std::pair<sf::Vector2i, sf::Vector2i>;
using location = sf::Vector2i;
enum EntityType {
    CHARACTER,
    TREE,
    WOOD,
    STONE,
    WALL,
    DOOR,
    DOG,
    STORAGE
};
struct locationComponent {
    location loc;
};
struct fieldComponent {
    field field;
};
struct collisionComponent {
    bool collidable;
};
struct moveComponent {
    float veloX;
    float veloY;
};
struct ResourceComponent {
    int amount;
    bool collectable;
};
struct RenderComponent {
    EntityType entityType;
    bool is_selected;
    sf::Vector2f render_pos;
};
struct ConstructionComponent {
    bool destoryable;
    bool buildable;
    bool is_blueprint;
    bool to_build;
};
struct StorageComponent {
    int storage_capacity;
};

struct TaskComponent {
    std::queue<Task> task_queue;
    Task* current_task;
    float task_progress;  // 0-100
};

struct ActionComponent {
    Action* current_action;
    float action_progress;  // 0-100
    float action_timer;
};