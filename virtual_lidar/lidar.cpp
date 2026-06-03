#include "lidar.hpp"
#include "units.hpp"
#include <iostream>

LidarConfig::LidarConfig(std::string name, double min_dist, double max_dist, double accuracy)
    :m_name(name), m_min_dist(min_dist), m_max_dist(max_dist), m_accuracy(accuracy){}

LidarConfig::~LidarConfig() = default;

MechanicalLidarConfig::MechanicalLidarConfig(std::string name, double min_dist, double max_dist, double accuracy, std::vector<double> h_step)
    : LidarConfig(name, min_dist, max_dist, accuracy), m_h_step(h_step){}

void MechanicalLidarConfig::serialize(nlohmann::json& data) const{
    data["model"] = m_name;
    data["min_range"] = m_min_dist;
    data["max_range"] = m_max_dist;
    data["accuracy"] = m_accuracy;

    data["type"] = "mechanical";

    data["h_step"] = nlohmann::json::array();
    for(double hstep : m_h_step) {
        data["h_step"].push_back(to_degrees(hstep));
    }

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