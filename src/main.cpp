#include <iostream>
#include "ui/ui.hpp"
#include <sstream>
int main() {
    std::cout << "Game initializing..." << std::endl;

    bool is_fullscreen = true;
    int custom_width = 1280, custom_height = 720;
    
    try {
        UI ui(custom_width, custom_height, is_fullscreen);
        ui.setFrameRate(30);
        ui.run();        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "Game terminated normally" << std::endl;
    return 0;
}

