#include "lidar_factory.hpp"
#include "units.hpp"
#include "logger.hpp"

#include <fstream>

std::shared_ptr<LidarConfig> LidarFactory::createFromJsonConfig(const std::string& configPath){
    std::ifstream file(configPath);
    
    if(!file.is_open()){
        SIM_ERROR("Erreur le fichier de configuration de Lidar : {} ne peut être lu.", configPath);
        return nullptr;
    }

    nlohmann::json data;
    file >> data;

    try {
        std::string lidar_type = data.value("type", "mechanical");

        if      (lidar_type == "mechanical")    return parseMechanicalLidar(data);
        else if (lidar_type == "flash")         return parseFlashLidar(data);
        else if (lidar_type == "mirrored")      return parseMirroredLidar(data);
        else    SIM_ERROR("Erreur type de lidar inconnu : {} dans le fichier {}", lidar_type, configPath);
    
    }catch(const nlohmann::json::type_error& e){
        SIM_ERROR("Type de donnée incorrect dans {} : {}", configPath, e.what());
        throw;
    }catch(const std::exception& e){
        SIM_ERROR("Erreur non gérée au moment du parsing dans {} : {}", configPath, e.what());
        throw;
    }
    return nullptr;
}

NoiseProfile LidarFactory::parseNoiseProfile(const nlohmann::json& data){
    NoiseProfile profile;
    profile.resolution = data.value("noise_resolution", 0.0);

    if(data.contains("noise_profile")){
        for(const auto& step : data["noise_profile"]){
            profile.steps.push_back({
                step.at("max_distance").get<double>(),
                step.at("sigma").get<double>()});
        }
        SIM_DEBUG("Profil de bruit chargé : {} paliers, résolution {:.4f} mètres", 
            profile.steps.size(), 
            profile.resolution);
    }else{
        // Profil par défaut
        profile.steps.push_back({999.0, 0.03});
        SIM_DEBUG("Pas de profil de bruit trouvé on en génère un par défaut");
    }
    return profile;
}

std::shared_ptr<MechanicalLidarConfig> LidarFactory::parseMechanicalLidar(const nlohmann::json &data)
{   

     //vitesse de rotation
    double rotation_rate = 0.0;
    if(data.contains("rotation_rate")){
        rotation_rate = data["rotation_rate"].get<double>();
    }else if(data.contains("rpm")){
        rotation_rate = data["rpm"].get<double>() / 60.0;
    }else{
        SIM_WARNING("Pas de vitesse de roration trouvée dans {}",  data.at("model").get<std::string>());
    }

    auto lidar_config = std::make_shared<MechanicalLidarConfig>(
        data.at("model").get<std::string>(), data.at("min_range").get<double>(), data.at("max_range").get<double>(), data.at("noise_resolution").get<double>(), rotation_rate
    );

    // resolution horizontale azimuth
    if(data.contains("horizontal_columns")){
        lidar_config->m_h_step = std::make_unique<ColumnCountSource>(data.at("horizontal_columns").get<unsigned int>());
    }else if(data.contains("horizontal_resolution")){
        lidar_config->m_h_step = std::make_unique<DirectResolutionSource>(data.at("horizontal_resolution").get<double>());
    }else if(data.contains("points_per_second")){
        lidar_config->m_h_step = std::make_unique<PointsPerSecondSource>(data.at("points_per_second").get<unsigned int>(), lidar_config->m_rotation_rate, lidar_config->m_lasers.size());
    }else{
        SIM_ERROR("Pas de résolution horizontale trouvée dans {}!", lidar_config->m_name);
        throw new std::runtime_error("Pas de résolution horizontale (azimuth) trouvé");
    }

    SIM_DEBUG("Resolution horizontale calculée : {:.4f} degrés", lidar_config->horizontal_step());
    
    for(const auto& laser : data.at("lasers")){
        lidar_config->addLaser(to_radians(laser.at("v_angle").get<double>()), to_radians(laser.at("h_offset").get<double>()), laser.at("d_offset").get<double>());
    }

    lidar_config->m_noise_profile = parseNoiseProfile(data);

    return lidar_config;
}

std::shared_ptr<FlashLidarConfig> LidarFactory::parseFlashLidar(const nlohmann::json &data){
    auto lidar_config = std::make_shared<FlashLidarConfig>(
        data.at("model").get<std::string>(), data.at("min_range").get<double>(), data.at("max_range").get<double>(), data.at("noise_resolution").get<double>(),
        data.at("resolution_h").get<double>(), data.at("resolution_v").get<double>(),
        to_radians(data.at("fov_h").get<double>()), to_radians(data.at("fov_v").get<double>())
    );

    lidar_config->m_noise_profile = parseNoiseProfile(data);

    return lidar_config;
}

std::shared_ptr<MirroredLidarConfig> LidarFactory::parseMirroredLidar(const nlohmann::json &data){
    auto lidar_config = std::make_shared<MirroredLidarConfig>(
        data.at("model").get<std::string>(), data.at("min_range").get<double>(), data.at("max_range").get<double>(), data.at("noise_resolution").get<double>(),
        data.at("amplitude_h").get<double>(), data.at("amplitude_v").get<double>(),
        to_radians(data.at("freq_h").get<double>()), to_radians(data.at("freq_v").get<double>()),
        data.at("phase_diff").get<double>(), data.at("sample_rate").get<double>()
    );

    lidar_config->m_noise_profile = parseNoiseProfile(data);

    return lidar_config;
}

bool LidarFactory::saveToJson(const std::string& configPath, const LidarConfig& lidar_config){

    nlohmann::json data;

    try{
        lidar_config.serialize(data);

        std::ofstream file(configPath);
        
        if(!file.is_open()){
            SIM_ERROR("Impossible d'écrire la configuration de lidar dans {}", configPath);
            return false;
        }

        file << data.dump(4);
        SIM_INFO("Configuration de lidar {} sauvegardée dans : {}", lidar_config.m_name, configPath);
        return true;
    }catch(const std::exception& e){
        SIM_ERROR("Erreur lors de la save du lidar {} dans {} : {}", lidar_config.m_name, configPath, e.what());
        return false;
    }
}