#include "world/world.h"
class TaskSystem {
    public:
        TaskSystem(World& world);
        void update();
    private:
        World& world;
};