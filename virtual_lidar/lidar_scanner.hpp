#ifndef LIDAR_SCANNER_HPP
#define LIDAR_SCANNER_HPP

#include "scene.hpp"
#include "entity.hpp"
#include <vector>

/// @brief Moteur de scan : intersection des rayons avec la scène et le noise
/// Reçoit les rayons générés par l'entité Lidar, 
/// trouve les intersections avec l'arbre AABB de la scène, 
/// applique le modèle de bruit si demandé et retourne le nuage de points résultat.
class LidarScanner{
public:
    LidarScanner() = default;
    /// @brief Effectue un scan.
    /// @param lidar_ent le lidar qui génère les rayons.
    /// @param scene la scène construite.
    /// @param apply_noise Active ou désactive le bruit (vrai par défaut).
    /// @param duration Durée du scan en secondes (en fonction de la stratégie).
    /// @return Nuage de points.
    std::vector<Point3> scan(const LidarEntity& lidar_ent, const Scene& scene, bool apply_noise = true, double duration = -1.0) const;
};

#endif