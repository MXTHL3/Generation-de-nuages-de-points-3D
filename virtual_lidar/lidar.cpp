#include "lidar.hpp"
#include "units.hpp"
#include "logger.hpp"
#include <iostream>
#include <iomanip>

LidarConfig::LidarConfig(std::string name, double min_dist, double max_dist, double accuracy)
    :m_name(name), m_min_dist(min_dist), m_max_dist(max_dist), m_accuracy(accuracy){}

LidarConfig::~LidarConfig() = default;

void LidarConfig::serialize_noise_profile(nlohmann::json& data) const{
    data["noise_resolution"] = m_noise_profile.resolution;
    data["noise_profile"] = nlohmann::json::array();
    for(const auto& step : m_noise_profile.steps){
        data["noise_profile"].push_back({
            {"max_distance", step.max_distance}, 
            {"sigma", step.sigma}});
    }
}


MechanicalLidarConfig::MechanicalLidarConfig(std::string name, double min_dist, double max_dist, double accuracy, double rotation_rate)
    : LidarConfig(name, min_dist, max_dist, accuracy), m_rotation_rate(rotation_rate){}

double MechanicalLidarConfig::horizontal_step() const {
    return m_h_step->compute_h_step();
}

void MechanicalLidarConfig::serialize(nlohmann::json& data) const{
    data["model"] = m_name;
    data["min_range"] = m_min_dist;
    data["max_range"] = m_max_dist;
    data["accuracy"] = m_accuracy;

    serialize_noise_profile(data);

    if(m_h_step){
        m_h_step->serialize(data);
    }

    data["type"] = "mechanical";

    data["lasers"] = nlohmann::json::array();
    for(const auto& laser : m_lasers){
        nlohmann::json laser_json;
        laser_json["v_angle"] = to_degrees(laser.v_rad);
        laser_json["h_offset"] = to_degrees(laser.h_off);
        laser_json["d_offset"] = laser.d_off;
        data["lasers"].push_back(laser_json);
    }
}

std::string MechanicalLidarConfig::to_string() const{
    std::ostringstream os;
    os << std::fixed << std::setprecision(4);
    os << "Lidar " << m_name << " (mechanical)\n";
    os << " Range : " << m_min_dist << " - " << m_max_dist << " m\n";
    os << " Rotation rate : " << m_rotation_rate << " Hz\n";
    os << " H step : " << horizontal_step() << "°\n";
    os << " Lasers : " << m_lasers.size() << " channels/lasers" ;

    if(!m_lasers.empty()){
        os << " V range : " << to_degrees(m_lasers.front().v_rad)
            << "° à " << to_degrees(m_lasers.back().v_rad) << "°\n";
    }

    os << " Noise : " << m_noise_profile.steps.size() << " palier(s) \n"; 
    for(const auto& s : m_noise_profile.steps){
        os << "     [<" << s.max_distance << " m -> +-" << s.sigma * 100 << "cm]";
        os << "\n";
    }

    os << " Quantification : " << m_noise_profile.resolution * 1000 << "mm\n";

    return os.str();
}

void MechanicalLidarConfig::addLaser(double v_rad, double h_rad, double d_off){
    m_lasers.push_back({v_rad, h_rad, d_off});
}

FlashLidarConfig::FlashLidarConfig(std::string name, double min_dist, double max_dist, double accuracy, 
    int res_h, int res_v, double fov_h, double fov_v)
    : LidarConfig(name, min_dist, max_dist, accuracy), 
    m_resolution_h(res_h), m_resolution_v(res_v), m_fov_h(fov_h), m_fov_v(fov_v){}

void FlashLidarConfig::serialize(nlohmann::json& data) const{
    data["model"] = m_name;
    data["min_range"] = m_min_dist;
    data["max_range"] = m_max_dist;
    data["accuracy"] = m_accuracy;

    serialize_noise_profile(data);

    data["type"] = "flash";
    data["resolution_h"] = m_resolution_h;
    data["resolution_v"] = m_resolution_v;
    data["fov_h"] = to_degrees(m_fov_h);
    data["fov_v"] = to_degrees(m_fov_v);
}

