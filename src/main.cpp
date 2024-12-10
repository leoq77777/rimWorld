#include <iostream>
#include "ui/ui.hpp"
int main() {
    std::cout << "Hello, World!" << std::endl;
    World world;
    mainUI ui(world);
    ui.run();
    int wait;
    std::cin >> wait;
    return 0;
}