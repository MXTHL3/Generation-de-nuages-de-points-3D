#include "lidar.hpp"
#include <iostream>

Lidar::Lidar(std::string model, double min_r, double max_r, std::vector<double> h_step, double acc)
    : m_model(model), m_min_dist(min_r), m_max_dist(max_r), m_h_step(h_step), m_accuracy(acc) {}

void Lidar::addLaser(double v_deg, double h_deg, double d_off){
    m_lasers.push_back({v_deg * M_PI/180.0, h_deg * M_PI/180.0, d_off});
}