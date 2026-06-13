#pragma once

#include <random>
#include <cmath>
#include <nlohmann/json.hpp>

/// @brief  Palier du profil de bruit d'un capteur lidar
struct NoiseStep{
    double max_distance; // borne supérieure du palier (en mètres)
    double sigma; // écart type du bruit gaussien (en mètres)
};

/// @brief Profil du bruit d'un capteur lidar
struct NoiseProfile{
    std::vector<NoiseStep> steps; // paliers du profil ordonnés
    double resolution = 0.0; // step de quantification (mètres)
};


/// @brief Modèle de bruit pour capteur lidar
/// Applique un bruit gaussien dépendant de la distance et du profil de bruit choisi.
/// Le générateur aléatoire "mt19937" est déterministe : on peut générer des séquences bruités identiques avec la même seed !
class NoiseModel {
public:
    NoiseModel();
    virtual ~NoiseModel() = default;

    /// @param profile Profil de bruit (paliers et résolution de bruit).
    /// @param seed Graine du générateur aléatoire.
    NoiseModel(const NoiseProfile& profile, unsigned int seed = 42);

    /// @brief Remplace par un nouveau profil de bruit.
    /// @param profile nouveau profil de bruit.
    void profile(const NoiseProfile& profile);

    /// @brief Retourne le profil de bruit.
    /// @return profil de bruit courant.
    const NoiseProfile& profile() const;
    
    /// @brief  Applique le bruit à une mesure de distance.
    /// @param real_distance Distance réelle (mètres).
    /// @return Distance bruitée (mètres).
    double apply(double real_distance)const;

private:

    /// @brief Trouve l'écart type du bruit gaussien qui convient à la distance donnée.
    double find_sigma(double distance) const;

    NoiseProfile m_profile;
    mutable std::mt19937 m_gen; // mutable car apply modifie l'état du générateur
};