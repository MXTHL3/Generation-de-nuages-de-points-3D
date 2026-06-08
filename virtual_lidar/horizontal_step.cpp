#include "horizontal_step.hpp"

ColumnCountSource::ColumnCountSource(unsigned int columns)
    :m_columns(columns){}

double ColumnCountSource::compute_h_step() const {
    return 360.0 / static_cast<double>(m_columns);
}

void ColumnCountSource::serialize(nlohmann::json& data) const{
    data["horizontal_columns"] = m_columns;
}

DirectResolutionSource::DirectResolutionSource(double resolution_deg)
    :m_resolution(resolution_deg){}

double DirectResolutionSource::compute_h_step() const{
    return m_resolution;
}

void DirectResolutionSource::serialize(nlohmann::json& data) const{
    data["horizontal_resolution"] = m_resolution;
}

PointsPerSecondSource::PointsPerSecondSource(unsigned int points_per_second, double rotation_rotate, size_t n_lasers)
    :m_points_per_second(points_per_second), m_rotation_rotate(rotation_rotate), m_n_lasers(n_lasers){}

double PointsPerSecondSource::compute_h_step() const{
    return 360.0 * m_rotation_rotate 
        * static_cast<double>(m_n_lasers) 
        / m_points_per_second;
}

void PointsPerSecondSource::serialize(nlohmann::json& data) const{
    data["points_per_second"] = m_points_per_second;
}