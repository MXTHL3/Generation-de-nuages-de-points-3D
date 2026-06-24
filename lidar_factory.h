#pragma once

#include "lidar.h"
#include "scan_strategy.h"
#include <nlohmann/json.hpp>

/// @brief Alias chemins vers les fichiers JSON de configuration des lidars prédéfinis
namespace LidarModels {
    const std::string OUSTER_OS1_64 =           "lidars_config/ouster_os1_64.json";
    const std::string OUSTER_OS2_128_REVD =     "lidars_config/ouster_os2_128_revd.json";
    const std::string OUSTER_OS2_128_REV7 =     "lidars_config/ouster_os2_128_rev7.json";


    const std::string VELODYNE_VLP16 =          "lidars_config/velodyne_vpl16.json";
    const std::string VELDOYNE_VLP32C =         "lidars_config/velodyne_vlp32c.json";
};

/// @brief Résolution dérivée pour un flash Lidar
struct FlashFovResolution {
    int res_h = 0, res_v = 0;
    double fov_h_min = 0.0, fov_h_max = 0.0, fov_v_min = 0.0, fov_v_max = 0.0;
};

/// @brief Factory de création de configurations et sessions Lidar depuis des fichiers jsons
class LidarFactory {
public:
    /// @brief Crée une configuration lidar depuis un fichier json
    /// @param configPath chemin vers le fichier json
    /// @return Configuration de lidar Mécanique/Flash/Miroir, nullptr si erreur
    static std::shared_ptr<LidarConfig> createFromJsonConfig(const std::string& configPath);

    /// @brief Crée une session de scan depuis le bloc "scan_session" du JSON de scène
    /// @param data Json de scène complet
    /// @return SessionScan avec la stratégie configurée
    static std::unique_ptr<SessionScan> parseScanSession(const nlohmann::json& data);

    /// @brief Sauvegarde une configuration lidar dans un fichier json.
    /// @param configPath le fichier json de sortie
    /// @param lidar_config la configuration de lidar à sauvegarder
    /// @return "faux" si cela a échoué "vrai" sinon
    static bool saveToJson(const std::string& configPath, const LidarConfig& lidar_config);

private:
    /// @brief Parse le profil de bruit depuis le JSON.
    static NoiseProfile parseNoiseProfile(const nlohmann::json& data);

    // extraction des différents types de lidars
    static std::shared_ptr<MechanicalLidarConfig> parseMechanicalLidar(const nlohmann::json& data);
    static std::shared_ptr<FlashLidarConfig> parseFlashLidar(const nlohmann::json &data);
    static std::shared_ptr<MirroredLidarConfig> parseMirroredLidar(const nlohmann::json &data);
};