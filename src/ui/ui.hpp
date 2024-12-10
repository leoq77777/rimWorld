#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <queue>
#include <unordered_map>
#include <cmath>
#include <limits>
#include <algorithm>
#include "../world/world.h"
#include "../utils/routing.hpp"
using namespace std;


class ActionMenu {
public:
    ActionMenu();
    ActionMenu(sf::RenderWindow& window) : window(window), is_visible(false) {
        menuBackground.setFillColor(sf::Color(255, 255, 255, 192));
        menuBackground.setSize(sf::Vector2f(200, 100));

        chopTreesButton.setString("Chop Trees (C)");
        createStorageButton.setString("Create Storage (S)");

      

        font.loadFromFile("C:/Windows/Fonts/Arial.ttf");
        chopTreesButton.setFont(font);
        createStorageButton.setFont(font);
        chopTreesButton.setCharacterSize(20);
        createStorageButton.setCharacterSize(20);
        chopTreesButton.setFillColor(sf::Color::Black);
        createStorageButton.setFillColor(sf::Color::Black);

        
    }

    void act_on_ActionMenu() {
        update();
        if (is_visible)
            hide();
        else
            show();
    }

    void show() {
        cout << "show action menu" << endl;
        is_visible = true;
    }

    void hide() {
        cout << "close action menu" << endl;
        is_visible = false;
    }

    void update() {
        auto position = sf::Mouse::getPosition(window);
        chopTreesButton.setPosition(position.x, position.y);
        createStorageButton.setPosition(position.x, position.y + 30);
        menuBackground.setPosition(position.x, position.y);
    }

    void draw() {
        if (is_visible) {
            window.draw(menuBackground);
            window.draw(chopTreesButton);
            window.draw(createStorageButton);
        }
    }

    void handleInput(const sf::Event& event) {
        if (!is_visible) return;

        if (event.type == sf::Event::KeyPressed) {
            if (event.key.code == sf::Keyboard::C) {
                performAction("Chop Trees");
            } else if (event.key.code == sf::Keyboard::S) {
                performAction("Create Storage");
            }
        }
    }

private:
    void performAction(const std::string& action) {
        if (action == "Chop Trees") {
            std::cout << "Marking trees for chopping" << std::endl;
        } else if (action == "Create Storage") {
            std::cout << "Creating storage area." << std::endl;
        }
    }

    bool is_visible;
    sf::RectangleShape menuBackground;
    sf::Text chopTreesButton;
    sf::Text createStorageButton;
    sf::Font font;
    sf::RenderWindow& window;
};


class mainUI {
public:
    mainUI(World& world) : world_(world), 
                        window(sf::VideoMode(MAP_SIZE * TILE_SIZE, MAP_SIZE * TILE_SIZE), "Game Map"), 
                        action_menu_(window) 
    {
        window.setFramerateLimit(60);
        if (!font.loadFromFile("C:/Windows/Fonts/Arial.ttf")) {
            cerr << "Failed to load font!" << endl;
        }
        text.setCharacterSize(TILE_SIZE);
        text.setFont(font);
        text.setFillColor(sf::Color::Black);

        cout << "running UI at world" << endl;
        mark_map_.resize(MAP_SIZE, std::vector<int>(MAP_SIZE, 0));
        is_unreachable = false;
        is_selecting = false;
    }