std::string FlashLidarConfig::to_string() const{
    std::ostringstream os;
    os << "Lidar " << m_name << " (flash)\n";
    os << std::fixed << std::setprecision(4);
    os << " Range : " << m_min_dist << " - " << m_max_dist << " m\n";
    os << " Resolution : " << m_resolution_h << " par " << m_resolution_v << " pixels\n";
    os << " Fov : " << to_degrees(m_fov_h) << "° par "<< to_degrees(m_fov_v) << "°\n";
    os << " Total points : " << m_resolution_h * m_resolution_v << " pts/frame\n";

    os << " Noise : " << m_noise_profile.steps.size() << " palier(s)"; 
    for(const auto& s : m_noise_profile.steps){
        os << "     [<" << s.max_distance << " m -> +-" << s.sigma * 100 << "cm]";
        os << "\n";
    }

    os << " Quantification : " << m_noise_profile.resolution * 1000 << "mm\n";

    return os.str();
}

MirroredLidarConfig::MirroredLidarConfig(std::string name, double min_dist, double max_dist, double accuracy,
    int points_per_second, double fov_h, double fov_v, double integration_time)
    : LidarConfig(name, min_dist, max_dist, accuracy), m_points_per_second(points_per_second),
    m_fov_h(fov_h), m_fov_v(fov_v), m_integration_time(integration_time){}

bool MirroredLidarConfig::is_lissajou() const{
    return std::holds_alternative<LissajouParams>(m_mirrored_scan_params);
}

bool MirroredLidarConfig::is_raster() const{
    return std::holds_alternative<RasterParams>(m_mirrored_scan_params);
}

void MirroredLidarConfig::serialize(nlohmann::json& data) const{
    data["model"] = m_name;
    data["min_range"] = m_min_dist;
    data["max_range"] = m_max_dist;
    data["accuracy"] = m_accuracy;

    serialize_noise_profile(data);

    data["type"] = "mirrored";
    data["points_per_second"] = m_points_per_second;
    data["fov_h"] = m_fov_h;
    data["fov_v"] = m_fov_v;
    data["integration_time"] = m_integration_time;

    if(is_lissajou()){
        LissajouParams lissajou_params = std::get<LissajouParams>(m_mirrored_scan_params);
        data["lissajou"] = {
            {"amplitude_h", to_degrees(lissajou_params.m_amplitude_h)},
            {"amplitude_v", to_degrees(lissajou_params.m_amplitude_v)},
            {"freq_h", lissajou_params.m_freq_h},
            {"freq_v", lissajou_params.m_freq_v},
            {"phase_diff", lissajou_params.m_phase_diff}
        };
    }else if(is_raster()){
        RasterParams raster_params = std::get<RasterParams>(m_mirrored_scan_params);
        data["raster"] = {
            {"resolution_h", raster_params.resolution_h},
            {"resolution_v", raster_params.resolution_v}
        };
    }else{
        SIM_ERROR("Erreur non gérée !");
        throw;
    }
}

std::string MirroredLidarConfig::to_string() const{
    std::ostringstream os;
    os << "Lidar " << m_name << " (mirror)\n";
    os << std::fixed << std::setprecision(4);
    os << " Range : " << m_min_dist << " - " << m_max_dist << " m\n";
    os << " Fov : " << to_degrees(m_fov_h) << "° par "<< to_degrees(m_fov_v) << "°\n";
    os << " Total points per second : " << m_points_per_second << " pts/s\n";
    os << " integration : "<< m_integration_time << " s\n";
    
    os << " Noise : " << m_noise_profile.steps.size() << " palier(s)"; 
    for(const auto& s : m_noise_profile.steps){
        os << "     [<" << s.max_distance << " m -> +-" << s.sigma * 100 << "cm]";
        os << "\n";
    }

    os << " Quantification : " << m_noise_profile.resolution * 1000 << "mm\n";

    return os.str();
}