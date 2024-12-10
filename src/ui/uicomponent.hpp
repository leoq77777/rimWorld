// uicomponents.hpp
class UIComponent {
public:
    virtual void draw(sf::RenderWindow& window) = 0;
    virtual void handleInput(const sf::Event& event) = 0;
    virtual void update(float dt) = 0;
    virtual ~UIComponent() = default;
};

class SelectionBox : public UIComponent {
public:
    SelectionBox() : is_active(false) {
        rect.setOutlineColor(sf::Color::Green);
        rect.setOutlineThickness(2);
        rect.setFillColor(sf::Color(255, 255, 255, 128));
    }

    void startSelection(const sf::Vector2i& start) {
        start_pos = start;
        is_active = true;
    }

    void updateSelection(const sf::Vector2i& current) {
        current_pos = current;
        float width = std::abs(current_pos.x - start_pos.x);
        float height = std::abs(current_pos.y - start_pos.y);
        float left = std::min(start_pos.x, current_pos.x);
        float top = std::min(start_pos.y, current_pos.y);
        rect.setPosition(left, top);
        rect.setSize(sf::Vector2f(width, height));
    }

    void draw(sf::RenderWindow& window) override {
        if (is_active) {
            window.draw(rect);
        }
    }

    sf::FloatRect getSelectionBounds() const {
        return rect.getGlobalBounds();
    }

private:
    sf::RectangleShape rect;
    sf::Vector2i start_pos;
    sf::Vector2i current_pos;
    bool is_active;
};