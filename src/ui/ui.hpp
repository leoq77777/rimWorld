#pragma once
#include "world/world.hpp"
#include <cmath>
#include <SFML/Graphics.hpp>
#include <thread>
#include <chrono>
#include <iomanip>
#include <unordered_map>
#include <memory>

enum class GameState { StartMenu, InGame, EndMenu };

class StartMenu {
    sf::RenderWindow& window_;
    sf::Font font;
    sf::Text newGameButton;
    sf::Text loadGameButton;
    sf::RectangleShape newGameButtonBox;
    sf::RectangleShape loadGameButtonBox;
    int window_width_;
    int window_height_;

    void setupButton(sf::Text& button, sf::RectangleShape& box, const std::string& text, float posX, float posY) {
        button.setFont(font);
        button.setString(text);
        button.setCharacterSize(24);
        button.setPosition(posX, posY);
        button.setFillColor(sf::Color::White);
        button.setOutlineColor(sf::Color::Black);
        button.setOutlineThickness(2);

        float width = 130;
        float height = 45;
        box.setSize(sf::Vector2f(width, height));
        box.setPosition(button.getPosition().x - 5, button.getPosition().y - 5);
        box.setFillColor(sf::Color(50, 50, 50, 64)); 
        box.setOutlineColor(sf::Color::Black);
        box.setOutlineThickness(2);
    }
public:
    StartMenu(sf::RenderWindow& window, int window_width, int window_height) : 
        window_(window),
        window_width_(window_width),
        window_height_(window_height) 
    {
        std::cout << "initializing start menu" << std::endl;
        if (!font.loadFromFile("C:/Windows/Fonts/Arial.ttf")) {
            std::cerr << "Failed to load font!" << std::endl;
        }

        setupButton(newGameButton, newGameButtonBox, "New Game", window_width_ / 2 - 50, window_height_ / 2 - 50);
        setupButton(loadGameButton, loadGameButtonBox, "Load Game", window_width_ / 2 - 50, window_height_ / 2 + 50);
    }

    void draw() {
        window_.clear(sf::Color::White);
        window_.draw(newGameButton);
        window_.draw(newGameButtonBox);
        window_.draw(loadGameButton);
        window_.draw(loadGameButtonBox);
    }

    bool isButtonClicked(const sf::Vector2i& mousePos, const std::string& buttonName) {
        sf::FloatRect bounds = (buttonName == "New Game") ? newGameButtonBox.getGlobalBounds() : loadGameButtonBox.getGlobalBounds();
        return bounds.contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));
    }

    void updateHover(const sf::Vector2i& mousePos) {
        bool isNewGameHovered = newGameButton.getGlobalBounds().contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));
        bool isLoadGameHovered = loadGameButton.getGlobalBounds().contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));

        newGameButtonBox.setFillColor(isNewGameHovered ? sf::Color(255, 255, 25, 200) : sf::Color(50, 50, 50, 200));
        loadGameButtonBox.setFillColor(isLoadGameHovered ? sf::Color(255, 255, 255, 200) : sf::Color(50, 50, 50, 200));
    }  
};

class SelectionMenu {
    sf::RenderWindow& window_;
    sf::RectangleShape menuBackground;
    sf::Text buildStorageBtn;
    sf::Text markTreesBtn;
    sf::RectangleShape buildStorageBtnBox;
    sf::RectangleShape markTreesBtnBox;
    sf::Font font;
    bool isVisible_ = false;
public:
    SelectionMenu(sf::RenderWindow& window) : window_(window) {
        std::cout << "initializing selection menu" << std::endl;
        if (!font.loadFromFile("C:/Windows/Fonts/Arial.ttf")) {
            std::cerr << "Failed to load font!" << std::endl;
        }
        menuBackground.setFillColor(sf::Color(50, 50, 50, 50));
        menuBackground.setSize(sf::Vector2f(175, 85));
        setupButton(buildStorageBtn, buildStorageBtnBox, "Set Storage (1)", 0);
        setupButton(markTreesBtn, markTreesBtnBox, "Mark Trees (2) ", 1);
    }

