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
        SIM_DEBUG("Profil de bruit chargé : {} paliers, résolution {:.2f} mètres", 
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
        data.at("model").get<std::string>(), 
        data.at("min_range").get<double>(), 
        data.at("max_range").get<double>(), 
        data.at("accuracy").get<double>(), 
        rotation_rate
    );

    for(const auto& laser : data.at("lasers")){
        lidar_config->addLaser(to_radians(laser.at("v_angle").get<double>()), 
        to_radians(laser.at("h_offset").get<double>()), laser.at("d_offset").get<double>());
    }

    // resolution horizontale azimuth
    if(data.contains("horizontal_columns")){
        lidar_config->m_h_step = std::make_unique<ColumnCountSource>(data.at("horizontal_columns").get<unsigned int>());
    }else if(data.contains("horizontal_resolution")){
        lidar_config->m_h_step = std::make_unique<DirectResolutionSource>(data.at("horizontal_resolution").get<double>());
    }else if(data.contains("points_per_second")){
        lidar_config->m_h_step = std::make_unique<PointsPerSecondSource>(data.at("points_per_second").get<unsigned int>(), lidar_config->m_rotation_rate, lidar_config->m_lasers.size());
    }else{
        SIM_ERROR("Pas de résolution horizontale trouvée dans {}!", lidar_config->m_name);
        throw std::runtime_error("Pas de résolution horizontale (azimuth) trouvé");
    }

    SIM_DEBUG("Resolution horizontale calculée : {:.2f} degrés", lidar_config->horizontal_step());

    lidar_config->m_noise_profile = parseNoiseProfile(data);

    return lidar_config;
}

std::shared_ptr<FlashLidarConfig> LidarFactory::parseFlashLidar(const nlohmann::json &data){
    bool has_res = data.contains("resolution_h") && data.contains("resolution_v");
    bool has_fov = data.contains("fov_h_min") && data.contains("fov_v_min");
    bool has_angr = data.contains("angular_resolution_h") && data.contains("angular_resolution_v");
    
    FlashFovResolution params;

    if(has_res && has_fov){
        params.res_h = data["resolution_h"].get<int>();
        params.res_v = data["resolution_v"].get<int>();
        params.fov_h_min = data["fov_h_min"].get<double>();
        params.fov_h_max = data["fov_h_max"].get<double>();
        params.fov_v_min = data["fov_v_min"].get<double>();
        params.fov_v_max = data["fov_v_max"].get<double>();
    }

    else if(has_angr && has_fov){
        double angular_resolution_h = data["angular_resolution_h"].get<double>();
        double angular_resolution_v = data["angular_resolution_v"].get<double>();
        params.fov_h_min = data["fov_h_min"].get<double>();
        params.fov_h_max = data["fov_h_max"].get<double>();
        params.fov_v_min = data["fov_v_min"].get<double>();
        params.fov_v_max = data["fov_v_max"].get<double>();
        params.res_h = static_cast<int>(params.fov_h_max - params.fov_h_min / angular_resolution_h);
        params.res_v = static_cast<int>(params.fov_v_max - params.fov_v_min / angular_resolution_v);
    }

    else if(has_angr && has_res){
        double angular_resolution_h = data["angular_resolution_h"].get<double>();
        double angular_resolution_v = data["angular_resolution_v"].get<double>();
        params.res_h = data["resolution_h"].get<int>();
        params.res_v = data["resolution_v"].get<int>();
        double amplitude_h = params.res_h * angular_resolution_h;
        params.fov_h_min = - amplitude_h / 2.0;
        params.fov_h_max = amplitude_h / 2.0;
        double amplitude_v = params.res_v * angular_resolution_v;
        params.fov_v_min = - amplitude_v / 2.0;
        params.fov_v_max = amplitude_v / 2.0;
    }

    else {
        SIM_ERROR("Flash lidar {} : paramètres manquants", data.at("model").get<std::string>());
        return nullptr;
    }
    
    auto lidar_config = std::make_shared<FlashLidarConfig>(
        data.at("model").get<std::string>(), data.at("min_range").get<double>(), 
        data.at("max_range").get<double>(), data.at("accuracy").get<double>(),
        params.res_h, params.res_v,
        to_radians(params.fov_h_min), to_radians(params.fov_h_max),
        to_radians(params.fov_v_min), to_radians(params.fov_v_max)
    );

    lidar_config->m_noise_profile = parseNoiseProfile(data);

    return lidar_config;
}

std::shared_ptr<MirroredLidarConfig> LidarFactory::parseMirroredLidar(const nlohmann::json &data){
    auto lidar_config = std::make_shared<MirroredLidarConfig>(
        data.at("model").get<std::string>(), 
        data.at("min_range").get<double>(), data.at("max_range").get<double>(),
        data.at("accuracy").get<double>(), data.at("points_per_second").get<int>(), 
        data.at("fov_h_min").get<double>(), data.at("fov_h_max").get<double>(),
        data.at("fov_v_min").get<double>(), data.at("fov_v_max").get<double>(),
        data.at("integration_time").get<double>() 
    );

    std::string mode = data.value("scan_mode", "lissajou");

    if(mode == "lissajou"){
        const auto& lissajou_json = data["lissajou"];
        lidar_config->m_mirrored_scan_params = LissajouParams{
            to_radians(lissajou_json.at("amplitude_h").get<double>()),
            to_radians(lissajou_json.at("amplitude_v").get<double>()),
            lissajou_json.at("freq_h").get<double>(),
            lissajou_json.at("freq_v").get<double>(),
            to_radians(lissajou_json.at("phase_diff").get<double>())
        };   
    }else if(mode == "raster"){
        const auto& raster_json = data["raster"];
        lidar_config->m_mirrored_scan_params = RasterParams {
            raster_json.at("resolution_h").get<int>(),
            raster_json.at("resolution_v").get<int>()
        };
    }else{
        SIM_ERROR("Paramètre non reconnu pour le mode (il doit être soit 'raster' soit 'lissajou' dans la config du lidar Miroir : {}", lidar_config->m_name);
        throw std::runtime_error("Mode de Scan inconnu pour "+ lidar_config->m_name);
    }

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

std::unique_ptr<SessionScan> LidarFactory::parseScanSession(const nlohmann::json& data){
    if(!data.contains("scan_session")){
        // par défaut
        return std::make_unique<SessionScan>(std::make_unique<ScanThreeSixty>());
    }

    const auto& session = data["scan_session"];
    std::string strategy = session.at("strategy").get<std::string>();

    if(strategy == "timed"){
        double time = session.at("duration").get<double>();
        return std::make_unique<SessionScan>(std::make_unique<ScanTimed>(time));
    }

    if(strategy == "multi"){
        int n = session.at("n_scans").get<int>();
        return std::make_unique<SessionScan>(std::make_unique<ScanMultiple>(n));
    }

    SIM_WARNING("Strategie de scan inconnue : {}", strategy);
    return std::make_unique<SessionScan>(std::make_unique<ScanThreeSixty>());
}