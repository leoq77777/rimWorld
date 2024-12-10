class MainUI {
public:
    MainUI(World& world) 
        : world_(world),
          window(sf::VideoMode(MAP_SIZE * TILE_SIZE, MAP_SIZE * TILE_SIZE), "Game Map"),
          entity_renderer_(world),
          action_menu_(window)
    {
        window.setFramerateLimit(60);
        setupComponents();
    }

    void run() {
        sf::Clock clock;
        while (window.isOpen()) {
            float dt = clock.restart().asSeconds();
            handleEvents();
            update(dt);
            render();
        }
    }

private:
    void setupComponents() {
        ui_components.push_back(std::make_unique<SelectionBox>());
        ui_components.push_back(std::make_unique<ActionMenu>(window));
    }

    void handleEvents() {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
                return;
            }

            for (auto& component : ui_components) {
                component->handleInput(event);
            }

            handleMouseInput(event);
            handleKeyboardInput(event);
        }
    }

    void update(float dt) {
        for (auto& component : ui_components) {
            component->update(dt);
        }
        updateEntities(dt);
    }

    void render() {
        window.clear(sf::Color::White);
        
        entity_renderer_.draw(window);
        
        for (auto& component : ui_components) {
            component->draw(window);
        }
        
        window.display();
    }

private:
    World& world_;
    sf::RenderWindow window;
    EntityRenderer entity_renderer_;
    std::vector<std::unique_ptr<UIComponent>> ui_components;
    InputHandler input_handler_;
};