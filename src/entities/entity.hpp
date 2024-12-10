#ifndef ENTITY_HPP
#define ENTITY_HPP
#include "../components/componentManager.hpp"
#include <queue>
#include <bitset>
#include <iostream>
#include <fstream>
class EntityManager {  
public:
    EntityManager() {
        //read from save files
        for (Entity i = 0; i < MAX_ENTITIES; ++i) {
            available_entities.push(i);
        }
    }
    Entity create_entity() {
        assert(living_entity_count < MAX_ENTITIES && "Too many entities in existence.");
        Entity id = available_entities.front();
        available_entities.pop();
        ++living_entity_count;
        entity_used.set(id);
        return id;
    }
    void destroy_entity(Entity entity) {
        available_entities.push(entity);
        --living_entity_count;
        entity_used.reset(entity);
    }
    std::vector<Entity> get_all_entities() const {
        std::vector<Entity> entities;
        for (Entity i = 0; i < MAX_ENTITIES; ++i) {
            if (entity_used[i]) {
                entities.push_back(i);
            }
        }
        return entities;
    }

    bool is_entity_alive(Entity entity) {
        return entity_used.test(entity);
    }

    void save() {
        std::ofstream file("entity.txt");
        for (Entity i = 0; i < MAX_ENTITIES; ++i) {
            if (entity_used[i]) {
                file << i << std::endl;
            }
        }
    }

private:
    Entity next_entity{0};
    std::uint32_t living_entity_count{0};
    std::queue<Entity> available_entities;
    std::bitset<MAX_ENTITIES> entity_used;
};
#endif //ENTITY_HPP
