#include <iostream>
#include "ui/ui.hpp"
#include "world/world.hpp"


int main() {
    std::cout << "Hello, World!" << std::endl;
    World world;
    //mainUI ui(world);
    //ui.run();
    int wait;
    for(auto& entity : world.get_all_entities()) {
        std::cout << "Entity " << entity << " is alive" << std::endl;
    }
    
    UI ui(world);
    ui.run();
    std::cin >> wait;
    return 0;
}