    void show(sf::Vector2i position) {
        isVisible_ = true;
        menuBackground.setPosition(position.x, position.y);
        buildStorageBtn.setPosition(position.x + 10, position.y + 10);
        markTreesBtn.setPosition(position.x + 10, position.y + 45);
        buildStorageBtnBox.setPosition(position.x + 10, position.y + 10);
        markTreesBtnBox.setPosition(position.x + 10, position.y + 45);
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
        window_.draw(buildStorageBtnBox);
        window_.draw(markTreesBtnBox);
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
    void setupButton(sf::Text& btn, sf::RectangleShape&rect, const std::string& str, int index) {
        btn.setFont(font);
        btn.setString(str);
        btn.setCharacterSize(20);
        btn.setFillColor(sf::Color::White);

        rect.setFillColor(sf::Color(50, 50, 50, 120));
        rect.setOutlineColor(sf::Color::White);
        rect.setOutlineThickness(2);
        sf::FloatRect textRect = btn.getLocalBounds();
        rect.setSize(sf::Vector2f(textRect.width + 10, textRect.height + 10));  
    }   
};

class EndMenu {
    sf::RenderWindow& window_;
    sf::Font font;
    sf::Text saveAndExitButton;
    sf::RectangleShape saveAndExitButtonBox;
    sf::RectangleShape menuBackground; 
    sf::RectangleShape menuPanel;     
    int window_width_;
    int window_height_;

    void setupButton(sf::Text& button, sf::RectangleShape& box, const std::string& text, float posX, float posY) {
        button.setFont(font);
        button.setString(text);
        button.setCharacterSize(24);
        button.setPosition(posX, posY);
        button.setFillColor(sf::Color::White);
        button.setOutlineColor(sf::Color::Black);
        button.setOutlineThickness(2);

        float width = 170;
        float height = 45;
        box.setSize(sf::Vector2f(width, height));
        box.setPosition(button.getPosition().x - 5, button.getPosition().y - 5);
        box.setFillColor(sf::Color(50, 50, 50, 64)); 
        box.setOutlineColor(sf::Color::Black);
        box.setOutlineThickness(2);
    }

public:
    EndMenu(sf::RenderWindow& window, int window_width, int window_height)
        : window_(window), window_width_(window_width), window_height_(window_height) {
        if (!font.loadFromFile("C:/Windows/Fonts/Arial.ttf")) {
            std::cerr << "Failed to load font!" << std::endl;
        }

        menuBackground.setSize(sf::Vector2f(window_width_, window_height_));
        menuBackground.setFillColor(sf::Color(0, 0, 0, 128)); 

        float panelWidth = window_width_ * 0.25f;
        float panelHeight = window_height_ * 0.5f;
        menuPanel.setSize(sf::Vector2f(panelWidth, panelHeight));
        menuPanel.setPosition((window_width_ - panelWidth) / 2, (window_height_ - panelHeight) / 2);
        menuPanel.setFillColor(sf::Color(50, 50, 50, 200)); 
        menuPanel.setOutlineColor(sf::Color::Black);
        menuPanel.setOutlineThickness(2);

        float buttonPosX = (window_width_ - panelWidth) / 2 + (panelWidth - 150) / 2;
        float buttonPosY = (window_height_ - panelHeight) / 2 + panelHeight / 2 - 25;
        setupButton(saveAndExitButton, saveAndExitButtonBox, "Save and Exit", buttonPosX, buttonPosY);
    }

    void draw() {
        window_.draw(menuBackground); 
        window_.draw(menuPanel);      
        window_.draw(saveAndExitButton);
        window_.draw(saveAndExitButtonBox);
    }

