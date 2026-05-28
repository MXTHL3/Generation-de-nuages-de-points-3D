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

bool LidarFactory::saveToJson(const std::string& configPath, const Lidar& lidar){

    nlohmann::json data;
    data["model"] = lidar.m_name;
    data["min_range"] = lidar.m_min_dist;
    data["max_range"] = lidar.m_max_dist;
    data["h_step"] = lidar.m_h_step;
    data["accuracy"] = lidar.m_accuracy;

    data["lasers"] = nlohmann::json::array();
    for(const auto& laser : lidar.m_lasers){
        nlohmann::json laser_json;

        laser_json["v_angle"] = to_degrees(laser.v_rad);
        laser_json["h_offset"] = to_degrees(laser.h_off);
        laser_json["d_offset"] = laser.d_off;

        data["lasers"].push_back(laser_json);
    } 

    std::ofstream file(configPath);
    if(!file.is_open()) return false;
    file << data.dump(4);
    return true;
}