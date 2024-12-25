#pragma once
#include "component.hpp"
#include <cassert>
#include <unordered_map>
#include "../lib/nlohmann/json.hpp"
#include <fstream>

#define MAX_ENTITIES 10000

class IComponentArray
{
public:
	virtual ~IComponentArray() = default;
	virtual void entity_destroy(Entity entity) = 0;
	virtual void save() const = 0;
	virtual void load() = 0;
};


template<typename T>
class ComponentArray : public IComponentArray
{
public:
	void insert_data(Entity entity, T component)
	{
		assert(entity_to_index.find(entity) == entity_to_index.end() && "Component added to same entity more than once.");

		size_t new_index = array_size;
		entity_to_index[entity] = new_index;
		index_to_entity[new_index] = entity;
		component_array[new_index] = component;
		++array_size;
	}

	void remove_data(Entity entity)
	{
		assert(entity_to_index.find(entity) != entity_to_index.end() && "Removing non-existent component.");

		size_t index_of_removed_entity = entity_to_index[entity];
		size_t index_of_last_element = array_size - 1;
		component_array[index_of_removed_entity] = component_array[index_of_last_element];
        
		Entity entity_of_last_element = index_to_entity[index_of_last_element];
		entity_to_index[entity_of_last_element] = index_of_removed_entity;
		index_to_entity[index_of_removed_entity] = entity_of_last_element;

        component_array[index_of_last_element] = T();
		entity_to_index.erase(entity);
		index_to_entity.erase(index_of_last_element);
		--array_size;
	}

	T& get_data(Entity entity)
	{	
		assert(entity_to_index.find(entity) != entity_to_index.end() && "Retrieving non-existent component.");
		return component_array[entity_to_index[entity]];
	}

	void entity_destroy(Entity entity) override
	{
		if (entity_to_index.find(entity) != entity_to_index.end())
		{
			remove_data(entity);
		}
	}

	bool has_data(Entity entity) {
		return entity_to_index.find(entity) != entity_to_index.end();
	}

	void save() const {
		std::string filename = "../saves/components/" + std::string(typeid(T).name()) + ".json";
		nlohmann::json j;
		for (const auto& pair : entity_to_index) {
            Entity entity = pair.first;
            size_t index = pair.second;
            j[std::to_string(entity)] = component_array[index];
        }

        std::ofstream file(filename);
        if (file.is_open()) {
            file << j.dump(4); 
            file.close();
        } else {
            std::cerr << "Error: Could not open file for saving: " << filename << std::endl;
        }
	}

	void load() {
		std::string filename = "../saves/components/" + std::string(typeid(T).name()) + ".json";
		nlohmann::json j;

		std::ifstream file(filename);
        if (file.is_open()) {
            file >> j;
            file.close();

            entity_to_index.clear();
            index_to_entity.clear();
            array_size = 0;

            for (auto it = j.begin(); it != j.end(); ++it) {
                Entity entity = std::stoi(it.key());
                T component = it.value().get<T>();
                insert_data(entity, component);
            }
        } else {
            std::cerr << "Error: Could not open file for loading: " << filename << std::endl;
        }
	}

private:
	// The packed array of components (of generic type T),
	// set to a specified maximum amount, matching the maximum number
	// of entities allowed to exist simultaneously, so that each entity
	// has a unique spot.
	std::array<T, MAX_ENTITIES> component_array;

	// Map from an entity ID to an array index.
	std::unordered_map<Entity, size_t> entity_to_index;

	// Map from an array index to an entity ID.
	std::unordered_map<size_t, Entity> index_to_entity;

	// Total size of valid entries in the array.
	size_t array_size;
};
