#pragma once
#include "world/world.hpp"
#include <cmath>


class SelectionMenu {
public:
    SelectionMenu(sf::RenderWindow& window) : window_(window) {
        if (!font.loadFromFile("C:/Windows/Fonts/Arial.ttf")) {
            std::cerr << "Failed to load font!" << std::endl;
        }
        menuBackground.setFillColor(sf::Color(255, 255, 255, 230));
        menuBackground.setSize(sf::Vector2f(150, 80));
        setupButton(buildStorageBtn, "建造存储区 (S)", 0);
        setupButton(markTreesBtn, "标记树木 (T)", 1);
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

    bool isMarkTreesClicked(sf::Vector2i mousePos) {
        return isVisible_ && markTreesBtn.getGlobalBounds().contains(mousePos.x, mousePos.y);
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

    sf::RenderWindow& window_;
    sf::RectangleShape menuBackground;
    sf::Text buildStorageBtn;
    sf::Text markTreesBtn;
    sf::Font font;
    bool isVisible_ = false;
};

class BuildMenu {
public:
    BuildMenu(sf::RenderWindow& window) : window_(window) {
        if (!font.loadFromFile("C:/Windows/Fonts/Arial.ttf")) {
            std::cerr << "Failed to load font!" << std::endl;
        }
        menuBackground.setFillColor(sf::Color(255, 255, 255, 230));
        menuBackground.setSize(sf::Vector2f(150, 80));
        setupButton(buildDoorBtn, "建造门 (D)", 0);
        setupButton(buildWallBtn, "建造墙 (W)", 1);
    }

    void show(sf::Vector2i position) {
        isVisible_ = true;
        menuBackground.setPosition(position.x, position.y);
        buildDoorBtn.setPosition(position.x + 10, position.y + 10);
        buildWallBtn.setPosition(position.x + 10, position.y + 45);
    }

    void hide() { isVisible_ = false; }
    bool isVisible() const { return isVisible_; }
    
    void draw() {
        if (!isVisible_) return;
        window_.draw(menuBackground);
        window_.draw(buildDoorBtn);
        window_.draw(buildWallBtn);
    }

    bool isBuildDoorClicked(sf::Vector2i mousePos) {
        return isVisible_ && buildDoorBtn.getGlobalBounds().contains(mousePos.x, mousePos.y);
    }

    bool isBuildWallClicked(sf::Vector2i mousePos) {
        return isVisible_ && buildWallBtn.getGlobalBounds().contains(mousePos.x, mousePos.y);
    }

    bool handleClick(sf::Vector2i mousePos) {
        if (!isVisible_) return false;
        
        if (isBuildDoorClicked(mousePos) || isBuildWallClicked(mousePos)) {
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

    sf::RenderWindow& window_;
    sf::RectangleShape menuBackground;
    sf::Text buildDoorBtn;
    sf::Text buildWallBtn;
    sf::Font font;
    bool isVisible_ = false;
};

class UI {
public:
    UI(World& world) : 
        world_(world), 
        window(sf::VideoMode(800, 600), "My Game"),
        selectionMenu(window),  // 初始化选择菜单
        buildMenu(window)       // 初始化建造菜单
    {
        if (!font.loadFromFile("C:/Windows/Fonts/Arial.ttf")) {
            std::cerr << "Failed to load font!" << std::endl;
        }
    }

    void run() {
        sf::Clock clock;
        while (window.isOpen()) {
            handleEvents(clock);
            world_.update();  // 确保世界状态更新
            draw();
        }
    }

    void draw() {
        window.clear(sf::Color(128, 128, 128));  // 灰色背景
        
        // 绘制网格线
        drawGrid();
        
        // 渲染所有实体
        for(auto& entity : world_.get_all_entities()) {
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
        
        // 绘制菜单
        selectionMenu.draw();
        buildMenu.draw();
        
        window.display();
    }

private:
    void drawGrid();
    void drawEntity(Entity entity);
    float smoothstep(float x);
    float lerp(float a, float b, float t);
    void drawSelectionHighlight(float x, float y);
    bool isSelected(Entity entity) { 
        auto& render = world_.component_manager_.get_component<RenderComponent>(entity);
        return render.is_selected;
    }

    void unselect_all_entities() {
        for(auto& entity : world_.get_all_entities()) {
            auto& render = world_.component_manager_.get_component<RenderComponent>(entity);
            render.is_selected = false;
        }
    }

    void select_entities_in_area(sf::Vector2i start, sf::Vector2i end) {
        int minX = std::min(start.x, end.x) / TILE_SIZE;
        int maxX = std::max(start.x, end.x) / TILE_SIZE;
        int minY = std::min(start.y, end.y) / TILE_SIZE;
        int maxY = std::max(start.y, end.y) / TILE_SIZE;
        
        for(auto& entity : world_.get_all_entities()) {
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
            }
        }
    }

    void handleMousePress(const sf::Event& event) {
        if (event.mouseButton.button == sf::Mouse::Left) {
            sf::Vector2i mousePos = sf::Mouse::getPosition(window);
            
            // 检查是否点击了菜单
            if (selectionMenu.handleClick(mousePos)) {
                handleSelectionMenuClick(mousePos);
                return;
            }
            if (buildMenu.handleClick(mousePos)) {
                handleBuildMenuClick(mousePos);
                return;
            }
            
            // 开始框选
            mousePressedPos = mousePos;
            is_selecting = true;
            unselect_all_entities();
            
            // 隐藏所有菜单
            selectionMenu.hide();
            buildMenu.hide();
        }
        else if (event.mouseButton.button == sf::Mouse::Right) {
            // 显示建造菜单
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
            
            // 如果有选中的实体，显示选择菜单
            if (hasSelectedEntities()) {
                selectionMenu.show(mouseCurrentPos);
            }
        }
    }

    void handleSelectionMenuClick(sf::Vector2i mousePos) {
        if (selectionMenu.isBuildStorageClicked(mousePos)) {
            std::cout << "Building storage area..." << std::endl;
            // 获取选中区域的起点和终点
            Location start = {mousePressedPos.x / TILE_SIZE, mousePressedPos.y / TILE_SIZE};
            Location end = {mouseCurrentPos.x / TILE_SIZE, mouseCurrentPos.y / TILE_SIZE};
            // 创建存储区域，这会触发相关的建造任务
            world_.make_storage_area(start, end);
        }
        else if (selectionMenu.isMarkTreesClicked(mousePos)) {
            std::cout << "Marking trees..." << std::endl;
            // 标记选中的树木，这会触发伐木任务
            world_.mark_tree();
        }
        
        // 操作完成后隐藏菜单
        selectionMenu.hide();
    }

    void handleBuildMenuClick(sf::Vector2i mousePos) {
        // 获取右键点击的网格位置
        Location buildPos = {rightClickPos.x / TILE_SIZE, rightClickPos.y / TILE_SIZE};
        
        if (buildMenu.isBuildDoorClicked(mousePos)) {
            std::cout << "Building door..." << std::endl;
            // 创建门的蓝图，这会触发建造任务
            world_.set_door_blueprint(buildPos);
        }
        else if (buildMenu.isBuildWallClicked(mousePos)) {
            std::cout << "Building wall..." << std::endl;
            // 创建墙的蓝图，这会触发建造任务
            world_.set_wall_blueprint(buildPos);
        }
        
        // 操作完成后隐藏菜单
        buildMenu.hide();
    }

    void handleKeyPress(sf::Keyboard::Key key) {
        switch (key) {
            case sf::Keyboard::S:
                if (selectionMenu.isVisible()) {
                    std::cout << "Building storage area..." << std::endl;
                    // 实现建造存储区的逻辑
                }
                break;
            case sf::Keyboard::T:
                if (selectionMenu.isVisible()) {
                    std::cout << "Marking trees..." << std::endl;
                    // 实现标记树木的逻辑
                }
                break;
            case sf::Keyboard::D:
                if (buildMenu.isVisible()) {
                    std::cout << "Building door..." << std::endl;
                    // 实现建造门的逻辑
                }
                break;
            case sf::Keyboard::W:
                if (buildMenu.isVisible()) {
                    std::cout << "Building wall..." << std::endl;
                    // 实现建造墙的逻辑
                }
                break;
            case sf::Keyboard::Escape:
                // 按ESC键隐藏所有菜单
                selectionMenu.hide();
                buildMenu.hide();
                break;
        }
    }

    // ��量
    static constexpr int WINDOW_WIDTH = 800;
    static constexpr int WINDOW_HEIGHT = 600;
    
    World& world_;
    sf::RenderWindow window;
    sf::Font font;
    SelectionMenu selectionMenu;
    BuildMenu buildMenu;
    sf::Vector2i mousePressedPos;
    sf::Vector2i mouseCurrentPos;
    sf::Vector2i rightClickPos;
    bool is_selecting = false;

    bool hasSelectedEntities() {
        for(auto& entity : world_.get_all_entities()) {
            if (isSelected(entity)) {
                return true;
            }
        }
        return false;
    }
};

void UI::drawGrid() {
    sf::RectangleShape line(sf::Vector2f(WINDOW_WIDTH, 1));
    line.setFillColor(sf::Color(100, 100, 100));  // 深灰色网格线
    
    // 绘制水平线
    for(int y = 0; y <= WINDOW_HEIGHT; y += TILE_SIZE) {
        line.setPosition(0, y);
        window.draw(line);
    }
    
    // 绘制垂直线
    line.setSize(sf::Vector2f(1, WINDOW_HEIGHT));
    for(int x = 0; x <= WINDOW_WIDTH; x += TILE_SIZE) {
        line.setPosition(x, 0);
        window.draw(line);
    }
}

void UI::drawEntity(Entity entity) {
    auto& type = world_.component_manager_.get_component<RenderComponent>(entity).entityType;
    auto& pos = world_.component_manager_.get_component<LocationComponent>(entity).loc;
    
    // 计算实际渲染位置
    float render_x = pos.x;
    float render_y = pos.y;

    // 如果实体正在移动，使用插值位置
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
            text.setString("👤");
            text.setFillColor(sf::Color::Blue);
            if (world_.component_manager_.has_component<MovementComponent>(entity)) {
                auto& movement = world_.component_manager_.get_component<MovementComponent>(entity);
                float bounce = sin(movement.progress * M_PI * 2) * 2.0f;
                text.setPosition(screen_x, screen_y - bounce);
            }
            break;
        case EntityType::DOG:
            text.setString("🐕");
            text.setFillColor(sf::Color(139, 69, 19));
            if (world_.component_manager_.has_component<MovementComponent>(entity)) {
                auto& movement = world_.component_manager_.get_component<MovementComponent>(entity);
                float bounce = sin(movement.progress * M_PI) * 4.0f;
                text.setPosition(screen_x, screen_y - bounce);
            }
            break;
        case EntityType::TREE:
            text.setString("🌲");
            text.setFillColor(sf::Color::Green);
            break;
        case EntityType::WALL:
            text.setString("🧱");
            text.setFillColor(sf::Color(139, 69, 19));
            break;
        case EntityType::DOOR:
            text.setString("🚪");
            text.setFillColor(sf::Color(139, 69, 19));
            break;
        case EntityType::STORAGE:
            text.setString("📦");
            text.setFillColor(sf::Color(139, 69, 19));
            break;
        case EntityType::WOODPACK:
            text.setString("🪵");
            text.setFillColor(sf::Color(139, 69, 19));
            break;
    }

    // 绘制选中效果
    if (isSelected(entity)) {
        drawSelectionHighlight(screen_x, screen_y);
    }

    window.draw(text);
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
    highlight.setOutlineColor(sf::Color::Yellow);
    highlight.setOutlineThickness(2);
    window.draw(highlight);
}

