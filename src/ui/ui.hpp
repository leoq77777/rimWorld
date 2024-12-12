#pragma once
#include "world/world.hpp"
#include <cmath>
#include <SFML/Graphics.hpp>
#include <thread>
#include <chrono>

class SelectionMenu {
    sf::RenderWindow& window_;
    sf::RectangleShape menuBackground;
    sf::Text buildStorageBtn;
    sf::Text markTreesBtn;
    sf::Font font;
    bool isVisible_ = false;
public:
    SelectionMenu(sf::RenderWindow& window) : window_(window) {
        if (!font.loadFromFile("C:/Windows/Fonts/Arial.ttf")) {
            std::cerr << "Failed to load font!" << std::endl;
        }
        menuBackground.setFillColor(sf::Color(255, 255, 255, 230));
        menuBackground.setSize(sf::Vector2f(150, 80));
        setupButton(buildStorageBtn, "Set Storage Area (1)", 0);
        setupButton(markTreesBtn, "Mark Trees (2)", 1);
    }

    void show(sf::Vector2i position) {
        isVisible_ = true;
        menuBackground.setPosition(position.x, position.y);
        buildStorageBtn.setPosition(position.x + 10, position.y + 10);
        markTreesBtn.setPosition(position.x + 10, position.y + 45);
    }

    void hide() { 
        isVisible_ = false; 
    
    }
    bool isVisible() const {    
        return isVisible_; 
    }
    
    void draw() {
        if (!isVisible_) return;
        window_.draw(menuBackground);
        window_.draw(buildStorageBtn);
        window_.draw(markTreesBtn);
    }

    bool isBuildStorageClicked(sf::Vector2i mousePos) {
        return isVisible_ && buildStorageBtn.getGlobalBounds().contains(mousePos.x, mousePos.y);
    }

    bool notBuildStorageClicked(sf::Vector2i mousePos) {
        return isVisible_ && !buildStorageBtn.getGlobalBounds().contains(mousePos.x, mousePos.y);
    }

    bool isMarkTreesClicked(sf::Vector2i mousePos) {
        return isVisible_ && markTreesBtn.getGlobalBounds().contains(mousePos.x, mousePos.y);
    }

    bool notMarkTreesClicked(sf::Vector2i mousePos) {
        return isVisible_ && !markTreesBtn.getGlobalBounds().contains(mousePos.x, mousePos.y);
    }

    bool handleClick(sf::Vector2i mousePos) {
        if (!isVisible_) return false;
        
        if (isBuildStorageClicked(mousePos) || isMarkTreesClicked(mousePos)) {
            return true;
        }
        return false;
    }

private:
    void setupButton(sf::Text& btn, const std::string& str, int index) {
        btn.setFont(font);
        btn.setString(str);
        btn.setCharacterSize(16);
        btn.setFillColor(sf::Color::Black);
    }
};

class BuildMenu {
    sf::RenderWindow& window_;
    sf::RectangleShape menuBackground;
    sf::Text buildDoorBtn;
    sf::Text buildWallBtn;
    sf::RectangleShape doorRect; 
    sf::RectangleShape wallRect;
    sf::Font font;
    bool isVisible_ = false;
public:
    BuildMenu(sf::RenderWindow& window) : window_(window) {
        if (!font.loadFromFile("C:/Windows/Fonts/Arial.ttf")) {
            std::cerr << "Failed to load font!" << std::endl;
        }
        menuBackground.setFillColor(sf::Color(255, 255, 255, 230));
        menuBackground.setSize(sf::Vector2f(150, 80));
        setupButton(buildDoorBtn, doorRect, "  Build Door (3)  ", 0);
        setupButton(buildWallBtn, wallRect, "  Build Wall (4)  ", 1);
    }

    void show(sf::Vector2i position) {
        isVisible_ = true;
        menuBackground.setPosition(position.x, position.y);
        
        adjustButtonPositions(position);    
    }

