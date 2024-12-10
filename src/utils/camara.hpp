#ifndef CAMERA_HPP
#define CAMERA_HPP
#include <SFML/Graphics.hpp>

class Camera {
public:
    void move(float dx, float dy);
    void zoom(float factor);
    sf::View get_view();
private:
    sf::View view;
    float zoom_level = 1.0f;
};

#endif