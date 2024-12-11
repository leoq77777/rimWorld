#pragma once
#include "../components/component.hpp"
#include "../entities/entity.hpp"

class ISystem {
public:
    virtual ~ISystem() = default;
    virtual void update() = 0;
};
