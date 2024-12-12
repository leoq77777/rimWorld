#pragma once
#include <typeindex>
#include "componentArray.hpp"
#include <unordered_map>
#include <memory>
#include <cassert>

    #include <iostream>
    class ComponentManager {
    public:
        ComponentManager() {
            //read from save files
        }
        template<typename T>
        void register_component() {
            std::type_index type_name = typeid(T);
            assert(component_types.find(type_name) == component_types.end() && "Registering component type more than once.");
            component_types.insert({type_name, next_component_type});
            component_arrays.insert({type_name, std::make_shared<ComponentArray<T>>()});
            ++next_component_type;
        }

        template<typename T>
        ComponentType get_component_type() {
            std::type_index type_name = typeid(T);
            assert(component_types.find(type_name) != component_types.end() && "Component not registered before use.");
            return component_types[type_name];
        }

        template<typename T>
        void add_component(Entity entity, T component) {
            get_component_array<T>()->insert_data(entity, component);
        }

        template<typename T>
        void remove_component(Entity entity) {
            get_component_array<T>()->remove_data(entity);
        }

        template<typename T>
        T& get_component(Entity entity) {
            return get_component_array<T>()->get_data(entity);
        }

        void entity_destroy(Entity entity) {
            for (auto const& pair : component_arrays) {
                auto const& component = pair.second;
                component->entity_destroy(entity);
            }
        }

        void list_all_components() {
            for (auto const& pair : component_arrays) {
                auto const& component = pair.second;
                auto type = component_types[pair.first];
                std::cout << "Component type: " << type << std::endl;
            }
        }

        template<typename T>
        bool has_component(Entity entity) {
            return get_component_array<T>()->has_data(entity);
        }
        
    private:
        std::unordered_map<std::type_index, ComponentType> component_types{};
        std::unordered_map<std::type_index, std::shared_ptr<IComponentArray>> component_arrays{};
        ComponentType next_component_type{1};
        template<typename T>
        std::shared_ptr<ComponentArray<T>> get_component_array() {
            std::type_index type_name = typeid(T);
            assert(component_types.find(type_name) != component_types.end() && "Component not registered before use.");
            return std::static_pointer_cast<ComponentArray<T>>(component_arrays[type_name]);
        }
    };
