#include <iostream>
#include "ui/ui.hpp"

int main() {
    std::cout << "Game initializing..." << std::endl;
    
    // 初始化随机数生成器
    srand(time(nullptr));
    
    try {
        // 创建世界
        World world;
        std::cout << "World created successfully" << std::endl;
        
        // 打印初始实体信息
        for(auto& entity : world.get_all_entities()) {
            std::cout << "Entity " << entity << " is alive" << std::endl;
        }
        
        // 创建并运行UI
        UI ui(world);
        ui.run();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "Game terminated normally" << std::endl;
    return 0;
}