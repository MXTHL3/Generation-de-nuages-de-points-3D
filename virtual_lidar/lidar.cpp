#include "lidar.hpp"
#include "units.hpp"
#include "logger.hpp"
#include <iostream>

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

MirroredLidarConfig::MirroredLidarConfig(std::string name, double min_dist, double max_dist, double accuracy, 
    double amp_h, double amp_v, double f_h, double f_v, double phase, int sample_rate)
    : LidarConfig(name, min_dist, max_dist, accuracy), m_amplitude_h(amp_h), 
    m_amplitude_v(amp_v), m_freq_h(f_h), m_freq_v(f_v), m_phase_diff(phase), m_sample_rate(sample_rate){}

void MirroredLidarConfig::serialize(nlohmann::json& data) const{
    data["model"] = m_name;
    data["min_range"] = m_min_dist;
    data["max_range"] = m_max_dist;
    data["accuracy"] = m_accuracy;

    serialize_noise_profile(data);

    data["type"] = "mirrored";

    data["amplitude_h"] = to_degrees(m_amplitude_h);
    data["amplitude_v"] = to_degrees(m_amplitude_v);
    data["freq_h"] = m_freq_h;
    data["freq_v"] = m_freq_v;
    data["phase_diff"] = m_phase_diff;
    data["sample_rate"] = m_sample_rate;
}