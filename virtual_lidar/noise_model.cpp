#include "noise_model.hpp"
#include "logger.hpp"

NoiseModel::NoiseModel():m_gen(42){}

NoiseModel::NoiseModel(const NoiseProfile& profile, unsigned int seed)
        :m_profile(profile), m_gen(seed){}

void NoiseModel::profile(const NoiseProfile& profile){
        m_profile = profile;
}

const NoiseProfile& NoiseModel::profile() const {
        return m_profile;
}

double NoiseModel::apply(double real_distance) {
        double sigma = find_sigma(real_distance);

        // pas de bruit si sigma nul après le point ne sera surement pas détecté de toute manière car zone aveugle
        if(sigma <= 0.0) return real_distance;

        std::normal_distribution<double> dist(0.0, sigma);
        double noisy_distance = real_distance + dist(m_gen);

        // Application de la résolution
        if(m_profile.resolution > 0.0){
                noisy_distance = std::round(noisy_distance / m_profile.resolution) 
                        * m_profile.resolution;
        }

        return noisy_distance;
}

double NoiseModel::find_sigma(double distance) const {

        if(m_profile.steps.empty()){
                SIM_WARNING("Noise Model : pas de sigmas trouvés !");
                return 0.0;
        }

        for(const auto& step :  m_profile.steps){
                if(distance <= step.max_distance){
                        return step.sigma;
                }
        }

        // si plus de palier alors c'est le dernier palier
        return m_profile.steps.back().sigma;
}

/*double OusterOS2Noise::apply(double real_distance){
        // Sigma selon OS2 manuel
        double sigma = (real_distance <= 30.0) ? 0.025 : 
                       (real_distance <= 60.0) ? 0.040 : 0.080;

        std::normal_distribution<double> dist(0.0, sigma);
        double noisy_distance = real_distance + dist(m_gen);
        
        // Arrondi selon la précision du capteur
        return std::round(noisy_distance / m_resolution) * m_resolution;
}*/