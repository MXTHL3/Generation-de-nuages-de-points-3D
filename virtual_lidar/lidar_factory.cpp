#include "lidar_factory.hpp"
#include "units.hpp"
#include "logger.hpp"

#include <fstream>

std::shared_ptr<LidarConfig> LidarFactory::createFromJsonConfig(const std::string& configPath){
    std::ifstream file(configPath);
    
    if(!file.is_open()){
        SIM_ERROR("Erreur le fichier de configuration de Lidar : {} ne peut être lu.", configPath);
    }

    nlohmann::json data;
    file >> data;

    try {
        std::string lidar_type = data.value("type", "mechanical");

        if      (lidar_type == "mechanical")    return parseMechanicalLidar(data);
        else    SIM_ERROR("Erreur type de lidar inconnu : {} dans le fichier {}", lidar_type, configPath);
    
    }catch(const nlohmann::json::type_error& e){
        SIM_ERROR("Type de donné incorrect dans {} : {}", configPath, e.what());
        throw;
    }catch(const std::exception& e){
        SIM_ERROR("Erreur non gérée au moment du parsing dans {} : {}", configPath, e.what());
        throw;
    }
    return nullptr;
}

std::shared_ptr<MechanicalLidarConfig> LidarFactory::parseMechanicalLidar(const nlohmann::json &data)
{   
    std::vector<double> hsteps;
    for(double hstep : data["h_step"]) {
        hsteps.push_back(to_radians(hstep));
    }

    auto lidar_config = std::make_shared<MechanicalLidarConfig>(
        data.at("model").get<std::string>(), data.at("min_range").get<double>(), data.at("max_range").get<double>(), data.at("accuracy").get<double>(), hsteps
    );

    for(const auto& laser : data.at("lasers")){
        lidar_config->addLaser(to_radians(laser.at("v_angle").get<double>()), to_radians(laser.at("h_offset").get<double>()), laser.at("d_offset").get<double>());
    }

    return lidar_config;
}

bool LidarFactory::saveToJson(const std::string& configPath, const LidarConfig& lidar_config){

    nlohmann::json data;

    try{
        lidar_config.serialize(data);

    std::ofstream file(configPath);
        if(!file.is_open()){
            SIM_ERROR("Impossible d'écrire la configuration de lidar dans {}", configPath);
        }

    file << data.dump(4);
        SIM_INFO("Configuration de lidar {} sauvegardée dans : {}", lidar_config.m_name, configPath);
    return true;
    }catch(const std::exception& e){
        SIM_ERROR("Erreur lors de la save du lidar {} dans {} : {}", lidar_config.m_name, configPath, e.what());
        return false;
    }
}