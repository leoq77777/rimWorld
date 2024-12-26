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

/*test texture
    sf::RenderWindow window(sf::VideoMode(resolution_width, resolution_height), "My Game");
    int x = resolution_width / 2;
    int y = resolution_height / 2;
    sf::Sprite sprite;
    sf::Texture treeTexture;
    if (!treeTexture.loadFromFile("../resources/images/tree.png")) {
        std::cerr << "Failed to load tree texture" << std::endl;
    }
    sprite.setTexture(treeTexture);
    while(window.isOpen()) {
        sf::Event event;
        while(window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
        }
        window.clear();
        window.draw(sprite);
        window.display();
    }
    */