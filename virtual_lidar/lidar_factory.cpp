#include "lidar_factory.hpp"
#include "units.hpp"

#include <fstream>

std::shared_ptr<Lidar> LidarFactory::createFromJsonConfig(const std::string& configPath){
    std::ifstream file(configPath);
    
    if(!file.is_open()){
        std::runtime_error("Erreur le fichier de configuration de Lidar : "+ configPath + " ne peut être lu.");
    }

    nlohmann::json data;
    file >> data;

    std::vector<double> hsteps;
    for(double hstep : data["h_step"]){
        hsteps.push_back(to_radians(hstep));
    }

    auto lidar = std::make_shared<Lidar>(
        data["model"],
        data["min_range"],
        data["max_range"],
        hsteps,
        data["accuracy"]
    );

    for(const auto& laser : data["lasers"]) {
        lidar->addLaser(to_radians(laser["v_angle"]), to_radians(laser["h_offset"]), laser["d_offset"]);
    }

    return lidar;
}