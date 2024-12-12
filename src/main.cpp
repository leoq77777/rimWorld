#include <iostream>
#include "ui/ui.hpp"
#include <sstream>

int main() {
    std::cout << "Game initializing..." << std::endl;

    bool is_fullscreen = false;
    sf::VideoMode desktopMode = sf::VideoMode::getDesktopMode();
    int custom_width = 1280;
    int custom_height = 720;
    int resolution_width = is_fullscreen ? desktopMode.width : custom_width;
    int resolution_height = is_fullscreen ? desktopMode.height : custom_height;
    
    // 初始化随机数生成器
    srand(time(nullptr));
    
    try {
        // 创建世界
        World world;
        std::cout << "World created successfully" << std::endl;
        
        world.router_.print_all_live_entities();
        
        UI ui(world, resolution_width, resolution_height);
        ui.run();

        string command;
        while (getline(cin, command)) {
            auto command_list = world.controller_.parse_command(command);
            world.controller_.exeucate_command(command_list);
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "Game terminated normally" << std::endl;
    return 0;
}