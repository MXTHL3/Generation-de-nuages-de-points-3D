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
    static std::shared_ptr<LidarConfig> createFromJsonConfig(const std::string& configPath);
    static bool saveToJson(const std::string& configPath, const LidarConfig& lidar_config);

private:
    // extraction des différents types de lidars
    static std::shared_ptr<MechanicalLidarConfig> parseMechanicalLidar(const nlohmann::json& data);
    std::shared_ptr<FlashLidarConfig> LidarFactory::parseFlashLidar(const nlohmann::json &data);
    std::shared_ptr<MirroredLidarConfig> LidarFactory::parseMirroredLidar(const nlohmann::json &data);
};

#endif