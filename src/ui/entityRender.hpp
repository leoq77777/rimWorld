// entityrenderer.hpp
class EntityRenderer {
public:
    EntityRenderer(const World& world) : world_(world) {
        if (!font.loadFromFile("C:/Windows/Fonts/Arial.ttf")) {
            throw std::runtime_error("Failed to load font");
        }
        
        // 预加载实体图标
        entity_icons = {
            {EntityType::TREE, "T"},
            {EntityType::DOG, "D"},
            {EntityType::CHARACTER, "P"},
            {EntityType::STORAGE, "S"}
        };

        setupText();
    }

    void draw(sf::RenderWindow& window) {
        drawGrid(window);
        drawEntities(window);
        drawSelectionMarkers(window);
    }

private:
    void setupText() {
        text.setFont(font);
        text.setCharacterSize(TILE_SIZE);
        text.setFillColor(sf::Color::Black);
    }

    void drawGrid(sf::RenderWindow& window) {
        static sf::RectangleShape tile(sf::Vector2f(TILE_SIZE, TILE_SIZE));
        tile.setFillColor(sf::Color::Transparent);
        tile.setOutlineColor(sf::Color(210, 180, 140, 32));
        tile.setOutlineThickness(1);

        for (int i = 0; i < MAP_SIZE; ++i) {
            for (int j = 0; j < MAP_SIZE; ++j) {
                tile.setPosition(i * TILE_SIZE, j * TILE_SIZE);
                window.draw(tile);
            }
        }
    }

    void drawEntities(sf::RenderWindow& window) {
        for (int i = 0; i < MAP_SIZE; ++i) {
            for (int j = 0; j < MAP_SIZE; ++j) {
                for (auto entity : world_.entity_map_[i][j]) {
                    drawEntity(window, entity, i, j);
                }
            }
        }
    }

    void drawEntity(sf::RenderWindow& window, Entity entity, int x, int y) {
        auto& render = world_.component_manager_.get_component<RenderComponent>(entity);
        
        if (entity_icons.count(render.entityType)) {
            text.setString(entity_icons[render.entityType]);
            text.setPosition(render.render_pos);
            window.draw(text);
        }

        if (render.entityType == EntityType::STORAGE) {
            drawStorageArea(window, x, y);
        }
    }

private:
    const World& world_;
    sf::Font font;
    sf::Text text;
    std::unordered_map<EntityType, std::string> entity_icons;
};