    void run() {
        sf::Clock clock;
        while (window.isOpen()) {
            handleEvents(clock);
            update(clock);
            draw();
        }
    }

private:
    void handleEvents(sf::Clock& clock) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            else if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Right) {
                sf::Vector2i pixelPos = sf::Mouse::getPosition(window);
                targetGridPos.x = pixelPos.x / TILE_SIZE;
                targetGridPos.y = pixelPos.y / TILE_SIZE;
                targetGridPos.x = max(min(targetGridPos.x, MAP_SIZE - 1), 0);
                targetGridPos.y = max(min(targetGridPos.y, MAP_SIZE - 1), 0);
                std::cout << "target: (" << targetGridPos.x << ", " << targetGridPos.y << ")" << std::endl;
                update_player_loc(clock.restart().asSeconds(), targetGridPos);
            }
            else if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::B) {
                    if (!is_creating_storage) {
                        is_creating_storage = true;
                        std::cout << "start creating storage" << std::endl;
                    } else {
                        is_creating_storage = false;
                        std::cout << "stop creating storage" << std::endl;
                    }
                } else if (event.key.code == sf::Keyboard::Q){
                    action_menu_.act_on_ActionMenu();
                }
            }
            //else if (event.type == sf::Event::MouseButtonEvent && event.key.code == sf::Mouse::left){

            else if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left){
                mousePressedPos = sf::Mouse::getPosition(window);
                is_selecting = true;
                unselect_all_entities();
            }
            else if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left){
                is_selecting = false;
            }
        }
    }

    void update(sf::Clock& clock) {
        update_dog_loc(clock.restart().asSeconds());
    }

    void update_player_loc(float deltaTime, const sf::Vector2i& targetGridPos) {
        for (auto& character : world_.characters_){
            auto& render = world_.component_manager_.get_component<RenderComponent>(character);
            if (!render.is_selected)
                continue;

            auto& playerGridPos = world_.component_manager_.get_component<locationComponent>(character).loc;
            auto& playerRenderPos = render.render_pos;

            auto path = findShortestPath(playerGridPos, targetGridPos, is_unreachable, world_);
            if (is_unreachable){
                cout << "unreachable to target position!" << endl;
                path.clear();
            }

            std::cout << "Character " << character << " is moving from (" << playerGridPos.x << ", " << playerGridPos.y << ") to (" << targetGridPos.x << ", " << targetGridPos.y << ")" << std::endl;
            if (!path.empty() && is_selected) {
                sf::Vector2i nextPos = path.front();
                sf::Vector2f targetRenderPos(static_cast<float>(nextPos.x) * TILE_SIZE, static_cast<float>(nextPos.y) * TILE_SIZE);
                auto distanceX = targetRenderPos.x - playerRenderPos.x;
                auto distanceY = targetRenderPos.y - playerRenderPos.y;
    
                float maxDistance = speed * deltaTime;
                float length = sqrt(distanceX * distanceX + distanceY * distanceY);

                if (length <= maxDistance || (std::abs(distanceX) <= maxDistance && std::abs(distanceY) <= maxDistance)) {
                    playerRenderPos = targetRenderPos;
                    playerGridPos = nextPos;
                    path.erase(path.begin());
                } else {
                    if (length > 0) {
                        playerRenderPos.x += (distanceX / length) * maxDistance;
                        playerRenderPos.y += (distanceY / length) * maxDistance;
                    }
                }
            }
        }
    }


    void update_dog_loc(float deltaTime) {
        std::vector<sf::Vector2i> directions = {
            sf::Vector2i(0, -1), // up
            sf::Vector2i(0, 1),  // down
            sf::Vector2i(-1, 0), // left
            sf::Vector2i(1, 0)   // right
        };

        for (auto entity : world_.get_active_entities()) {
            auto& render = world_.component_manager_.get_component<RenderComponent>(entity);
            if (render.entityType != EntityType::DOG)
                continue;

            auto& location = world_.component_manager_.get_component<locationComponent>(entity).loc;
            float speed = static_cast<float>(rand() % 33);

            int dir = rand() % 4;
            if (location.x == 0 && dir == 0)
                dir = 1;
            if (location.x == MAP_SIZE - 1 && dir == 1)
                dir = 0;
            if (location.y == 0 && dir == 2)
                dir = 3;
            if (location.y == MAP_SIZE - 1 && dir == 3)
                dir = 2;

            sf::Vector2f moveDir = sf::Vector2f(directions[dir]);
            float distance = speed * deltaTime;

            sf::Vector2f newPosition(static_cast<float>(location.x) * TILE_SIZE, static_cast<float>(location.y) * TILE_SIZE);
            newPosition += moveDir * distance;

            newPosition.x = std::clamp(newPosition.x, 0.0f, static_cast<float>(MAP_SIZE - 1) * TILE_SIZE);
            newPosition.y = std::clamp(newPosition.y, 0.0f, static_cast<float>(MAP_SIZE - 1) * TILE_SIZE);

            location.x = static_cast<int>(newPosition.x / TILE_SIZE);
            location.y = static_cast<int>(newPosition.y / TILE_SIZE);

            render.render_pos = newPosition;
            
            std::cout << "Dog " << entity << " moved to (" << location.x << ", " << location.y << ")" << std::endl;
        }
    }

    void draw() {
        window.clear(sf::Color(255, 255, 255));

        //draw map(trees)
        for (int i = 0; i < MAP_SIZE; ++i) {
            for (int j = 0; j < MAP_SIZE; ++j) {
                sf::RectangleShape unit;
                unit.setSize(sf::Vector2f(TILE_SIZE, TILE_SIZE));
                unit.setPosition(i * TILE_SIZE, j * TILE_SIZE);
                unit.setFillColor(sf::Color::Transparent);
                unit.setOutlineColor(sf::Color(210, 180, 140, 32));
                unit.setOutlineThickness(1);
                for(auto entity : world_.entity_map_[i][j]){
                    auto render = world_.component_manager_.get_component<RenderComponent>(entity);
                    if (render.entityType == EntityType::TREE) {
                        text.setString("T");
                    } 
                    else if (render.entityType == EntityType::DOG) {
                        text.setString("D");
                    }
                    else {
                        text.setString(" ");
                    }
                    text.setPosition(i * TILE_SIZE, j * TILE_SIZE);
                    window.draw(text);
                    //draw mark
                    if (render.is_selected){
                        sf::CircleShape circle;
                        circle.setRadius(TILE_SIZE / 4);
                        circle.setPosition(i * TILE_SIZE + TILE_SIZE / 4, j * TILE_SIZE + TILE_SIZE / 4 );
                        circle.setFillColor(sf::Color(255, 255, 255, 0));
                        circle.setOutlineColor(sf::Color::Red);
                        circle.setOutlineThickness(1);
                        window.draw(circle);
                    }
                    //draw storage
                    if (render.entityType == EntityType::STORAGE) {
                        unit.setFillColor(sf::Color(0, 0, 255, 128));
                    } 
                    
                    
                }
                window.draw(unit);
            }
        }

        //draw characters
        for (auto& character : world_.characters_){
            text.setString("P");
            auto& render = world_.component_manager_.get_component<RenderComponent>(character);
            text.setPosition(render.render_pos.x, render.render_pos.y);
            window.draw(text);
        }

        if (is_selecting){
            mouseCurrentPos = sf::Mouse::getPosition(window);
            if (is_creating_storage){
                world_.make_storage_area(mousePressedPos, mouseCurrentPos);
            } else {
                select_entities_in_area(mousePressedPos, mouseCurrentPos);
            }
        }

        //draw selection area
        if (1){
            float width = abs(mouseCurrentPos.x - mousePressedPos.x);
            float height = abs(mouseCurrentPos.y - mousePressedPos.y);
            float left = min(mousePressedPos.x, mouseCurrentPos.x);
            float top = min(mousePressedPos.y, mouseCurrentPos.y);
            rect.setPosition(left, top);
            rect.setSize(sf::Vector2f(width, height));
            rect.setOutlineColor(sf::Color::Green);
            rect.setOutlineThickness(2);
            rect.setFillColor(sf::Color(255, 255, 255, 128));
            window.draw(rect);
        }
        
        //draw action menu
        action_menu_.draw();

        //display window
        window.display();
    }

    void unselect_all_entities() {
        for (int i = 0; i < MAP_SIZE; ++i) {
            for (int j = 0; j < MAP_SIZE; ++j) {
                for(auto entity : world_.entity_map_[i][j]){
                    auto& render = world_.component_manager_.get_component<RenderComponent>(entity);
                    render.is_selected = false;
                    mark_map_[i][j] = 0;
                }
            }
        }
    }

    void select_entities_in_area(sf::Vector2i start, sf::Vector2i end) {
        int minX = std::min(start.x, end.x) / TILE_SIZE;
        int maxX = std::max(start.x, end.x) / TILE_SIZE;
        int minY = std::min(start.y, end.y) / TILE_SIZE;
        int maxY = std::max(start.y, end.y) / TILE_SIZE;
        for(int i = minX; i <= maxX; ++i) {
            for(int j = minY; j <= maxY; ++j) {
                for(auto entity : world_.entity_map_[i][j]){
                    auto& render = world_.component_manager_.get_component<RenderComponent>(entity);
                    const auto& loc = world_.component_manager_.get_component<locationComponent>(entity).loc;
                    if (!render.is_selected){
                        render.is_selected = true;
                        std::cout << "Entity " << entity << " is selected" << std::endl;
                        mark_map_[loc.x][loc.y] = 1;
                    } 
                }
            }
        }
    }

    std::vector<std::vector<int>> mark_map_;
    World& world_;
    sf::RenderWindow window;
    ActionMenu action_menu_;
    sf::Font font;
    sf::Text text;
    sf::Vector2i targetGridPos;
    sf::Vector2i mousePressedPos;
    sf::Vector2i mouseCurrentPos;
    sf::RectangleShape rect;
    bool is_unreachable;
    bool is_selecting;
    bool is_selected;
    bool is_creating_storage;
    float speed = 64.0;
};