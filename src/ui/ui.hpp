#pragma once
#include "world/world.hpp"
#include <cmath>
#include <SFML/Graphics.hpp>
#include <thread>
#include <chrono>
#include <iomanip>
#include <unordered_map>
#include <memory>

enum class GameState { Start, In, End };
class Message {
public:
    Message(sf::Font& font, float window_width, sf::RenderWindow& window)
        : font_(font), window_width_(window_width), window_(window), messageVisible_(false) {
        message_.setFont(font_);
        message_.setCharacterSize(32);
        message_.setFillColor(sf::Color(255, 0, 0, 128)); 
        message_.setOutlineColor(sf::Color::Black);
        message_.setOutlineThickness(2);
        message_.setString("");
    }

    void draw() {
        if (messageVisible_) {
            sf::RectangleShape bg;
            bg.setSize(sf::Vector2f(message_.getGlobalBounds().width + 20, message_.getGlobalBounds().height + 10));
            bg.setFillColor(sf::Color(0, 0, 0, 150)); 
            bg.setPosition(message_.getPosition().x - 10, message_.getPosition().y - 5); 

            window_.draw(bg);
            window_.draw(message_);

            if (messageTimer_.getElapsedTime().asSeconds() > 3.0f) 
                hideMessage();
        }
    }

    void showMessage(const std::string& text) {
        message_.setString(text);
        float x = window_width_ / 2.0f - message_.getGlobalBounds().width / 2.0f;
        message_.setPosition(x, 50);
        messageVisible_ = true;
        messageTimer_.restart();
    }

