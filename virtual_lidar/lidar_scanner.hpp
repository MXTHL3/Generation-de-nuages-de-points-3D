#ifndef LIDAR_SCANNER_HPP
#define LIDAR_SCANNER_HPP

#include "scene.hpp"
#include "entity.hpp"
#include <vector>

class LidarScanner{
public:
    LidarScanner() = default;
    std::vector<Point3> scan(const std::shared_ptr<MechanicalLidarEntity>& lidar_ent, const Scene& scene) const;
};

#endif