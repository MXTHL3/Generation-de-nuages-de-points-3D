#ifndef LIDAR_HPP
#define LIDAR_HPP

#include <vector>
#include <string>
#include <cmath>

// Stocke les spéfs d'un Lidar
class Lidar {
public:
    struct Laser
    {
        double v_rad;   // Angle vertical en radians 
        double h_off;   // Décalage angulaire horizontal en radians 
        double d_off;   // Décalage distance par rapport à l'origine
    };
    
    std::string m_model;            // nom du modele
    double m_min_dist;              // distance minimale de detection  
    double m_max_dist;              // distance maximale de detection
    std::vector<double> m_h_step;   // steps liste horizontal
    double m_accuracy;              // précision
    std::vector<Laser> m_lasers;    // lasers du Lidar

    Lidar(std::string model, double min_r, double max_r, std::vector<double> m_h_step, double accuracy);

    // ajoute un laser (1 rayon à lancer) au Lidar
    void addLaser(double v_rad, double h_rad, double d_off);
};
#endif