#pragma once
#include <string>
#include <raylib.h>

struct GroundPickup
{
    std::string itemId;       /* "sword", "boomerang", "rupee" */
    std::string name;         /* Nom affiché lors de la collecte */
    Vector2 position;
    bool active = true;
};
