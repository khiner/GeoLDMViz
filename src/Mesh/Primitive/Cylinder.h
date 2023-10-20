#pragma once

#include "Mesh/Geometry.h"

struct Cylinder : Geometry {
    Cylinder(float radius = 0.1, float height = 1, uint slices = 32);
};
