#include "lidar_factory.h"
#include <fstream>

std::shared_ptr<Lidar> LidarFactory::createFromJsonConfig(const std::string& configPath){
    std::ifstream file(configPath);
    
    if(!file.is_open()){
        throw std::runtime_error("Erreur le fichier de configuration de Lidar : "+ configPath + " ne peut être lu.");
    }

    nlohmann::json data;
    file >> data;

    auto lidar = std::make_shared<Lidar>(
        data.value("model", data.value("model_name", std::string("unknown"))),
        data.value("min_range", data.value("min_dist", 0.5)),
        data.value("max_range", data.value("max_dist", 100.0)),
        data.value("h_step", std::vector<double>{1.0}),
        data.value("accuracy", 0.02)
    );

    for(const auto& laser : data["lasers"]) {
        lidar->addLaser(laser["v_angle"], laser["h_offset"], laser["d_offset"]);
    }

    return lidar;
}