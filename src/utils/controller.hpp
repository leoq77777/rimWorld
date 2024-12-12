#pragma once
#include <string>
#include <vector>
#include <sstream>
#include "../entities/entity.hpp"
using namespace std;
class Controller {
    EntityManager& entity_manager_;
    ComponentManager& component_manager_;
public:
    Controller(EntityManager& entity_manager, ComponentManager& component_manager) : 
        entity_manager_(entity_manager), 
        component_manager_(component_manager) {
            cout << "Controller initialized" << endl;
        }

    vector<string> parse_command(string command) {
        vector<string> command_list;
        stringstream ss(command);
        string item;
        while (getline(ss, item, ' ')) {
            command_list.push_back(item);
        }
        return command_list;
    }

    void exeucate_command(vector<string> command_list) {
        if (command_list[0] == "create" && command_list.size() == 5 && command_list[2] == "-p") {
            int x = stoi(command_list[3]);
            int y = stoi(command_list[4]);
            Location loc = {x, y};
            string entity_type = command_list[1];
            if (entity_type == "character") {
                cout << "Controller: create character at (" << x << ", " << y << ")" << endl;
                Entity temp_character = entity_manager_.create_entity();
                component_manager_.add_component(temp_character, LocationComponent{loc});
                component_manager_.add_component(temp_character, CreateComponent{true, 1, EntityType::CHARACTER});
            }
        }
    }

};