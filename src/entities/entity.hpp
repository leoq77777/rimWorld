#pragma once
#include "../components/componentManager.hpp"
#include <queue>
#include <bitset>
#include <iostream>
#include <fstream>

class EntityManager {  
public:
    EntityManager() {
        for(int i = 0; i < MAX_ENTITIES; ++i) {
            available_entities.push(i);
        }
        max_entity_id = 0;
    }
    
    Entity create_entity() {
        assert(living_entity_count < MAX_ENTITIES && "Too many entities in existence.");
        Entity id = available_entities.front();
        available_entities.pop();
        ++living_entity_count;
        entity_used.set(id);
        max_entity_id = std::max(max_entity_id, id);
        std::cout << "Entity " << id << " created" << std::endl;
        return id;
    }
    void destroy_entity(Entity entity) {
        available_entities.push(entity);
        --living_entity_count;
        entity_used.reset(entity);
    }

    Entities get_all_entities() const {
        //std::cout << "call: get_all_entities from entity manager\ncurrent max_entity_id: " << max_entity_id << std::endl;
        Entities entities;
        //std::cout << "checkpoint, arrays established" << std::endl;
        for (int i = 0; i <= max_entity_id; ++i) {
            //std::cout << "entity: " << i << std::endl;
            if (entity_used.test(i)) {
                //std::cout << i << " is alive" << std::endl;
                entities.emplace_back(i);
            }
        }
        return entities;
    }

    bool is_entity_alive(Entity entity) {
        return entity_used.test(entity);
    }

    /*
    void save_to_json(const std::string& filename = "entities.json") const {
        nlohmann::json j;
        // 序列化活跃的实体ID列表
        std::vector<Entity> active_entities;
        for (Entity i = 0; i < MAX_ENTITIES; ++i) {
            if (entity_used[i]) {
                active_entities.push_back(i);
            }
        }
        j["active_entities"] = active_entities;

        // 保存到文件
        std::ofstream file(filename);
        if (file.is_open()) {
            file << j.dump(4); // 使用缩进使JSON更易读
            file.close();
        } else {
            std::cerr << "Unable to open file for saving: " << filename << std::endl;
        }
    }

    // 从JSON文件加载实体管理器状态
    void load_from_json(const std::string& filename = "entities.json") {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cout << "No save file found, starting with empty state." << std::endl;
            return;
        }

        nlohmann::json j;
        file >> j;
        file.close();

        // 清除当前状态
        while (!available_entities.empty()) available_entities.pop();
        living_entity_count = 0;
        entity_used.reset();

        // 反序列化活跃的实体ID列表
        if (j.contains("active_entities")) {
            for (const auto& id : j["active_entities"]) {
                if (id.get<Entity>() < MAX_ENTITIES) {
                    entity_used.set(id);
                    ++living_entity_count;
                }
            }
        }

        // 重新填充可用实体队列
        for (Entity i = 0; i < MAX_ENTITIES; ++i) {
            if (!entity_used[i]) {
                available_entities.push(i);
            }
        }
    }
    */

private:

    std::uint32_t living_entity_count{0};
    std::queue<Entity> available_entities;
    std::bitset<MAX_ENTITIES> entity_used;
    int max_entity_id{0};
};