    void hideMessage() {
        messageVisible_ = false;
    }

private:
    sf::Font& font_;
    sf::Text message_;
    bool messageVisible_;
    sf::Clock messageTimer_;
    float window_width_; 
    sf::RenderWindow& window_;
};

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
        button.setCharacterSize(32);
        button.setPosition(posX, posY);
        button.setFillColor(sf::Color::White);
        button.setOutlineColor(sf::Color::Black);
        button.setOutlineThickness(2);

        sf::FloatRect textBounds = button.getGlobalBounds();

        float width = textBounds.width + 30; 
        float height = textBounds.height + 10;
        box.setSize(sf::Vector2f(width, height));
        box.setPosition(button.getPosition().x - 15, button.getPosition().y - 5);
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

        setupButton(newGameButton, newGameButtonBox, "New Game(N)", window_width_ / 2 - 60, 3 * window_height_ / 4);
        setupButton(loadGameButton, loadGameButtonBox, "Load Game(L)", window_width_ / 2 - 60, 3 * window_height_ / 4 + 100);
    }

    void draw() {
        window_.clear(sf::Color::White);

        sf::Texture bg_texture;
        if (!bg_texture.loadFromFile("../resources/images/logo.jpg")) std::cerr << "Failed to load background texture" << std::endl;
        sf::Sprite bgSprite(bg_texture);
        sf::Vector2f scale(
            static_cast<float>(window_.getSize().x) / static_cast<float>(bg_texture.getSize().x),
            static_cast<float>(window_.getSize().y) / static_cast<float>(bg_texture.getSize().y)
        );
        bgSprite.setScale(scale);
        sf::Color color = bgSprite.getColor();
        color.a = 255; 
        bgSprite.setColor(color); 
        window_.draw(bgSprite);

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
    sf::Text unMarkBtn;
    sf::RectangleShape buildStorageBtnBox;
    sf::RectangleShape markTreesBtnBox;
    sf::RectangleShape unMarkBtnBox;
    sf::Font font;
    bool isVisible_ = false;
public:
    SelectionMenu(sf::RenderWindow& window) : window_(window) {
        std::cout << "initializing selection menu" << std::endl;
        if (!font.loadFromFile("C:/Windows/Fonts/Arial.ttf")) {
            std::cerr << "Failed to load font!" << std::endl;
        }
        menuBackground.setFillColor(sf::Color(50, 50, 50, 50));
        menuBackground.setSize(sf::Vector2f(190, 125));
        setupButton(buildStorageBtn, buildStorageBtnBox, "  Set Storage (1)  ", 0);
        setupButton(markTreesBtn, markTreesBtnBox, "  Mark Trees (2)   ", 1);
        setupButton(unMarkBtn, unMarkBtnBox, "  Cancel Marks (3)", 2);
    }

    void show(sf::Vector2i position) {
        isVisible_ = true;
        menuBackground.setPosition(position.x, position.y);

        buildStorageBtn.setPosition(position.x + 10, position.y + 10);
        markTreesBtn.setPosition(position.x + 10, position.y + 45);
        unMarkBtn.setPosition(position.x + 10, position.y + 80);

        buildStorageBtnBox.setPosition(position.x + 10, position.y + 10);
        markTreesBtnBox.setPosition(position.x + 10, position.y + 45);
        unMarkBtnBox.setPosition(position.x + 10, position.y + 80);
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
        window_.draw(unMarkBtnBox);
        window_.draw(buildStorageBtn);
        window_.draw(markTreesBtn);
        window_.draw(unMarkBtn);
    }

    bool isBuildStorageClicked(sf::Vector2i mousePos) {
        return isVisible_ && buildStorageBtnBox.getGlobalBounds().contains(mousePos.x, mousePos.y);
    }

    bool notBuildStorageClicked(sf::Vector2i mousePos) {
        return isVisible_ && !buildStorageBtnBox.getGlobalBounds().contains(mousePos.x, mousePos.y);
    }

    bool isMarkTreesClicked(sf::Vector2i mousePos) {
        return isVisible_ && markTreesBtnBox.getGlobalBounds().contains(mousePos.x, mousePos.y);
    }

    bool notMarkTreesClicked(sf::Vector2i mousePos) {
        return isVisible_ && !markTreesBtnBox.getGlobalBounds().contains(mousePos.x, mousePos.y);
    }

    bool isUnmarkClicked(sf::Vector2i mousePos) {
        return isVisible_ && unMarkBtnBox.getGlobalBounds().contains(mousePos.x, mousePos.y);
    }

    bool notUnmarkClicked(sf::Vector2i mousePos) {
        return isVisible_ && !unMarkBtnBox.getGlobalBounds().contains(mousePos.x, mousePos.y);
    }

    bool handleClick(sf::Vector2i mousePos) {
        if (!isVisible_) return false;
        if (isBuildStorageClicked(mousePos) || isMarkTreesClicked(mousePos) || isUnmarkClicked(mousePos)) 
            return true;
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
        rect.setSize(sf::Vector2f(168, 30));  
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
        button.setCharacterSize(32);
        button.setPosition(posX, posY);
        button.setFillColor(sf::Color::White);
        button.setOutlineColor(sf::Color::Black);
        button.setOutlineThickness(2);

        sf::FloatRect textBounds = button.getGlobalBounds();
        float width = textBounds.width + 30; 
        float height = textBounds.height + 10;

        box.setSize(sf::Vector2f(width, height));
        box.setPosition(button.getPosition().x - 15, button.getPosition().y - 3);
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

        float panelWidth = window_width_ * 0.2f;
        float panelHeight = window_height_ * 0.3f;

        float buttonPosX = (window_width_ - panelWidth) / 2 + (panelWidth - 150) / 2 - 25;
        float buttonPosY = (window_height_ - panelHeight) / 2 + panelHeight / 2 - 25;
        setupButton(saveAndExitButton, saveAndExitButtonBox, "Save and Exit(S)", buttonPosX, buttonPosY);
    }

    void draw() {
        window_.draw(menuBackground); 

        sf::Texture bg_texture;
        if (!bg_texture.loadFromFile("../resources/images/logo.jpg")) std::cerr << "Failed to load background texture" << std::endl;
        sf::Sprite bgSprite(bg_texture);
        sf::Vector2f scale(
            static_cast<float>(window_.getSize().x) / static_cast<float>(bg_texture.getSize().x),
            static_cast<float>(window_.getSize().y) / static_cast<float>(bg_texture.getSize().y)
        );
        bgSprite.setScale(scale);
        sf::Color color = bgSprite.getColor();
        color.a = 100; 
        bgSprite.setColor(color); 
        window_.draw(bgSprite);

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
        setupButton(buildDoorBtn, doorRect, "  Build Door  (4)  ", 0);
        setupButton(buildWallBtn, wallRect, "  Build Wall  (5)  ", 1);
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
        rect.setSize(sf::Vector2f(156, 30)); 

    }

    void adjustButtonPositions(sf::Vector2i position) {
        buildDoorBtn.setPosition(position.x + 10, position.y + 10);
        buildWallBtn.setPosition(position.x + 10, position.y + 45);

        doorRect.setPosition(position.x + 10, position.y + 10);
        wallRect.setPosition(position.x + 10, position.y + 45);
    }
};

