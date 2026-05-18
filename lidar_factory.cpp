#include "lidar_factory.h"
#include <fstream>

std::shared_ptr<Lidar> LidarFactory::createFromJsonConfig(const std::string& configPath){
    std::ifstream file(configPath);
    
    if(!file.is_open()){
        std::runtime_error("Erreur le fichier de configuration de Lidar : "+ configPath + " ne peut être lu.");
    }

    nlohmann::json data;
    file >> data;

    auto lidar = std::make_shared<Lidar>(
        data["model"],
        data["min_range"],
        data["max_range"],
        data["h_step"],
        data["accuracy"]
    );

    for(const auto& laser : data["lasers"]) {
        lidar->addLaser(laser["v_angle"], laser["h_offset"], laser["d_offset"]);
    }

    return lidar;
}