    void hide() { isVisible_ = false; }
    bool isVisible() const { return isVisible_; }
    
    void draw() {
        if (!isVisible_) return;
        window_.draw(menuBackground);
        window_.draw(buildDoorBtn);
        window_.draw(doorRect);
        window_.draw(buildWallBtn);
        window_.draw(wallRect);
    }

    bool isBuildDoorClicked(sf::Vector2i mousePos) {
        return isVisible_ && doorRect.getGlobalBounds().contains(mousePos.x, mousePos.y);
    }

    bool notBuildDoorClicked(sf::Vector2i mousePos) {
        return isVisible_ && !doorRect.getGlobalBounds().contains(mousePos.x, mousePos.y);
    }

    bool isBuildWallClicked(sf::Vector2i mousePos) {
        return isVisible_ && wallRect.getGlobalBounds().contains(mousePos.x, mousePos.y);
    }

    bool notBuildWallClicked(sf::Vector2i mousePos) {
        return isVisible_ && !wallRect.getGlobalBounds().contains(mousePos.x, mousePos.y);
    }

    bool handleClick(sf::Vector2i mousePos) {
        if (!isVisible_) return false;
        
        if (isBuildDoorClicked(mousePos) || isBuildWallClicked(mousePos)) {
            return true;
        }
        return false;
    }

private:
    void setupButton(sf::Text& btn, sf::RectangleShape& rect, const std::string& str, int index) {
        btn.setFont(font);
        btn.setString(str);
        btn.setCharacterSize(16);
        btn.setFillColor(sf::Color::Black);

        rect.setFillColor(sf::Color(0, 0, 0, 0)); 
        rect.setOutlineThickness(1); 
        //rect.setOutlineColor(sf::Color::Blue);

        sf::FloatRect textRect = btn.getLocalBounds();
        rect.setSize(sf::Vector2f(textRect.width + 20, textRect.height + 10)); 
    }

    void adjustButtonPositions(sf::Vector2i position) {
        buildDoorBtn.setPosition(position.x + 10, position.y + 10);
        buildWallBtn.setPosition(position.x + 10, position.y + 45);

        doorRect.setPosition(position.x + 10, position.y + 10);
        wallRect.setPosition(position.x + 10, position.y + 45);
    }
};

class UI {
public:
    UI(World& world, int window_width, int window_height) : 
        world_(world), 
        window(sf::VideoMode(window_width, window_height), "My Game"),
        selectionMenu(window), 
        buildMenu(window),       
        view(sf::FloatRect(0, 0, window_width, window_height)),
        window_width_(window_width),
        window_height_(window_height)
    {
        if (!font.loadFromFile("C:/Windows/Fonts/Arial.ttf")) {
            std::cerr << "Failed to load font!" << std::endl;
        }
        window.setView(view);

        message.setFont(font);
        message.setCharacterSize(24);
        message.setFillColor(sf::Color(255, 0, 0, 128));
        message.setPosition(window_width_ / 2.0f - 100, 50); 
        message.setString("");
    }