class UI {
    //settings
    std::unique_ptr<World> world_;
    int window_width_;
    int window_height_;
    sf::RenderWindow window;
    int frame_rate_;
    sf::Font font;

    //some menu thing
    GameState game_state;
    StartMenu startMenu;
    EndMenu endMenu;
    SelectionMenu selectionMenu;
    BuildMenu buildMenu;
    Message msg;
    
    //helper
    sf::Vector2i mousePressedPos;
    sf::Vector2i mouseCurrentPos;
    sf::Vector2i rightClickPos;
    std::unordered_map<Location, int> wood_count;
    bool is_selecting = false;
    bool is_game_paused = false;
    
    //camara thing
    sf::View gameView;
    float viewMoveSpeed = 200.0f;
    static constexpr int MAP_WIDTH = MAP_SIZE;
    static constexpr int MAP_HEIGHT = MAP_SIZE;
    static constexpr float MIN_VIEW_X = 0.0f;
    static constexpr float MIN_VIEW_Y = 0.0f;
    static constexpr float MAX_VIEW_X = MAP_WIDTH * TILE_SIZE;
    static constexpr float MAX_VIEW_Y = MAP_HEIGHT * TILE_SIZE;
    static constexpr float ZOOM_SPEED = 0.1f;
    static constexpr float MIN_ZOOM = 0.5f;
    static constexpr float MAX_ZOOM = 2.0f;
    float currentZoom = 1.0f;

public:
     UI(int window_width, int window_height, bool is_fullscreen = false) : 
        window(is_fullscreen ? sf::VideoMode::getDesktopMode() : 
                               sf::VideoMode(window_width, window_height),
               "RinWorld",
               is_fullscreen ? (sf::Style::Fullscreen | sf::Style::Close) : 
                               (sf::Style::Default)),
        startMenu(window, 
                 is_fullscreen ? sf::VideoMode::getDesktopMode().width : window_width,
                 is_fullscreen ? sf::VideoMode::getDesktopMode().height : window_height),
        endMenu(window,
                is_fullscreen ? sf::VideoMode::getDesktopMode().width : window_width,
                is_fullscreen ? sf::VideoMode::getDesktopMode().height : window_height),
        selectionMenu(window), 
        buildMenu(window), 
        msg(font, is_fullscreen ? sf::VideoMode::getDesktopMode().width : window_width, window),      
        window_width_(is_fullscreen ? sf::VideoMode::getDesktopMode().width : window_width),
        window_height_(is_fullscreen ? sf::VideoMode::getDesktopMode().height : window_height),
        game_state(GameState::Start)
    {   
        gameView.setSize(window_width_, window_height_);
        float initial_x = window_width_ / 2.0f;
        float initial_y = window_height_ / 2.0f;
        gameView.setCenter(initial_x, initial_y);
        window.setView(gameView);
        std::cout << "initializing UI" << std::endl;
    }

    void setFrameRate(int frameRate) { 
        window.setFramerateLimit(frameRate);
        frame_rate_ = frameRate;
    }

