#include "world/world.h"

enum class ActionType {
    MOVE,
    CHOP,
    BUILD,
    COLLECT
};

struct Action {
    ActionType type;
    location target_location;
    float duration;
    Entity target_entity;
};
class ActionSystem {
public:
    ActionSystem(World& world) {
        auto entity_manager = world.entity_manager_;
        auto component_manager = world.component_manager_; 
    }

    void update(float dt) {
        auto entities = entity_manager.get_all_entities();
        for (auto entity : entities) {
            if (!component_manager.has_component<ActionComponent>(entity) ||
                !component_manager.has_component<TaskComponent>(entity)) {
                continue;
            }

            auto& action_comp = component_manager.get_component<ActionComponent>(entity);
            auto& task_comp = component_manager.get_component<TaskComponent>(entity);

            if (task_comp.current_task) {
                // 执行当前动作
                if (action_comp.current_action) {
                    execute_current_action(entity, dt);
                }
                // 获取新动作
                else {
                    start_next_action(entity);
                }
            }
        }
    }

private:
    void start_next_action(Entity entity) {
        auto& task_comp = component_manager.get_component<TaskComponent>(entity);
        auto& action_comp = component_manager.get_component<ActionComponent>(entity);
        
        Action* next_action = task_comp.current_task->get_next_action();
        if (next_action) {
            action_comp.current_action = next_action;
            action_comp.action_timer = 0;
            action_comp.action_progress = 0;
        }
    }

    void execute_current_action(Entity entity, float dt) {
        auto& action_comp = component_manager.get_component<ActionComponent>(entity);
        
        switch (action_comp.current_action->type) {
            case ActionType::MOVE:
                execute_move_action(entity, dt);
                break;
            default:
                update_action_progress(entity, dt);
                break;
        }

        // 检查动作是否完成
        if (action_comp.action_progress >= 100) {
            complete_action(entity);
        }
    }

    void execute_move_action(Entity entity, float dt) {
        auto& action_comp = world_.component_manager_.get_component<ActionComponent>(entity);
        auto& loc_comp = world_.component_manager_.get_component<locationComponent>(entity);
        auto& move_comp = world_.component_manager_.get_component<moveComponent>(entity);
        auto& render_comp = world_.component_manager_.get_component<RenderComponent>(entity);

        location target = action_comp.current_action->target_location;
        float dx = target.x - loc_comp.loc.x;
        float dy = target.y - loc_comp.loc.y;
        float distance = std::sqrt(dx * dx + dy * dy);

        if (distance > 0.1f) {
            float speed = 5.0f;
            move_comp.veloX = (dx / distance) * speed;
            move_comp.veloY = (dy / distance) * speed;

            loc_comp.loc.x += move_comp.veloX * dt;
            loc_comp.loc.y += move_comp.veloY * dt;
            render_comp.render_pos = sf::Vector2f(loc_comp.loc.x * TILE_SIZE, loc_comp.loc.y * TILE_SIZE);

            action_comp.action_progress = (1.0f - (distance / action_comp.current_action->duration)) * 100;
        } else {
            loc_comp.loc = target;
            render_comp.render_pos = sf::Vector2f(target.x * TILE_SIZE, target.y * TILE_SIZE);
            action_comp.action_progress = 100;
            move_comp.veloX = 0;
            move_comp.veloY = 0;
        }
    }

    void update_action_progress(Entity entity, float dt) {
        auto& action_comp = component_manager.get_component<ActionComponent>(entity);
        action_comp.action_timer += dt;
        action_comp.action_progress = 
            (action_comp.action_timer / action_comp.current_action->duration) * 100;
    }

    void complete_action(Entity entity) {
        auto& action_comp = world_.component_manager_.get_component<ActionComponent>(entity);
        auto& task_comp = world_.component_manager_.get_component<TaskComponent>(entity);

        switch (action_comp.current_action->type) {
            case ActionType::CHOP:
                complete_chop_action(entity);
                break;
            case ActionType::BUILD:
                complete_build_action(entity);
                break;
            case ActionType::COLLECT:
                complete_collect_action(entity);
                break;
            default:
                break;
        }

        delete action_comp.current_action;
        action_comp.current_action = nullptr;
        action_comp.action_timer = 0;
        action_comp.action_progress = 0;

        if (!task_comp.current_task->get_next_action()) {
            task_comp.current_task = nullptr;
        }
    }

    void complete_chop_action(Entity entity) {
        auto& action_comp = world_.component_manager_.get_component<ActionComponent>(entity);
        Entity tree = action_comp.current_action->target_entity;
        
        if (world_.component_manager_.has_component<ResourceComponent>(tree)) {
            auto& tree_loc = world_.component_manager_.get_component<locationComponent>(tree);
            world_.create_wood(tree_loc.loc.x, tree_loc.loc.y);
            world_.entity_manager_.destroy_entity(tree);
        }
    }

    void complete_build_action(Entity entity) {
        auto& action_comp = world_.component_manager_.get_component<ActionComponent>(entity);
        Entity blueprint = action_comp.current_action->target_entity;
        
        if (world_.component_manager_.has_component<ConstructionComponent>(blueprint)) {
            auto& construction = world_.component_manager_.get_component<ConstructionComponent>(blueprint);
            auto& collision = world_.component_manager_.get_component<collisionComponent>(blueprint);
            
            construction.is_blueprint = false;
            construction.to_build = false;
            collision.collidable = true;
        }
    }

    void complete_collect_action(Entity entity) {
        auto& action_comp = world_.component_manager_.get_component<ActionComponent>(entity);
        Entity resource = action_comp.current_action->target_entity;
        
        if (world_.component_manager_.has_component<ResourceComponent>(resource)) {
            world_.entity_manager_.destroy_entity(resource);
        }
    }

    World& world_;


    ComponentManager& component_manager;
    EntityManager& entity_manager;
    World& world;
};