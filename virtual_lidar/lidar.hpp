#ifndef LIDAR_HPP
#define LIDAR_HPP

#include <vector>
#include <string>
#include <nlohmann/json.hpp>
#include <cmath>

class LidarConfig {
public:
    std::string m_name;
    double m_min_dist;
    double m_max_dist;
    double m_accuracy;
    
    LidarConfig(std::string name, double min_dist, double max_dist, double accuracy);

    virtual ~LidarConfig();

    virtual void serialize(nlohmann::json& data) const = 0;
};

// Stocke les spéfs d'un Lidar
class MechanicalLidarConfig : public LidarConfig{
public:
    struct Laser
    {
        double v_rad;   // Angle vertical en radians 
        double h_off;   // Décalage angulaire horizontal en radians 
        double d_off;   // Décalage distance par rapport à l'origine
    };
    
    std::vector<double> m_h_step;   // steps liste horizontal
    std::vector<Laser> m_lasers;    // lasers du Lidar

    MechanicalLidarConfig(std::string name, double min_dist, double max_dist, double accuracy,
         std::vector<double> h_step);

    void serialize(nlohmann::json& data) const override;

    // ajoute un laser (1 rayon à lancer) au Lidar
    void addLaser(double v_rad, double h_rad, double d_off);
};

class FlashLidarConfig : public LidarConfig {
public:    
    int m_resolution_h;
    int m_resolution_v;
    double m_fov_h;
    double m_fov_v;

    FlashLidarConfig(std::string name, double min_dist, double max_dist, double accuracy, 
        int res_h, int res_v, double fov_h, double fov_v);
    
    void serialize(nlohmann::json& data) const override;
};

class MirroredLidarConfig : public LidarConfig {
public:
    double m_amplitude_h;
    double m_amplitude_v;
    double m_freq_h;
    double m_freq_v;
    double m_phase_diff;
    int m_sample_rate;

    MirroredLidarConfig(std::string name, double min_dist, double max_dist, double accuracy,
        double amp_h, double amp_v, double f_h, double f_v, double phase, int sample_rate);
    
    void serialize(nlohmann::json& data) const override;
};

#endif