    void run() {
        sf::Clock clock;
        float deltaTime = 0.0f;
        int round = 0;
        while (window.isOpen()) {
            std::cout << "\n\nROUND: " << ++round << std::endl;
            handleEvents(clock);
            world_.update_world();
            updateMessage();
            //updateViewPosition(deltaTime);

            draw();

            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }

    void draw() {
        window.clear(sf::Color(128, 128, 128));  // 灰色背景
        
        // 绘制网格线
        drawGrid();
        
        // 渲染所有实体
        for(auto& entity : world_.router_.get_all_entities()) {
            drawEntity(entity);
        }
        
        // 绘制选择框
        if (is_selecting) {
            mouseCurrentPos = sf::Mouse::getPosition(window);
            float width = abs(mouseCurrentPos.x - mousePressedPos.x);
            float height = abs(mouseCurrentPos.y - mousePressedPos.y);
            float left = std::min(mousePressedPos.x, mouseCurrentPos.x);
            float top = std::min(mousePressedPos.y, mouseCurrentPos.y);
            
            sf::RectangleShape selectionRect;
            selectionRect.setPosition(left, top);
            selectionRect.setSize(sf::Vector2f(width, height));
            selectionRect.setOutlineColor(sf::Color::Green);
            selectionRect.setOutlineThickness(2);
            selectionRect.setFillColor(sf::Color(255, 255, 255, 128));
            window.draw(selectionRect);
        }
        
        
        selectionMenu.draw();
        buildMenu.draw();

        if (messageVisible) {
            window.draw(message);
        }

        window.display();
    }

private:
    void drawGrid();
    void drawEntity(Entity entity);
    void drawMark(Entity entity);
    void drawStorage(Location loc);
    void drawWood(Location loc);
    float smoothstep(float x);
    float lerp(float a, float b, float t);
    void drawSelectionHighlight(float x, float y);
    bool isSelected(Entity entity) { 
        auto& render = world_.component_manager_.get_component<RenderComponent>(entity);
        return render.is_selected;
    }
    void unselect_all_entities() {
        for(auto& entity : world_.router_.get_all_entities()) {
            if (!world_.component_manager_.has_component<RenderComponent>(entity)) {
                continue;
            }
            auto& render = world_.component_manager_.get_component<RenderComponent>(entity);
            render.is_selected = false;
        }
    }

    void select_entities_in_area(sf::Vector2i start, sf::Vector2i end) {
        int minX = std::min(start.x, end.x) / TILE_SIZE;
        int maxX = std::max(start.x, end.x) / TILE_SIZE;
        int minY = std::min(start.y, end.y) / TILE_SIZE;
        int maxY = std::max(start.y, end.y) / TILE_SIZE;
        
        for(auto& entity : world_.router_.get_all_entities()) {
            if (!world_.component_manager_.has_component<RenderComponent>(entity)) {
                continue;
            }
            auto& render = world_.component_manager_.get_component<RenderComponent>(entity);
            auto& loc = world_.component_manager_.get_component<LocationComponent>(entity).loc;
            
            if (loc.x >= minX && loc.x <= maxX && 
                loc.y >= minY && loc.y <= maxY) {
                render.is_selected = true;
                std::cout << "Entity " << entity << " is selected" << std::endl;
            }
        }
    }

    void handleEvents(sf::Clock& clock) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
            else if (event.type == sf::Event::MouseButtonPressed) {
                handleMousePress(event);
            }
            else if (event.type == sf::Event::MouseButtonReleased) {
                handleMouseRelease(event);
            }
            else if (event.type == sf::Event::KeyPressed) {
                handleKeyPress(event.key.code);
                if (event.key.code == sf::Keyboard::W || 
                    event.key.code == sf::Keyboard::A || 
                    event.key.code == sf::Keyboard::S || 
                    event.key.code == sf::Keyboard::D) {
                    keyStates[event.key.code] = true; // 记录按键状态
                }
            }
            else if (event.type == sf::Event::KeyReleased) {
                if (event.key.code == sf::Keyboard::W || 
                    event.key.code == sf::Keyboard::A || 
                    event.key.code == sf::Keyboard::S || 
                    event.key.code == sf::Keyboard::D) {
                    keyStates[event.key.code] = false; // 记录按键状态
                }
            }
        }
    }

    void handleMousePress(const sf::Event& event) {
        if (event.mouseButton.button == sf::Mouse::Left) {
            sf::Vector2i mousePos = sf::Mouse::getPosition(window);
            
            if (selectionMenu.handleClick(mousePos)) {
                handleSelectionMenuClick(mousePos);
                return;
            }
            if (buildMenu.handleClick(mousePos)) {
                handleBuildMenuClick(mousePos);
                return;
            }
            
            mousePressedPos = mousePos;
            is_selecting = true;
            unselect_all_entities();
            
            if (selectionMenu.notBuildStorageClicked(mousePos) && selectionMenu.notMarkTreesClicked(mousePos))
                selectionMenu.hide();
            if (buildMenu.notBuildDoorClicked(mousePos) && buildMenu.notBuildWallClicked(mousePos))
                buildMenu.hide();
        }
        else if (event.mouseButton.button == sf::Mouse::Right) {
            rightClickPos = sf::Mouse::getPosition(window);
            buildMenu.show(rightClickPos);
            selectionMenu.hide();
        }
    }

    void handleMouseRelease(const sf::Event& event) {
        if (event.mouseButton.button == sf::Mouse::Left) {
            is_selecting = false;
            mouseCurrentPos = sf::Mouse::getPosition(window);
            select_entities_in_area(mousePressedPos, mouseCurrentPos);
            
            selectionMenu.show(mouseCurrentPos);
        }
    }

    void handleSelectionMenuClick(sf::Vector2i mousePos) {
        if (selectionMenu.isBuildStorageClicked(mousePos)) {
            std::cout << "Building storage area..." << std::endl;
            Location start = {mousePressedPos.x / TILE_SIZE, mousePressedPos.y / TILE_SIZE};
            Location end = {mouseCurrentPos.x / TILE_SIZE, mouseCurrentPos.y / TILE_SIZE};
            if(world_.make_storage_area(start, end)) {
                showMessage("Storage area built");
            }
            else {
                showMessage("Storage construction failed");
            }
        }
        else if (selectionMenu.isMarkTreesClicked(mousePos)) {
            std::cout << "Marking trees..." << std::endl;
            if (world_.mark_tree()) {
                showMessage("Trees marked");
            }
            else {
                showMessage("No trees to mark");
            }
        }
        
        selectionMenu.hide();
    }

    void handleBuildMenuClick(sf::Vector2i mousePos) {
        Location buildPos = {rightClickPos.x / TILE_SIZE, rightClickPos.y / TILE_SIZE};
        
        if (buildMenu.isBuildDoorClicked(mousePos)) {
            std::cout << "Building door..." << std::endl;
            if (world_.set_door_blueprint(buildPos)) {
                showMessage("Door blueprint has been set");
            }
            else {
                showMessage("Door blueprint construction failed");
            }
        }
        else if (buildMenu.isBuildWallClicked(mousePos)) {
            std::cout << "Building wall..." << std::endl;
            if (world_.set_wall_blueprint(buildPos)) {
                showMessage("Wall blueprint has been set");
            }
            else {
                showMessage("Wall blueprint construction failed");
            }
        }
        
        buildMenu.hide();
    }

    void handleKeyPress(sf::Keyboard::Key key) {
        switch (key) {
            case sf::Keyboard::Num1:
                if (selectionMenu.isVisible()) {
                    std::cout << "Building storage area..." << std::endl;
                    //world_.make_storage_area(mousePressedPos, mouseCurrentPos);
                    selectionMenu.hide();
                }
                break;
            case sf::Keyboard::Num2:
                if (selectionMenu.isVisible()) {
                    std::cout << "Marking trees..." << std::endl;
                    //world_.mark_tree();
                    selectionMenu.hide();
                }
                break;
            case sf::Keyboard::Num3:
                if (buildMenu.isVisible()) {
                    std::cout << "Building door..." << std::endl;
                    //world_.set_door_blueprint(rightClickPos);
                    buildMenu.hide();
                }
                break;
            case sf::Keyboard::Num4:
                if (buildMenu.isVisible()) {
                    std::cout << "Building wall..." << std::endl;
                    //world_.set_wall_blueprint(rightClickPos);
                    buildMenu.hide();
                }
                break;
            case sf::Keyboard::Escape:
                selectionMenu.hide();
                buildMenu.hide();
                break;
        }
    }

    void updateViewPosition(float deltaTime);

    World& world_;
    int window_width_;
    int window_height_;
    sf::RenderWindow window;
    sf::Font font;
    SelectionMenu selectionMenu;
    BuildMenu buildMenu;
    sf::Vector2i mousePressedPos;
    sf::Vector2i mouseCurrentPos;
    sf::Vector2i rightClickPos;
    bool is_selecting = false;
    sf::View view;
    float viewMoveSpeed = 200.0f; 
    std::unordered_map<int, bool> keyStates; 
    sf::Text message;
    bool messageVisible = false;
    sf::Clock messageTimer;

    bool hasSelectedEntities() {
        for(auto& entity : world_.router_.get_all_entities()) {
            if (isSelected(entity)) {
                return true;
            }
        }
        return false;
    }

    void showMessage(const std::string& text) {
        message.setString(text);
        message.setPosition(window_width_ / 2.0f - message.getGlobalBounds().width / 2.0f, 50); 
        messageVisible = true;
        messageTimer.restart(); 
    }

    void updateMessage() {
        if (messageVisible && messageTimer.getElapsedTime().asSeconds() >= 2.0f) {
            messageVisible = false;
            message.setString(""); 
        }
    }
};

