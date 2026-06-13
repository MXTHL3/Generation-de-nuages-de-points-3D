#pragma once

#include "scene.h"
#include "entity.h"
#include <vector>

class LidarScanner{
public:
    LidarScanner() = default;
    std::vector<Point3> scan(const LidarEntity& lidar_ent, const Scene& scene) const;
};