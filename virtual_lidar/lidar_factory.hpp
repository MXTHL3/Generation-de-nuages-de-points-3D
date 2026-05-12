#ifndef LIDAR_FACTORY_HPP
#define LIDAR_FACTORY_HPP

#include "lidar.hpp"
#include <nlohmann/json.hpp>

// créé les lidars à prédéfini à partir de fichiers json
class LidarFactory {
public:
    static std::shared_ptr<Lidar> createFromJsonConfig(const std::string& configPath);
};

#endif