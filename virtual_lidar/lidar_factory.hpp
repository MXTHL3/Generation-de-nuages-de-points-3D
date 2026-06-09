#ifndef LIDAR_FACTORY_HPP
#define LIDAR_FACTORY_HPP

#include "lidar.hpp"
#include <nlohmann/json.hpp>

// alias pour les configs en json
namespace LidarModels {
    const std::string OUSTER_OS1_64 =           "lidars_config/ouster_os1_64.json";
    const std::string OUSTER_OS2_128_REVD =     "lidars_config/ouster_os2_128_revd.json";
    const std::string OUSTER_OS2_128_REV7 =     "lidars_config/ouster_os2_128_rev7.json";


    const std::string VELODYNE_VLP16 =          "lidars_config/velodyne_vpl16.json";
    const std::string VELDOYNE_VLP32C =         "lidars_config/velodyne_vlp32c.json";
};

// créé les lidars à prédéfini à partir de fichiers json
class LidarFactory {
public:
    static std::shared_ptr<LidarConfig> createFromJsonConfig(const std::string& configPath);
    static bool saveToJson(const std::string& configPath, const LidarConfig& lidar_config);

private:
    static NoiseProfile parseNoiseProfile(const nlohmann::json& data);

    // extraction des différents types de lidars
    static std::shared_ptr<MechanicalLidarConfig> parseMechanicalLidar(const nlohmann::json& data);
    static std::shared_ptr<FlashLidarConfig> parseFlashLidar(const nlohmann::json &data);
    static std::shared_ptr<MirroredLidarConfig> parseMirroredLidar(const nlohmann::json &data);
};

#endif