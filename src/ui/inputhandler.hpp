// inputhandler.hpp
class InputHandler {
public:
    InputHandler(World& world) : world_(world) {}

    void handleMouseInput(const sf::Event& event, sf::RenderWindow& window) {
        switch (event.type) {
            case sf::Event::MouseButtonPressed:
                handleMousePressed(event, window);
                break;
            case sf::Event::MouseButtonReleased:
                handleMouseReleased(event, window);
                break;
            case sf::Event::MouseMoved:
                handleMouseMoved(event, window);
                break;
        }
    }

    void handleKeyboardInput(const sf::Event& event) {
        if (event.type != sf::Event::KeyPressed) return;

        switch (event.key.code) {
            case sf::Keyboard::B:
                toggleStorageCreation();
                break;
            case sf::Keyboard::Q:
                toggleActionMenu();
                break;
            // 添加更多快捷键
        }
    }

private:
    World& world_;
    // 其他必要的成员变量
};