void UI::drawGrid() {
    sf::RectangleShape line(sf::Vector2f(window_width_, 1));
    line.setFillColor(sf::Color(100, 100, 100)); 
    
    for(int y = 0; y <= window_height_; y += TILE_SIZE) {
        line.setPosition(0, y);
        window.draw(line);
    }
    
    line.setSize(sf::Vector2f(1, window_height_));
    for(int x = 0; x <= window_width_; x += TILE_SIZE) {
        line.setPosition(x, 0);
        window.draw(line);
    }
}

void UI::drawEntity(Entity entity) {
    auto& type = world_.component_manager_.get_component<RenderComponent>(entity).entityType;
    auto& pos = world_.component_manager_.get_component<LocationComponent>(entity).loc;
    
    float render_x = pos.x;
    float render_y = pos.y;

    if (world_.component_manager_.has_component<MovementComponent>(entity)) {
        auto& movement = world_.component_manager_.get_component<MovementComponent>(entity);
        float t = smoothstep(movement.progress);
        render_x = lerp(movement.start_pos.x, movement.end_pos.x, t);
        render_y = lerp(movement.start_pos.y, movement.end_pos.y, t);
    }

    float screen_x = render_x * TILE_SIZE;
    float screen_y = render_y * TILE_SIZE;

    sf::Text text;
    text.setFont(font);
    text.setCharacterSize(TILE_SIZE);
    text.setPosition(screen_x, screen_y);

    switch(type) {
        case EntityType::CHARACTER:
            text.setString("C");
            text.setFillColor(sf::Color::Blue);
            if (world_.component_manager_.has_component<MovementComponent>(entity)) {
                auto& movement = world_.component_manager_.get_component<MovementComponent>(entity);
                float bounce = sin(movement.progress * M_PI * 2) * 2.0f;
                text.setPosition(screen_x, screen_y - bounce);
            }
            break;
        case EntityType::DOG:
            text.setString("A");
            text.setFillColor(sf::Color(139, 69, 19));
            if (world_.component_manager_.has_component<MovementComponent>(entity)) {
                auto& movement = world_.component_manager_.get_component<MovementComponent>(entity);
                float bounce = sin(movement.progress * M_PI) * 4.0f;
                text.setPosition(screen_x, screen_y - bounce);
            }
            break;
        case EntityType::TREE:
            text.setString("T");
            text.setFillColor(sf::Color::Green);
            drawMark(entity);
            break;
        //if it is blueprint, add something
        case EntityType::WALL:
            text.setString("W");
            text.setFillColor(sf::Color(139, 69, 19));
            break;
        case EntityType::DOOR:
            text.setString("D");
            text.setFillColor(sf::Color(139, 69, 19));
            break;
        case EntityType::STORAGE:
            text.setString(" ");
            text.setFillColor(sf::Color(139, 69, 19));
            drawStorage(pos);
            break;
        case EntityType::WOODPACK:
            text.setString("R");
            text.setFillColor(sf::Color(139, 69, 19));
            break;
    }

    // 绘制选中效果
    if (isSelected(entity)) {
        drawSelectionHighlight(screen_x, screen_y);
    }

    window.draw(text);
}