    void run() {
        std::cout << "running UI" << std::endl;
        sf::Clock clock;
        float deltaTime = 0.0f;
        int round = 0;
        //startNewGame();
        while (window.isOpen()) {

            handleEvents(clock);

            switch (game_state) {
                case GameState::Start: {
                    draw_start();
                    break;
                }
                case GameState::In: {
                    if( !is_game_paused ) {
                        std::cout << "\n\n-----Round " << ++round << std::endl;
                        world_ -> update_world();
                        draw_in(true);
                    }
                    break;
                }
                case GameState::End: {
                    draw_end();
                    break;
                }
            }
            
            window.display();

            //std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }
    }

private:
    void set_up_button(sf::RectangleShape& btn, std::string text, sf::Vector2f pos, sf::Vector2f size);
    void draw_start();
    void draw_in(bool background);
    void draw_end();
    void draw_entity(Entity entity);
    void drawGrid();
    void drawEntity(Entity entity);
    void drawMark(Entity entity);
    void drawWoodsHeld(Entity entity);
    void drawWoodsDropped();
    void drawTask(Entity entity);
    void drawWoodtext(std::string wood_text, Location loc);
    void drawOneWood(Location loc);
    void drawProcess(Entity entity); //entity is a target
    void update_dropped_woods();
    float smoothstep(float x);
    float lerp(int a, int b, float t);
    void drawSelectionHighlight(float x, float y);

    void startNewGame() {
        std::cout << "try start new game" << std::endl;
        world_ = std::make_unique<World>();
        game_state = GameState::In;
        std::cout << "try set view to game world view" << std::endl;
        //window.setView(gameWorldView);
    }

    void loadExistingGame() {
        try {
            std::cout << "try load existing game" << std::endl;
            world_ = std::make_unique<World>();
            world_->load_world();
            game_state = GameState::In;
        } catch (const std::exception& e) {
            std::cerr << "Failed to load game: " << e.what() << std::endl;
        }
    }

    void save_and_exit() {
        world_ -> save_world();
        window.close();
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

    void select_entities_in_area(sf::Vector2i start, sf::Vector2i end, bool select) {
        int minX = std::min(start.x, end.x) / TILE_SIZE;
        int maxX = std::max(start.x, end.x) / TILE_SIZE;
        int minY = std::min(start.y, end.y) / TILE_SIZE;
        int maxY = std::max(start.y, end.y) / TILE_SIZE;
        
        for(auto& entity : world_ -> get_all_entities()) {
            if (!world_ -> component_manager_.has_component<RenderComponent>(entity))
                continue;
            auto& render = world_ -> component_manager_.get_component<RenderComponent>(entity);
            auto& loc = world_ -> component_manager_.get_component<LocationComponent>(entity).loc;
            
            if (loc.x >= minX && loc.x <= maxX && loc.y >= minY && loc.y <= maxY)
                render.is_selected = select;
        }
    }

    void handleEvents(sf::Clock& clock) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }

            switch (game_state) {
            case GameState::Start:
                handleStartMenuEvents(event);
                break;
            case GameState::In:
                handleInGameEvents(event);
                break;
            case GameState::End:
                handleEndMenuEvents(event);
                break;
            }
        }
    }

    void handleStartMenuEvents(const sf::Event& event) {
        if (game_state != GameState::Start)
            return;

        if (event.type == sf::Event::MouseButtonPressed) {
            auto mousePos = sf::Mouse::getPosition(window);
            if (startMenu.isButtonClicked(mousePos, "New Game(N)")) {
                startNewGame();
            } else if (startMenu.isButtonClicked(mousePos, "Load Game(L)")) {
                loadExistingGame();
            }
        } else if (event.type == sf::Event::KeyPressed) {
            if (event.key.code == sf::Keyboard::N)
                startNewGame();
            else if (event.key.code == sf::Keyboard::L)
                loadExistingGame();
        }
        startMenu.updateHover(sf::Mouse::getPosition(window));
    }

    void handleInGameEvents(const sf::Event& event) {
        if (game_state != GameState::In)
            return;

        if (event.type == sf::Event::MouseButtonPressed) {
            handleMousePress(event);
        }
        else if (event.type == sf::Event::MouseButtonReleased) {
            handleMouseRelease(event);
        }
        else if (event.type == sf::Event::KeyPressed) {
            handleKeyPress(event.key.code);
        }
        else if (event.type == sf::Event::MouseWheelScrolled) {
            handleZoom(event.mouseWheelScroll.delta);
        }
    }
    
    void handleEndMenuEvents(const sf::Event& event) {
        if (game_state != GameState::End)
            return;

        if (event.type == sf::Event::MouseButtonPressed) {
            auto mousePos = sf::Mouse::getPosition(window);
            if (endMenu.isButtonClicked(mousePos))
                save_and_exit();
        }
        endMenu.updateHover(sf::Mouse::getPosition(window));

        if (event.type == sf::Event::KeyPressed) {
            if (event.key.code == sf::Keyboard::Escape) {
                game_state = GameState::In;
                is_game_paused = false;
            }
            else if (event.key.code == sf::Keyboard::S)
                save_and_exit();
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
            
            if (selectionMenu.notBuildStorageClicked(mousePos)
                && selectionMenu.notMarkTreesClicked(mousePos)
                && selectionMenu.notUnmarkClicked(mousePos))
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
            select_entities_in_area(mousePressedPos, mouseCurrentPos, true);
            
            if (!selectionMenu.isVisible())
                selectionMenu.show(mouseCurrentPos);
            else
                selectionMenu.hide();
        }
    }

    void handleSelectionMenuClick(sf::Vector2i mousePos) {
        if (selectionMenu.isBuildStorageClicked(mousePos)) {
            std::cout << "Building storage area..." << std::endl;
            Location start = {mousePressedPos.x / TILE_SIZE, mousePressedPos.y / TILE_SIZE};
            Location end = {mouseCurrentPos.x / TILE_SIZE, mouseCurrentPos.y / TILE_SIZE};
            if(world_ -> make_storage_area(start, end))
                msg.showMessage("Storage area built");
            else 
                msg.showMessage("Storage construction failed");
        }
        else if (selectionMenu.isMarkTreesClicked(mousePos)) {
            std::cout << "Marking trees..." << std::endl;
            if (world_ -> mark_tree(true))
                msg.showMessage("Trees marked");
            else 
                msg.showMessage("No trees to mark");
        }
        else if (selectionMenu.isMarkTreesClicked(mousePos)) {
            std::cout << "Unmarking trees..." << std::endl;
            if (world_ -> mark_tree(false))
                msg.showMessage("Mark canceled!");
            else 
                msg.showMessage("No marks");
        }
        
        selectionMenu.hide();
    }

    void handleBuildMenuClick(sf::Vector2i mousePos) {
        Location buildPos = {rightClickPos.x / TILE_SIZE, rightClickPos.y / TILE_SIZE};
        
        if (buildMenu.isBuildDoorClicked(mousePos)) {
            std::cout << "Building door..." << std::endl;
            if (world_ -> set_door_blueprint(buildPos)) 
                msg.showMessage("Door blueprint has been set");
            else 
                msg.showMessage("Door blueprint construction failed");
        }
        else if (buildMenu.isBuildWallClicked(mousePos)) {
            std::cout << "Building wall..." << std::endl;
            if (world_ -> set_wall_blueprint(buildPos))
                msg.showMessage("Wall blueprint has been set");
            else
                msg.showMessage("Wall blueprint construction failed");
        }
        
        buildMenu.hide();
    }

    void handleKeyPress(sf::Keyboard::Key key) {
        switch (key) {
            case sf::Keyboard::R: {
                resetView();
                break;
            }
            case sf::Keyboard::Escape: {
                is_game_paused = true;
                game_state = GameState::End;
                break;
            }
            case sf::Keyboard::Space: {
                is_game_paused = !is_game_paused;
                std::cout << (is_game_paused ? "game pause!" : "game continue!") << std::endl;
                break;
            }
            case sf::Keyboard::Num1: {
                if (selectionMenu.isVisible()) {
                    std::cout << "Building storage area..." << std::endl;
                    Location start = {mousePressedPos.x / TILE_SIZE, mousePressedPos.y / TILE_SIZE};
                    Location end = {mouseCurrentPos.x / TILE_SIZE, mouseCurrentPos.y / TILE_SIZE};
                    if(world_ -> make_storage_area(start, end))
                        msg.showMessage("Storage area built");
                    else 
                        msg.showMessage("Storage construction failed");
                }
                break;
            }
            case sf::Keyboard::Num2: {
                if (selectionMenu.isVisible()) {
                    std::cout << "Marking trees..." << std::endl;
                    if (world_ -> mark_tree(true))
                        msg.showMessage("Trees marked");
                    else 
                        msg.showMessage("No trees to mark");
                }
                break;
            }
            case sf::Keyboard::Num3: {
                if (selectionMenu.isVisible()) {
                    std::cout << "Unmarking trees..." << std::endl;
                    if (world_ -> mark_tree(false))
                        msg.showMessage("Mark canceled!");
                    else 
                    msg.showMessage("No marks");
                }
                break;
            }
            case sf::Keyboard::Num4: {
                if (buildMenu.isVisible()) {
                    std::cout << "Building door..." << std::endl;
                    Location loc = {mouseCurrentPos.x / TILE_SIZE, mouseCurrentPos.y / TILE_SIZE};
                    if (world_ -> set_door_blueprint(loc)) 
                        msg.showMessage("Door blueprint has been set");
                    else 
                        msg.showMessage("Door blueprint construction failed");
                    }
                break;
            }
            case sf::Keyboard::Num5: {
                if (buildMenu.isVisible()) {
                    std::cout << "Building wall..." << std::endl;
                    Location loc = {mouseCurrentPos.x / TILE_SIZE, mouseCurrentPos.y / TILE_SIZE};
                    if (world_ -> set_wall_blueprint(loc))
                        msg.showMessage("Wall blueprint has been set");
                    else
                        msg.showMessage("Wall blueprint construction failed");
                    }
                break;
            }
            default:
                break;
        }
    }

    //Aborted, this cause some display problem
    sf::Vector2f clampViewPosition(const sf::Vector2f& position) {
        sf::Vector2f viewSize = gameView.getSize();
        float halfWidth = viewSize.x / 2;
        float halfHeight = viewSize.y / 2;

        return sf::Vector2f(
            std::clamp(position.x, MIN_VIEW_X + halfWidth, MAX_VIEW_X - halfWidth),
            std::clamp(position.y, MIN_VIEW_Y + halfHeight, MAX_VIEW_Y - halfHeight)
        );
    }

    void updateViewPosition(float deltaTime) {
        float moveAmount = viewMoveSpeed * deltaTime;
        sf::Vector2f currentCenter = gameView.getCenter();
        sf::Vector2f newCenter = currentCenter;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
            newCenter.x -= moveAmount;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
            newCenter.x += moveAmount;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
            newCenter.y -= moveAmount;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
            newCenter.y += moveAmount;
        }

        //newCenter = clampViewPosition(newCenter);
        gameView.setCenter(newCenter);
        window.setView(gameView);
    }
    
    void handleZoom(float delta) {
        float zoom = (delta > 0) ? (1.0f - ZOOM_SPEED) : (1.0f + ZOOM_SPEED);
        float newZoom = currentZoom * zoom;
        
        if (newZoom < MIN_ZOOM || newZoom > MAX_ZOOM) {
            return;
        }
        
        currentZoom = newZoom;
        
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        sf::Vector2f beforeCoords = window.mapPixelToCoords(mousePos, gameView);
        
        gameView.setSize(window_width_ * currentZoom, window_height_ * currentZoom);
        
        sf::Vector2f afterCoords = window.mapPixelToCoords(mousePos, gameView);
        gameView.move(beforeCoords - afterCoords);
        
        window.setView(gameView);
    }
    
    void resetView() {
        currentZoom = 1.0f;
        
        gameView.setSize(static_cast<float>(window_width_), 
                        static_cast<float>(window_height_));
        
        float initial_x = window_width_ / 2.0f;
        float initial_y = window_height_ / 2.0f;
        gameView.setCenter(initial_x, initial_y);
        
        window.setView(gameView);
    }

    bool hasSelectedEntities() {
        for(auto& entity : world_ -> get_all_entities()) {
            if (isSelected(entity)) {
                return true;
            }
        }
        return false;
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

//HERE entity is character or storage or blueprint
void UI::drawWoodsHeld(Entity entity) {
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
            info = "Allocated: " + std::to_string(amount) + " / " + std::to_string(target_amount);
        }
    } else {
        return;
    }

    if (amount == 0) {
        return;
    } 

    auto loc = world_ -> component_manager_.get_component<LocationComponent>(entity).loc;
    drawOneWood(loc);
    //drawWoodtext(info, loc);
}

