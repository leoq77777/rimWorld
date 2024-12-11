#pragma once
#include "world/world.hpp"
#include <SFML/Graphics.hpp>
#include <iostream>
class UI {
public:
    UI(World& world) : world_(world), window(sf::VideoMode(800, 600), "My Game") {
        if (!font.loadFromFile("C:/Windows/Fonts/Arial.ttf")) {
            std::cerr << "Failed to load font!" << std::endl;
        }
    }
    void run();
    void draw();
private:
    World& world_;
    sf::RenderWindow window;
    sf::Font font;
};

void UI::run() {
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }
        draw();
    }
}

void UI::draw() {
    window.clear(sf::Color(128, 128, 128));
    for(auto& entity : world_.get_all_entities()) {
        auto& type = world_.component_manager_.get_component<RenderComponent>(entity).entityType;
        auto& pos = world_.component_manager_.get_component<LocationComponent>(entity).loc;

        sf::Text text;
        text.setCharacterSize(TILE_SIZE);
        text.setFont(font);
        text.setPosition(pos.x * TILE_SIZE, pos.y * TILE_SIZE);
        text.setFillColor(sf::Color::Red);
        if (type == EntityType::TREE) {
            text.setString("T");
        } else if (type == EntityType::DOG) {
            text.setString("D");
        } else if (type == EntityType::CHARACTER) {
            text.setString("C");
        }

        window.draw(text);
    }
    window.display();
}

