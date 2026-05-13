#ifndef LIDAR_FACTORY_HPP
#define LIDAR_FACTORY_HPP

#include "lidar.hpp"
#include <nlohmann/json.hpp>

// alias pour les configs en json
namespace LidarModels {
    const std::string OUSTER_OS1_64 = "lidars_config/ouster_os1_64.json";
    const std::string OUSTER_OS2_128 = "lidars_config/ouster_os2_128.json";
};

// créé les lidars à prédéfini à partir de fichiers json
class LidarFactory {
public:
    static std::shared_ptr<Lidar> createFromJsonConfig(const std::string& configPath);
};

#endif