void UI::drawWoodsDropped() {
    update_dropped_woods();
    for(auto& pair : wood_count) {
        Location loc = pair.first;
        int amount = pair.second;
        auto wood_text = "Dropped: " + std::to_string(amount);
        if (amount > 0) {
            drawOneWood(loc);
            //drawWoodtext(wood_text, loc);
        }
    }
}

void UI::drawMark(Entity entity) {
    if(!world_ -> component_manager_.has_component<TargetComponent>(entity)) {
        return;
    }

    auto& target = world_ -> component_manager_.get_component<TargetComponent>(entity);
    if (target.is_target) {
        auto& pos = world_ -> component_manager_.get_component<LocationComponent>(entity).loc;
        float screen_x = pos.x * TILE_SIZE;
        float screen_y = pos.y * TILE_SIZE;
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
    task_text.setOutlineColor(sf::Color::White);
    task_text.setOutlineThickness(1);
    window.draw(task_text);
}

void UI::drawWoodtext(std::string wood_text, Location loc) {
    sf::Text info;
    info.setFont(font);
    info.setCharacterSize(TILE_SIZE / 2);

    sf::RectangleShape bg(sf::Vector2f(info.getGlobalBounds().width + 10, info.getGlobalBounds().height + 5));
    bg.setPosition(loc.x * TILE_SIZE - TILE_SIZE / 2, loc.y * TILE_SIZE + 3 * TILE_SIZE / 4 - 5);
    bg.setFillColor(sf::Color(0, 0, 0, 150));

    info.setString(wood_text);
    info.setFillColor(sf::Color(139, 69, 19));
    info.setPosition(loc.x * TILE_SIZE - TILE_SIZE / 2, loc.y * TILE_SIZE + 3 * TILE_SIZE / 4);

    window.draw(bg);
    window.draw(info);
}

void UI::drawOneWood(Location loc) {
    sf::Texture wood_texture;
    wood_texture.loadFromFile("../resources/images/woodpack.png");
    sf::Sprite wood(wood_texture);
    wood.setPosition(loc.x * TILE_SIZE, loc.y * TILE_SIZE);
    sf::Vector2f wood_scale(
        static_cast<float>(TILE_SIZE - 8 ) / static_cast<float>(wood_texture.getSize().x),
        static_cast<float>(TILE_SIZE - 8 ) / static_cast<float>(wood_texture.getSize().y)
    );
    wood.setScale(wood_scale);
    window.draw(wood);
}

void UI::drawProcess(Entity entity) {
    if (!world_ -> component_manager_.has_component<TargetComponent>(entity))
        return;
    auto &target = world_ -> component_manager_.get_component<TargetComponent>(entity);
    if (!target.is_target || target.progress == 0 || target.progress >= 100) 
        return;
    auto loc = world_ -> component_manager_.get_component<LocationComponent>(entity).loc;
    float bar_width = 1.5 * TILE_SIZE;
    float bar_height = 0.25 * TILE_SIZE;
    sf::RectangleShape background(sf::Vector2f(bar_width, bar_height));
    background.setFillColor(sf::Color::Black);
    background.setPosition(loc.x * TILE_SIZE - TILE_SIZE / 4, loc.y * TILE_SIZE + TILE_SIZE); 
    float p = target.progress / 100.0;
    sf::RectangleShape foreground(sf::Vector2f(p * bar_width, bar_height));
    foreground.setFillColor(sf::Color::Green);
    foreground.setPosition(loc.x * TILE_SIZE - TILE_SIZE / 4, loc.y * TILE_SIZE + TILE_SIZE); 

    window.draw(background);
    window.draw(foreground);
}

void UI::update_dropped_woods() {
    wood_count.clear();
    for(auto& entity : world_ -> get_all_entities()) {
        if (!world_ -> component_manager_.has_component<RenderComponent>(entity)) 
            continue;
        auto& render = world_ -> component_manager_.get_component<RenderComponent>(entity);

        if (render.entityType == EntityType::WOODPACK 
        && world_ -> component_manager_.has_component<ResourceComponent>(entity)
        && world_ -> component_manager_.has_component<TargetComponent>(entity)) {
            if (!world_ -> component_manager_.get_component<TargetComponent>(entity).is_target
            || world_ -> component_manager_.get_component<ResourceComponent>(entity).holder != -1) {
                continue;
            }

            auto& loc = world_ -> component_manager_.get_component<LocationComponent>(entity).loc;
            if (wood_count.find(loc) == wood_count.end()) {
                wood_count[loc] = 1;
            } else {
                wood_count[loc]++;
            }
        }
    }
}

float UI::smoothstep(float x) { 
    return x * x * (3 - 2 * x);
}

float UI::lerp(int a, int b, float t) {
    return static_cast<float>(a) + (static_cast<float>(b) - static_cast<float>(a)) * t;
}

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

void UI::set_up_button(sf::RectangleShape& btn, std::string text, sf::Vector2f pos, sf::Vector2f size) {
    btn.setSize(size);
    btn.setPosition(pos);
    btn.setFillColor(sf::Color(50, 50, 50, 120));
    btn.setOutlineColor(sf::Color::White);
    btn.setOutlineThickness(2);
}

void UI::draw_start() {
    startMenu.draw();
}

void UI::draw_end() {
    endMenu.draw();
}

void UI::draw_in(bool background) {
    static sf::Clock clock;
    float deltaTime = clock.restart().asSeconds();
    updateViewPosition(deltaTime);

    window.clear(sf::Color::White);

    if (background) {
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
        bgSprite.setPosition(0, 0);
        sf::Color color = bgSprite.getColor();
        color.a = 120; 
        bgSprite.setColor(color); 
        window.draw(bgSprite);
    }

    //drawGrid();
        //drawWoodsDropped();
        //here i dividely draw woods that are dropped and held by some entities
        //and for efficiency, only draw one of them at a time
    for(auto& entity : world_ -> get_all_entities()) {
        draw_entity(entity);
    }

    drawWoodsDropped();

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

    msg.draw();
    selectionMenu.draw();
    buildMenu.draw();


}

void UI::draw_entity(Entity entity) {
    if (!world_ -> component_manager_.has_component<RenderComponent>(entity)) {
        return;        }
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
            if (!tree_texture.loadFromFile("../resources/images/tree.png")) std::cerr << "Failed to load tree texture" << std::endl;
            sprite.setTexture(tree_texture);
            auto& loc = world_ -> component_manager_.get_component<LocationComponent>(entity).loc;
            sprite.setPosition(render_x - TILE_SIZE / 8, render_y - TILE_SIZE / 2);
            sprite.setScale(0.25f, 0.25f);
            window.draw(sprite);
            drawMark(entity);
            drawProcess(entity);
            break;
        }
        case EntityType::CHARACTER: {
            sf::Texture character_texture;
            if (!character_texture.loadFromFile("../resources/images/character.png")) std::cerr << "Failed to load character texture" << std::endl;
            sprite.setTexture(character_texture);
            sprite.setPosition(render_x, render_y);
            sprite.setScale(0.5f, 0.5f);
            window.draw(sprite);
            drawTask(entity);
            drawWoodsHeld(entity);
            break;
        }
        case EntityType::DOG: {
            sf::Texture dog_texture;
            if (!dog_texture.loadFromFile("../resources/images/cow.png")) std::cerr << "Failed to load dog texture" << std::endl;
            sprite.setTexture(dog_texture);
            sprite.setPosition(render_x, render_y);
            sprite.setScale(0.5f, 0.5f);
            window.draw(sprite);
            //drawTask(entity);
            break;
        }
        case EntityType::DOOR: {
            sf::Texture door_texture;
            if (!door_texture.loadFromFile("../resources/images/door.png")) std::cerr << "Failed to load door texture" << std::endl;
            sprite.setTexture(door_texture);
            sprite.setPosition(render_x, render_y);
            sprite.setScale(0.2f, 0.18f);
            auto& construction = world_ -> component_manager_.get_component<ConstructionComponent>(entity);
            if (!construction.is_built) {
                sf::Color color = sprite.getColor();
                color.a = 128;
                sprite.setColor(color);
            }
            window.draw(sprite);
            drawProcess(entity);
            break;
        }
        case EntityType::WALL: {
            sf::Texture wall_texture;
            if (!wall_texture.loadFromFile("../resources/images/wall.png")) std::cerr << "Failed to load wall texture" << std::endl;     
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
            drawProcess(entity);
            break;
        }
        case EntityType::STORAGE: {
            sf::Texture storage_texture;
            if (!storage_texture.loadFromFile("../resources/images/storage.png")) std::cerr << "Failed to load storage texture" << std::endl;
            sprite.setTexture(storage_texture);
            sprite.setPosition(render_x - TILE_SIZE / 8, render_y);
            sprite.setScale(0.25f, 0.25f);
            sf::Color color = sprite.getColor();
            color.a = 140;
            sprite.setColor(color);
            window.draw(sprite);
            drawWoodsHeld(entity);
            drawProcess(entity);
            break;
        }
            /* aborted: draw wood pack
            case EntityType::WOODPACK: {
                
                sf::Texture wood_texture;
                if (!wood_texture.loadFromFile("../resources/images/wood.png")) {
                    std::cerr << "Failed to load wodd texture" << std::endl;
                }
                sprite.setTexture(wood_texture);
                sprite.setPosition(render_x, render_y);
                sf::Vector2f wood_scale(
                    static_cast<float>(TILE_SIZE) / static_cast<float>(wood_texture.getSize().x),
                    static_cast<float>(TILE_SIZE) / static_cast<float>(wood_texture.getSize().y)
                );
                sprite.setScale(wood_scale);
                window.draw(sprite);
                sf::CircleShape wood(TILE_SIZE/2);
                wood.setPosition(render_x, render_y);
                wood.setFillColor(sf::Color(139, 69, 19, 120));
                window.draw(wood);
                break;
            }
            */
        default:
            break;
    }
}