void UI::drawStorage(Location loc) {
    sf::RectangleShape rect(sf::Vector2f(TILE_SIZE, TILE_SIZE));
    rect.setFillColor(sf::Color(0, 0, 255, 64));
    rect.setPosition(loc.x * TILE_SIZE, loc.y * TILE_SIZE);
    window.draw(rect);
}

void UI::drawWood(Location loc) {
    sf::Text wood_text;
    wood_text.setFont(font);
    wood_text.setCharacterSize(TILE_SIZE);
    wood_text.setPosition(loc.x * TILE_SIZE, loc.y * TILE_SIZE + TILE_SIZE / 2);
    wood_text.setString("5 woods");
    wood_text.setFillColor(sf::Color(139, 69, 19));
    window.draw(wood_text);
}

void UI::drawMark(Entity entity) {
    if(!world_.component_manager_.has_component<TargetComponent>(entity)) {
        return;
    }

    auto& target = world_.component_manager_.get_component<TargetComponent>(entity);
    if (target.is_target) {
        auto& pos = world_.component_manager_.get_component<LocationComponent>(entity).loc;
        float screen_x = pos.x * TILE_SIZE + TILE_SIZE / 4;
        float screen_y = pos.y * TILE_SIZE + TILE_SIZE / 4;
        sf::CircleShape circle(TILE_SIZE / 4);
        circle.setFillColor(sf::Color(0, 255, 0, 32));
        circle.setOutlineColor(sf::Color(0, 255, 0, 128));
        circle.setOutlineThickness(1);
        circle.setPosition(screen_x, screen_y);
        window.draw(circle);
    }
}

