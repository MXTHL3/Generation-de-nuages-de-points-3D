#include "noise_model.hpp"

double OusterOS2Noise::apply(double real_distance){
        // Sigma selon OS2 manuel
        double sigma = (real_distance <= 30.0) ? 0.025 : 
                       (real_distance <= 60.0) ? 0.040 : 0.080;

        std::normal_distribution<double> dist(0.0, sigma);
        double noisy_distance = real_distance + dist(m_gen);
        
        // Arrondi selon la précision du capteur
        return std::round(noisy_distance / m_resolution) * m_resolution;
}