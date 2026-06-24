#pragma once

#include <string>
#include <fstream>
#include <nlohmann/json.hpp>
#include "scene.h"
#include "entity.h"
#include "units.h"

/// @brief Génère un fichier manifest JSON décrivant chaque scan :
/// - l'index du scan
/// - les infos de la scène
/// - le path du fichier de sortie 3d
/// - le nom et la pose du capteur
/// - le nombre de points résultat du scan
class Manifest {
public:
    /// @param filepath Chemin du fichier manifest
    Manifest(const std::string& filepath);

    /// @brief Ajoute les informations d'un scan dans l'objet json m_data
    /// @param scan_index Index du scan.
    /// @param output_file Chemin du fichier du nuage de point 3d généré.
    /// @param lidar_name Nom du capteur utilisé.
    /// @param lidar_pose Pose du capteur utilisé.
    /// @param scene La scène scannée pour récupérer les infos des entités.
    /// @param n_points Nombre de points dans le nuage généré.
    void add_scan(int scan_index, const std::string& output_file,
        const std::string& lidar_name, const Pose& lidar_pose, 
        const Scene& scene, size_t n_points);
    
    /// @brief Ecrit sur le disque.
    void save();

    /// @brief Ecrit tout les N scans.
    void flush_if_equal(int interval = 1000);

private:
    std::string m_filepath;
    nlohmann::json m_data;
};