// 辅助函数：平滑插值
float UI::smoothstep(float x) {
    // 使用三次平滑插值
    return x * x * (3 - 2 * x);
}

// 辅助函数：线性插值
float UI::lerp(float a, float b, float t) {
    return a + (b - a) * t;
}

// 绘制选中效果
void UI::drawSelectionHighlight(float x, float y) {
    sf::RectangleShape highlight;
    highlight.setSize(sf::Vector2f(TILE_SIZE, TILE_SIZE));
    highlight.setPosition(x, y);
    highlight.setFillColor(sf::Color::Transparent);
    highlight.setOutlineColor(sf::Color(128, 128, 128, 64));
    highlight.setOutlineThickness(1);
    window.draw(highlight);
}

void UI::updateViewPosition(float deltaTime) {
    sf::Vector2f moveDirection(0, 0);
    float speed = viewMoveSpeed * deltaTime;

    if (keyStates[sf::Keyboard::W]) { moveDirection.y -= speed; }
    if (keyStates[sf::Keyboard::S]) { moveDirection.y += speed; }
    if (keyStates[sf::Keyboard::A]) { moveDirection.x -= speed; }
    if (keyStates[sf::Keyboard::D]) { moveDirection.x += speed; }

    // 更新视图位置
    view.move(moveDirection);

    // 确保视图不超出游戏世界的边界
    float min_x = 0;
    float max_x = world_.get_world_width() * TILE_SIZE - window_width_;
    float min_y = 0;
    float max_y = world_.get_world_height() * TILE_SIZE - window_height_;

    sf::FloatRect viewBounds = view.getViewport();
    view.setCenter(
        std::max(min_x + window_width_ / 2.0f, 
        std::min(view.getCenter().x, max_x + window_width_ / 2.0f)),
        std::max(min_y + window_height_ / 2.0f, 
        std::min(view.getCenter().y, max_y + window_height_ / 2.0f))
    );

    window.setView(view);
}

