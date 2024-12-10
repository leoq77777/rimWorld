// savesystem.hpp
#ifndef SAVE_SYSTEM_HPP
#define SAVE_SYSTEM_HPP

#include "world/world.h"
#include <fstream>
#include <json.hpp> // 使用nlohmann/json库
using json = nlohmann::json;

class SaveSystem {
public:
    static void save_world(const World& world, const std::string& filename) {
        json save_data;
        
        // 保存实体数据
        save_data["entities"] = save_entities(world);
        
        // 保存组件数据
        save_data["components"] = save_components(world);
        
        // 保存地图数据
        save_data["entity_map"] = save_entity_map(world);
        
        // 保存标记数据
        save_data["mark_map"] = world.mark_map_;

        // 写入文件
        std::ofstream file(filename);
        file << std::setw(4) << save_data;
    }

    static void load_world(World& world, const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cout << "No save file found, starting new game" << std::endl;
            return;
        }

        json save_data = json::parse(file);
        
        // 清理现有世界数据
        clear_world(world);
        
        // 加载实体
        load_entities(world, save_data["entities"]);
        
        // 加载组件
        load_components(world, save_data["components"]);
        
        // 加载地图
        load_entity_map(world, save_data["entity_map"]);
        
        // 加载标记
        world.mark_map_ = save_data["mark_map"].get<std::vector<std::vector<int>>>();
    }

private:
    static json save_entities(const World& world) {
        json entities_data;
        auto all_entities = world.entity_manager_.get_all_entities();
        entities_data["living_entities"] = all_entities;
        return entities_data;
    }

    static json save_components(const World& world) {
        json components_data;
        auto all_entities = world.entity_manager_.get_all_entities();
        
        for (Entity entity : all_entities) {
            json entity_components;
            
            // 保存各种组件数据
            if (world.component_manager_.has_component<locationComponent>(entity)) {
                auto& comp = world.component_manager_.get_component<locationComponent>(entity);
                entity_components["location"] = {
                    {"x", comp.loc.x},
                    {"y", comp.loc.y}
                };
            }
            
            if (world.component_manager_.has_component<ResourceComponent>(entity)) {
                auto& comp = world.component_manager_.get_component<ResourceComponent>(entity);
                entity_components["resource"] = {
                    {"amount", comp.amount},
                    {"collectable", comp.collectable}
                };
            }
            
            if (world.component_manager_.has_component<RenderComponent>(entity)) {
                auto& comp = world.component_manager_.get_component<RenderComponent>(entity);
                entity_components["render"] = {
                    {"type", static_cast<int>(comp.entityType)},
                    {"selected", comp.is_selected},
                    {"pos_x", comp.render_pos.x},
                    {"pos_y", comp.render_pos.y}
                };
            }
            
            // ... 其他组件的保存逻辑 ...
            
            components_data[std::to_string(entity)] = entity_components;
        }
        
        return components_data;
    }

    static json save_entity_map(const World& world) {
        json map_data;
        for (int x = 0; x < MAP_SIZE; ++x) {
            for (int y = 0; y < MAP_SIZE; ++y) {
                if (!world.entity_map_[x][y].empty()) {
                    map_data.push_back({
                        {"x", x},
                        {"y", y},
                        {"entities", world.entity_map_[x][y]}
                    });
                }
            }
        }
        return map_data;
    }

    static void clear_world(World& world) {
        auto all_entities = world.entity_manager_.get_all_entities();
        for (Entity entity : all_entities) {
            world.entity_manager_.destroy_entity(entity);
        }
        world.entity_map_.clear();
        world.entity_map_.resize(MAP_SIZE, std::vector<Entities>(MAP_SIZE));
        world.mark_map_.clear();
        world.mark_map_.resize(MAP_SIZE, std::vector<int>(MAP_SIZE, -1));
    }

    static void load_entities(World& world, const json& entities_data) {
        for (Entity entity : entities_data["living_entities"]) {
            world.entity_manager_.create_entity();
        }
    }

    static void load_components(World& world, const json& components_data) {
        for (auto& [entity_str, components] : components_data.items()) {
            Entity entity = std::stoi(entity_str);
            
            // 加载位置组件
            if (components.contains("location")) {
                locationComponent loc{
                    sf::Vector2i(
                        components["location"]["x"],
                        components["location"]["y"]
                    )
                };
                world.component_manager_.add_component(entity, loc);
            }
            
            // 加载资源组件
            if (components.contains("resource")) {
                ResourceComponent res{
                    components["resource"]["amount"],
                    components["resource"]["collectable"]
                };
                world.component_manager_.add_component(entity, res);
            }
            
            // 加载渲染组件
            if (components.contains("render")) {
                RenderComponent render{
                    static_cast<EntityType>(components["render"]["type"]),
                    components["render"]["selected"],
                    sf::Vector2f(
                        components["render"]["pos_x"],
                        components["render"]["pos_y"]
                    )
                };
                world.component_manager_.add_component(entity, render);
            }
            
            // ... 其他组件的加载逻辑 ...
        }
    }

    static void load_entity_map(World& world, const json& map_data) {
        for (const auto& tile : map_data) {
            int x = tile["x"];
            int y = tile["y"];
            world.entity_map_[x][y] = tile["entities"].get<std::vector<Entity>>();
        }
    }
};

#endif // SAVE_SYSTEM_HPP