    bool isButtonClicked(const sf::Vector2i& mousePos) {
        return saveAndExitButtonBox.getGlobalBounds().contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));
    }

    void updateHover(const sf::Vector2i& mousePos) {
        bool isHovered = saveAndExitButton.getGlobalBounds().contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));

        saveAndExitButtonBox.setFillColor(isHovered ? sf::Color(255, 255, 255, 200) : sf::Color(50, 50, 50, 200));
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
        std::cout << "initializing build menu" << std::endl;
        if (!font.loadFromFile("C:/Windows/Fonts/Arial.ttf")) {
            std::cerr << "Failed to load font!" << std::endl;
        }
        menuBackground.setFillColor(sf::Color(50, 50, 50, 50));
        menuBackground.setOutlineColor(sf::Color::Red);
        menuBackground.setOutlineThickness(3);
        menuBackground.setSize(sf::Vector2f(175, 85));
        setupButton(buildDoorBtn, doorRect, "  Build Door (3)  ", 0);
        setupButton(buildWallBtn, wallRect, "  Build Wall  (4)  ", 1);
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
        window_.draw(doorRect);
        window_.draw(wallRect);
        window_.draw(buildDoorBtn);
        window_.draw(buildWallBtn);
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
        btn.setCharacterSize(20);
        btn.setFillColor(sf::Color::White);

        rect.setFillColor(sf::Color(50, 50, 50, 120));
        rect.setOutlineColor(sf::Color::White);
        rect.setOutlineThickness(2);
        sf::FloatRect textRect = btn.getLocalBounds();
        rect.setSize(sf::Vector2f(textRect.width + 10, textRect.height + 10)); 

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
    UI(int window_width, int window_height) : 
        window(sf::VideoMode(window_width, window_height), "My Game"),
        startMenu(window, window_width, window_height),
        endMenu(window, window_width, window_height),
        selectionMenu(window), 
        buildMenu(window),       
        //view(sf::FloatRect(0, 0, window_width, window_height)),
        window_width_(window_width),
        window_height_(window_height),
        game_state(GameState::InGame)
    {   
        std::cout << "initializing UI" << std::endl;
        /*
        startMenuView.reset(sf::FloatRect(0.f, 0.f, static_cast<float>(window_width), static_cast<float>(window_height)));
        startMenuView.setViewport(sf::FloatRect(0.f, 0.f, 1.f, 1.f));

        // 设置游戏世界视图
        gameWorldView.reset(sf::FloatRect(0.f, 0.f, static_cast<float>(window_width), static_cast<float>(window_height)));
        gameWorldView.setViewport(sf::FloatRect(0.f, 0.f, 1.f, 1.f));
        
        // 初始设置为开始菜单视图
        window.setView(startMenuView);
        */
        
        messageVisible = false;
        message.setFont(font);
        message.setCharacterSize(32);
        message.setFillColor(sf::Color(255, 0, 0, 128));
        message.setOutlineColor(sf::Color::Black);
        message.setOutlineThickness(2);
        message.setPosition(window_width_ / 2.0f - 100, 50); 
        message.setString("No new message");
        std::cout << "UI initialized" << std::endl;
    }

    void run() {
        std::cout << "running UI" << std::endl;
        sf::Clock clock;
        float deltaTime = 0.0f;
        int round = 0;
        startNewGame();
        while (window.isOpen()) {

            std::cout << "\n\n-----Round " << ++round << std::endl;

            world_ -> update_world();

            handleEvents(clock);
            
            draw();

            window.display();

            //std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }

    void draw() {
        window.clear(sf::Color::White);

        bool draw_background = false;
        if (draw_background) {
            sf::Texture bg_texture;
            if (!bg_texture.loadFromFile("../resources/images/background.bmp")) {
            std::cerr << "Failed to load background texture" << std::endl;
        }
            sf::Sprite bgSprite(bg_texture);
            sf::Vector2f scale(
                static_cast<float>(window.getSize().x) / static_cast<float>(bg_texture.getSize().x),
                static_cast<float>(window.getSize().y) / static_cast<float>(bg_texture.getSize().y)
            );
            bgSprite.setScale(scale);
            sf::Color color = bgSprite.getColor();
            color.a = 120; 
            bgSprite.setColor(color); 
            window.draw(bgSprite);
        }

        drawGrid();

        for(auto& entity : world_ -> get_all_entities()) {
            new_draw_entity(entity);
        }

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
            updateMessage();
            window.draw(message);
        }
    }

    void new_draw_entity(Entity entity) {
        if (!world_ -> component_manager_.has_component<RenderComponent>(entity)) {
            return;
        }
        auto& render = world_ -> component_manager_.get_component<RenderComponent>(entity);
        sf::Sprite sprite;

        auto& loc = world_ -> component_manager_.get_component<LocationComponent>(entity).loc;
        float render_x = static_cast<float>(loc.x) * TILE_SIZE;
        float render_y = static_cast<float>(loc.y) * TILE_SIZE;

        if (world_ -> component_manager_.has_component<MovementComponent>(entity)) {
            auto& movement = world_ -> component_manager_.get_component<MovementComponent>(entity);
            float t = smoothstep(movement.progress);
            std::cout << "progress: " << movement.progress << ", t: " << t << std::endl;
            float bounce = sin(movement.progress * M_PI) * 4.0f;
            render_x = lerp(movement.start_pos.x, movement.end_pos.x, t) * TILE_SIZE - bounce;
            render_y = lerp(movement.start_pos.y, movement.end_pos.y, t) * TILE_SIZE;
        } 

        //draw_location(entity);
        
        switch (render.entityType) {
            case EntityType::TREE: {
                sf::Texture tree_texture;
                if (!tree_texture.loadFromFile("../resources/images/tree.png")) {
                    std::cerr << "Failed to load tree texture" << std::endl;
                }
                sprite.setTexture(tree_texture);
                auto& loc = world_ -> component_manager_.get_component<LocationComponent>(entity).loc;
                sprite.setPosition(render_x - TILE_SIZE / 8, render_y - TILE_SIZE);
                sprite.setScale(0.25f, 0.25f);
                window.draw(sprite);
                drawMark(entity);
                break;
            }
            case EntityType::CHARACTER: {
                sf::Texture character_texture;
                if (!character_texture.loadFromFile("../resources/images/character.png")) {
                    std::cerr << "Failed to load character texture" << std::endl;
                }
                sprite.setTexture(character_texture);
                sprite.setPosition(render_x, render_y);
                sprite.setScale(0.5f, 0.5f);
                window.draw(sprite);
                //drawTask(entity);
                break;
            }
            case EntityType::DOG: {
                sf::Texture dog_texture;
                if (!dog_texture.loadFromFile("../resources/images/cow.png")) {
                    std::cerr << "Failed to load dog texture" << std::endl;
                }
                sprite.setTexture(dog_texture);
                sprite.setPosition(render_x, render_y);
                sprite.setScale(0.5f, 0.5f);
                window.draw(sprite);
                //drawTask(entity);
                break;
            }
            case EntityType::DOOR: {
                sf::Texture door_texture;
                if (!door_texture.loadFromFile("../resources/images/door.png")) {
                    std::cerr << "Failed to load door texture" << std::endl;
                }
                sprite.setTexture(door_texture);
                sprite.setPosition(render_x, render_y);
                sprite.setScale(0.2f, 0.2f);
                auto& construction = world_ -> component_manager_.get_component<ConstructionComponent>(entity);
                if (!construction.is_built) {
                    sf::Color color = sprite.getColor();
                    color.a = 128;
                    sprite.setColor(color);
                }
                window.draw(sprite);
                break;
            }
            case EntityType::WALL: {
                sf::Texture wall_texture;
                if (!wall_texture.loadFromFile("../resources/images/wall.png")) {
                    std::cerr << "Failed to load wall texture" << std::endl;
                }
                sprite.setTexture(wall_texture);
                sprite.setPosition(render_x, render_y);
                sprite.setScale(0.6f, 0.6f);
                auto& construction = world_ -> component_manager_.get_component<ConstructionComponent>(entity);
                if (!construction.is_built) {
                    sf::Color color = sprite.getColor();
                    color.a = 128;
                    sprite.setColor(color);
                }
                window.draw(sprite);
                break;
            }
            case EntityType::STORAGE: {
                sf::Texture storage_texture;
                if (!storage_texture.loadFromFile("../resources/images/storage.png")) {
                    std::cerr << "Failed to load storage texture" << std::endl;
                }
                sprite.setTexture(storage_texture);
                sprite.setPosition(render_x - TILE_SIZE / 8, render_y);
                sprite.setScale(0.25f, 0.25f);
                sf::Color color = sprite.getColor();
                color.a = 140;
                sprite.setColor(color);
                window.draw(sprite);
                break;
            }
            case EntityType::WOODPACK: {
                sf::Texture wood_texture;
                if (!wood_texture.loadFromFile("../resources/images/wood.png")) {
                    std::cerr << "Failed to load wodd texture" << std::endl;
                }
                sprite.setTexture(wood_texture);
                sprite.setPosition(render_x, render_y);
                sprite.setScale(0.5f, 0.5f);
                window.draw(sprite);
                break;
            }
            default:
                break;
        }
    }

private:
    void drawGame();
    void drawEndMenu();

    void drawGrid();
    void drawEntity(Entity entity);
    void drawMark(Entity entity);
    void drawStorage(Location loc);
    void drawWood(Entity entity);
    void drawTask(Entity entity);
    void drawWoodtext();

    void update_dropped_woods();
    float smoothstep(float x);
    float lerp(int a, int b, float t);
    void drawSelectionHighlight(float x, float y);

    void startNewGame() {
        std::cout << "try start new game" << std::endl;
        world_ = std::make_unique<World>();
        game_state = GameState::InGame;
        std::cout << "try set view to game world view" << std::endl;
        //window.setView(gameWorldView);
    }

    void loadExistingGame() {
        try {
            std::cout << "try load existing game" << std::endl;
            world_ = std::make_unique<World>();
            world_->load_world();
            game_state = GameState::InGame;
        } catch (const std::exception& e) {
            std::cerr << "Failed to load game: " << e.what() << std::endl;
        }
    }

    bool isSelected(Entity entity) { 
        auto& render = world_ -> component_manager_.get_component<RenderComponent>(entity);
        return render.is_selected;
    }

    void unselect_all_entities() {
        for(auto& entity : world_ -> get_all_entities()) {
            if (!world_ -> component_manager_.has_component<RenderComponent>(entity)) {
                continue;
            }
            auto& render = world_ -> component_manager_.get_component<RenderComponent>(entity);
            render.is_selected = false;
        }
    }

    void select_entities_in_area(sf::Vector2i start, sf::Vector2i end) {
        int minX = std::min(start.x, end.x) / TILE_SIZE;
        int maxX = std::max(start.x, end.x) / TILE_SIZE;
        int minY = std::min(start.y, end.y) / TILE_SIZE;
        int maxY = std::max(start.y, end.y) / TILE_SIZE;
        
        for(auto& entity : world_ -> get_all_entities()) {
            if (!world_ -> component_manager_.has_component<RenderComponent>(entity)) {
                continue;
            }
            auto& render = world_ -> component_manager_.get_component<RenderComponent>(entity);
            auto& loc = world_ -> component_manager_.get_component<LocationComponent>(entity).loc;
            
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

            switch (game_state) {
            case GameState::StartMenu:
                handleStartMenuEvents(event);
                break;
            case GameState::InGame:
                handleInGameEvents(event);
                break;
            case GameState::EndMenu:
                handleEndMenuEvents(event);
                break;
            }
        }
    }

    void handleStartMenuEvents(const sf::Event& event) {
        if (event.type == sf::Event::MouseButtonPressed && game_state == GameState::StartMenu) {
            auto mousePos = sf::Mouse::getPosition(window);
            if (startMenu.isButtonClicked(mousePos, "New Game")) {
                startNewGame();
            } else if (startMenu.isButtonClicked(mousePos, "Load Game")) {
                loadExistingGame();
            }
        }
        startMenu.updateHover(sf::Mouse::getPosition(window));
    }

    void handleInGameEvents(const sf::Event& event) {
        if (event.type == sf::Event::MouseButtonPressed) {
            handleMousePress(event);
        }
        else if (event.type == sf::Event::MouseButtonReleased) {
            handleMouseRelease(event);
        }
        else if (event.type == sf::Event::KeyPressed) {
            if (event.key.code == sf::Keyboard::Escape) {
                game_state = GameState::EndMenu;
            }
        }
    }
    
    void handleEndMenuEvents(const sf::Event& event) {
        if (event.type == sf::Event::MouseButtonPressed && game_state == GameState::EndMenu) {
            auto mousePos = sf::Mouse::getPosition(window);
            if (endMenu.isButtonClicked(mousePos)) {
                window.close();
            }
        }
        endMenu.updateHover(sf::Mouse::getPosition(window));
        if (event.type == sf::Event::KeyPressed) {
            if (event.key.code == sf::Keyboard::Escape) {
                game_state = GameState::InGame;
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
            
            if(!is_selecting) {
                unselect_all_entities();
            }

            mousePressedPos = mousePos;
            is_selecting = true;
            
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
            if(world_ -> make_storage_area(start, end)) {
                showMessage("Storage area built");
            }
            else {
                showMessage("Storage construction failed");
            }
        }
        else if (selectionMenu.isMarkTreesClicked(mousePos)) {
            std::cout << "Marking trees..." << std::endl;
            if (world_ -> mark_tree()) {
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
            if (world_ -> set_door_blueprint(buildPos)) {
                showMessage("Door blueprint has been set");
            }
            else {
                showMessage("Door blueprint construction failed");
            }
        }
        else if (buildMenu.isBuildWallClicked(mousePos)) {
            std::cout << "Building wall..." << std::endl;
            if (world_ -> set_wall_blueprint(buildPos)) {
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

    GameState game_state;
    StartMenu startMenu;
    EndMenu endMenu;
    sf::View startMenuView;  
    sf::View gameWorldView;
    std::unique_ptr<World> world_;
    int window_width_;
    int window_height_;
    sf::RenderWindow window;
    sf::Font font;
    SelectionMenu selectionMenu;
    BuildMenu buildMenu;
    sf::Vector2i mousePressedPos;
    sf::Vector2i mouseCurrentPos;
    sf::Vector2i rightClickPos;
    unordered_map<Location, float> display_offset;
    unordered_map<Location, int> wood_count;
    bool is_selecting = false;
    std::unordered_map<int, bool> keyStates; 

    /*These are for camera movement
    sf::View view;
    float viewMoveSpeed = 200.0f; 
    */

    sf::Text message;
    bool messageVisible;
    sf::Clock messageTimer;
    
    bool hasSelectedEntities() {
        for(auto& entity : world_ -> get_all_entities()) {
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
            message.setString("No message"); 
        }
    }

    void update_offset() {
        //get all woods entity, and update display_offset
        //based on how many packs on the same tile
        for(auto& entity : world_ -> get_all_entities()) {
            if (!world_ -> component_manager_.has_component<RenderComponent>(entity)) {
                continue;
            }
            auto& render = world_ -> component_manager_.get_component<RenderComponent>(entity);
            if (render.entityType == EntityType::WOODPACK) {
                auto& loc = world_ -> component_manager_.get_component<LocationComponent>(entity).loc;
                if (display_offset.find(loc) == display_offset.end()) {
                    display_offset[loc] = -2 * TILE_SIZE / 16;
                } else {
                    display_offset[loc] += TILE_SIZE / 16;
                }
            }
        }
    }
    
    void draw_location(Entity entity) {
        auto loc = world_ -> component_manager_.get_component<LocationComponent>(entity).loc;
        sf::Text pos;
        pos.setFont(font);
        pos.setFillColor(sf::Color::Red);
        pos.setCharacterSize(20);
        pos.setPosition(loc.x * TILE_SIZE, (loc.y - 1) * TILE_SIZE);
        window.draw(pos);
    }
};

void UI::drawGrid() {
    sf::RectangleShape line(sf::Vector2f(window_width_, 1));
    line.setFillColor(sf::Color(100, 100, 100, 64)); 
    
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
    //some entities should not be drawn
    if (!world_ -> component_manager_.has_component<RenderComponent>(entity)) {
        return;
    }

    auto& type = world_ -> component_manager_.get_component<RenderComponent>(entity).entityType;
    auto& pos = world_ -> component_manager_.get_component<LocationComponent>(entity).loc;
    
    float render_x = static_cast<float>(pos.x);
    float render_y = static_cast<float>(pos.y);

    if (world_ -> component_manager_.has_component<MovementComponent>(entity)) {
        auto& movement = world_ -> component_manager_.get_component<MovementComponent>(entity);
        float t = smoothstep(movement.progress);
        //std::cout << "progress: " << movement.progress << ", smoothstep: " << t << std::endl;
        float mid_x = lerp(movement.start_pos.x, movement.end_pos.x, t);
        float mid_y = lerp(movement.start_pos.y, movement.end_pos.y, t);
        //std::cout << std::fixed << std::setprecision(3) << "mid loc: (" << mid_x << ", " << mid_y << ")" << std::endl;
        render_x = lerp(movement.start_pos.x, movement.end_pos.x, t);
        render_y = lerp(movement.start_pos.y, movement.end_pos.y, t);
        //std::cout << std::fixed << std::setprecision(3) << "updated render loc: (" << render_x << ", " << render_y << ")" << std::endl;
    }
    //std::cout << "entity: " << entity << std::endl;
    //std::cout << "tile loc: (" << pos.x << ", " << pos.y << ")" << std::endl;
    //std::cout << "render loc: (" << std::fixed << std::setprecision(2) << render_x << ", " << render_y << ")" << std::endl;
    float screen_x = render_x * TILE_SIZE;
    float screen_y = render_y * TILE_SIZE;
    //std::cout << "screen loc: (" << std::fixed << std::setprecision(2) << screen_x << ", " << screen_y << ")" << std::endl;
    sf::Text text;
    text.setFont(font);
    text.setCharacterSize(999);
    text.setPosition(screen_x, screen_y);

    sf::Sprite sprite;
    sprite.setPosition(screen_x, screen_y);
    sprite.setScale(0.5f, 0.5f);

    switch(type) {
        case EntityType::CHARACTER: {
            text.setString("C");
            text.setFillColor(sf::Color::Blue);
            if (world_ -> component_manager_.has_component<MovementComponent>(entity)) {
                auto& movement = world_ -> component_manager_.get_component<MovementComponent>(entity);
                float bounce = sin(movement.progress * M_PI * 2) * 2.0f;
                text.setPosition(screen_x, screen_y - bounce);
            }
            drawTask(entity);
            drawWood(entity);
            break;
        }
        case EntityType::DOG: {
            text.setString("A");
            text.setFillColor(sf::Color(139, 69, 19));
            if (world_ -> component_manager_.has_component<MovementComponent>(entity)) {
                auto& movement = world_ -> component_manager_.get_component<MovementComponent>(entity);
                float bounce = sin(movement.progress * M_PI) * 4.0f;
                text.setPosition(screen_x, screen_y - bounce);
            }
            break;
        }
        case EntityType::TREE: {
            text.setString("T");
            text.setFillColor(sf::Color::Green);
            drawMark(entity);
            sf::Texture treeTexture;
            if (!treeTexture.loadFromFile("../resources/images/tree.png")) {
                std::cerr << "Failed to load tree texture" << std::endl;
            }
            sprite.setTexture(treeTexture);
            break;
        }
        //if it is blueprint, add something
        case EntityType::WALL: {
            text.setString("&&");
            bool is_wall_blueprint = !world_ -> component_manager_.get_component<ConstructionComponent>(entity).is_built;
            if (is_wall_blueprint) {
                drawWood(entity);
                text.setFillColor(sf::Color(120, 65, 18, 128));
            } else
                text.setFillColor(sf::Color(139, 69, 19, 255));
            break;
        }
        case EntityType::DOOR: {
            text.setString("| |");
            bool is_door_blueprint = !world_ -> component_manager_.get_component<ConstructionComponent>(entity).is_built;
            if (is_door_blueprint)  {
                drawWood(entity);
                text.setFillColor(sf::Color(120, 65, 18, 128));
            } else 
                text.setFillColor(sf::Color(139, 69, 19, 255));
            break;
        }
        case EntityType::STORAGE: {
            text.setString(" ");
            text.setFillColor(sf::Color(139, 69, 19));
            drawStorage(pos);
            drawWood(entity);
            break;
        }
        case EntityType::WOODPACK: {
            //only woodpack is target that will be drawn(dropped and not collected yet)
            if (world_ -> component_manager_.has_component<TargetComponent>(entity)) {
                auto& target = world_ -> component_manager_.get_component<TargetComponent>(entity);
                if (target.is_target) {
                    text.setString("o");
                    text.setFillColor(sf::Color(139, 69, 19));
                    /*
                    auto wood_loc = world_.component_manager_.get_component<LocationComponent>(entity).loc;
                    text.setPosition(screen_x + display_offset[wood_loc], screen_y - display_offset[wood_loc]);
                    display_offset[wood_loc] -= TILE_SIZE / 16;
                    */
                }
            }
            break;
        }
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

//HERE entity is character or storager or blueprint
void UI::drawWood(Entity entity) {
    std::string info;

    if(!world_ -> component_manager_.has_component<RenderComponent>(entity)) {
        return;
    }
    int amount = 0;
    auto& type = world_ -> component_manager_.get_component<RenderComponent>(entity).entityType;
    if (type == EntityType::STORAGE || type == EntityType::CHARACTER ) {
        if (!world_ -> component_manager_.has_component<StorageComponent>(entity)) {
            return;
        }
        auto& storage = world_ -> component_manager_.get_component<StorageComponent>(entity);
        amount = storage.current_storage;
        info = "Holding: " + std::to_string(amount);
    } else if (type == EntityType::WALL || type == EntityType::DOOR) {
        if(!world_ -> component_manager_.has_component<ConstructionComponent>(entity)) {
            return;
        }
        auto& construction = world_ -> component_manager_.get_component<ConstructionComponent>(entity);
        if (construction.is_built) {
            return;
        } else {//still a blueprint
            if (!world_ -> component_manager_.has_component<StorageComponent>(entity)) {
                return;
            }
            auto& storage = world_ -> component_manager_.get_component<StorageComponent>(entity);
            amount = storage.current_storage;
            int target_amount = storage.storage_capacity;
            info = "Allocating: " + std::to_string(amount) + " / " + std::to_string(target_amount);
        }
    } else {
        return;
    }

    if (amount == 0) {
        return;
    } 

    auto& loc = world_ -> component_manager_.get_component<LocationComponent>(entity).loc;
    //draw wood
    int pack = amount / 5;
    for(int i = 0; i < pack; i++) {
        float pile = (i - 2) * TILE_SIZE / 16;
        sf::Text wood;
        wood.setFont(font);
        wood.setCharacterSize(TILE_SIZE);
        wood.setPosition(loc.x * TILE_SIZE + pile, loc.y * TILE_SIZE - pile);
        wood.setString("o");
        wood.setFillColor(sf::Color(139, 69, 19));
        window.draw(wood);
    }

    //draw wood text
    sf::Text wood_text;
    wood_text.setFont(font);
    wood_text.setCharacterSize(TILE_SIZE / 2);
    wood_text.setPosition(loc.x * TILE_SIZE - TILE_SIZE / 2, loc.y * TILE_SIZE + 3 * TILE_SIZE / 4);
    wood_text.setString(info);
    wood_text.setFillColor(sf::Color(139, 69, 19));
    window.draw(wood_text);
}

void UI::drawMark(Entity entity) {
    if(!world_ -> component_manager_.has_component<TargetComponent>(entity)) {
        return;
    }

    auto& target = world_ -> component_manager_.get_component<TargetComponent>(entity);
    if (target.is_target) {
        auto& pos = world_ -> component_manager_.get_component<LocationComponent>(entity).loc;
        float screen_x = pos.x * TILE_SIZE + TILE_SIZE / 4;
        float screen_y = pos.y * TILE_SIZE + TILE_SIZE / 4;
        std::cout << "mark tree at " << pos.x << ", " << pos.y << std::endl;
        sf::CircleShape circle(TILE_SIZE / 2);
        circle.setFillColor(sf::Color(0, 255, 0, 32));
        circle.setOutlineColor(sf::Color(0, 255, 0, 128));
        circle.setOutlineThickness(1);
        circle.setPosition(screen_x, screen_y);
        window.draw(circle);
    }
}

void UI::drawTask(Entity entity) {
    if(!world_ -> component_manager_.has_component<TaskComponent>(entity)) {
        return;
    }

    auto& task = world_ -> component_manager_.get_component<TaskComponent>(entity).current_task;
    auto& type = task.type;
    auto& pos = world_ -> component_manager_.get_component<LocationComponent>(entity).loc;
    std::string task_str = "";
    switch(type) {
        case TaskType::CHOP_WOOD: {
            task_str = "chopping tree...";
            break;
        }
        case TaskType::CONSTRUCT: {
            task_str = "building something...";
            break;
        }
        case TaskType::ALLOCATE: {
            task_str = "allocating some woods...";
            break;
        }
        case TaskType::COLLECT: {
            task_str = "collecting some woods...";
            break;
        }
        case TaskType::STORE: {
            task_str = "storing some woods...";
            break;
        }
        case TaskType::OBTAIN: {
            task_str = "obtaining some woods...";
            break;
        }
        case TaskType::EMPTY: {
            task_str = "doing nothing";
            break;
        }
        case TaskType::IDLE: {
            task_str = "wandering around...";
            break;
        }
    }

    std::string info = "" + task_str;
    sf::Text task_text;
    task_text.setFont(font);
    task_text.setCharacterSize(32);
    task_text.setPosition(pos.x * TILE_SIZE - 2 * TILE_SIZE, pos.y * TILE_SIZE - TILE_SIZE / 2);
    task_text.setString(info);
    task_text.setFillColor(sf::Color::Red);
    window.draw(task_text);
}

void UI::drawWoodtext() {
    for(auto& pair : wood_count) {
        auto& loc = pair.first;
        auto count = pair.second * 5;
        if(count == 0)
            continue;
        sf::Text wood_text;
        wood_text.setFont(font);
        wood_text.setCharacterSize(TILE_SIZE / 2);
        wood_text.setString(std::to_string(count) + " woods");
        wood_text.setFillColor(sf::Color(139, 69, 19));
        wood_text.setPosition(loc.x * TILE_SIZE - TILE_SIZE / 2, loc.y * TILE_SIZE + 3 * TILE_SIZE / 4);
        window.draw(wood_text);
    }
}

void UI::update_dropped_woods() {
    wood_count.clear();
    for(auto& entity : world_ -> get_all_entities()) {
        if (!world_ -> component_manager_.has_component<RenderComponent>(entity)
            || !world_ -> component_manager_.has_component<ResourceComponent>(entity)) {
            continue;
        }
        auto& render = world_ -> component_manager_.get_component<RenderComponent>(entity);
        auto& resource = world_ -> component_manager_.get_component<ResourceComponent>(entity);
        if (render.entityType == EntityType::WOODPACK && resource.holder == -1) {
            auto& loc = world_ -> component_manager_.get_component<LocationComponent>(entity).loc;
            if (wood_count.find(loc) == wood_count.end()) {
                wood_count[loc] = 1;
            } else {
                wood_count[loc]++;
            }
        }
    }
}

// 辅助函数：平滑插值
float UI::smoothstep(float x) {
    // 使用三次平滑插值
    return x * x * (3 - 2 * x);
}

// 辅助函数：线性插值
float UI::lerp(int a, int b, float t) {
    return static_cast<float>(a) + (static_cast<float>(b) - static_cast<float>(a)) * t;
}

// 绘制选中效果
void UI::drawSelectionHighlight(float x, float y) {
    sf::RectangleShape highlight;
    highlight.setSize(sf::Vector2f(TILE_SIZE, TILE_SIZE));
    highlight.setPosition(x, y);
    highlight.setFillColor(sf::Color::Transparent);
    highlight.setOutlineColor(sf::Color(128, 128, 128, 128));
    highlight.setOutlineThickness(1);
    window.draw(highlight);
}

/*
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
*/
//THIS is old version draw
void UI::drawGame() {
    window.clear(sf::Color(128, 128, 128));

    world_ -> update_world();
    updateMessage();
    //updateViewPosition(deltaTime);
    
    drawGrid();

    for(auto& entity : world_ -> get_all_entities()) {
        //this old version draw entity
        drawEntity(entity);
    }
        
    update_dropped_woods();
    drawWoodtext();

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
}

