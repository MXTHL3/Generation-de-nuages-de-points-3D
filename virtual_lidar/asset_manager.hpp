#ifndef ASSET_MANAGER_HPP
#define ASSET_MANAGER_HPP

#include <map>
#include <memory>
#include <string>
#include "entity.hpp"

/// @brief  Gestionnaire de ressources avec cache
/// Charge et met en cache les maillages 3D et les configs lidar dans une hashmap.
/// Un fichier 3d et de configuration de lidar n'est donc chargé qu'une fois.
class AssetManager {
public:
    /// @brief Retourne le mesh pour un path donné (charge si nécessaire).
    /// @param path Chemin vers le fichier mesh.
    /// @return Pointeur partagé vers l'objet chargé.
    std::shared_ptr<Object> get_mesh(const std::string& path);

    /// @brief Retourne la config lidar pour un path donné (charge si nécessaire).
    /// @param path Chemin vers le fichier json de config lidar.
    /// @return Pointeur partagé vers la configuration lidar chargée. 
    std::shared_ptr<LidarConfig> get_lidar_config(const std::string& path);

private:
    std::map<std::string, std::shared_ptr<Object>> m_mesh_cache; ///< Cache des meshes
    std::map<std::string, std::shared_ptr<LidarConfig>> m_lidar_config_cache; ///< Cache des configs

    /// @brief Charge un mesh depuis un fichier (CGAL polygon mesh I/O)
    std::shared_ptr<Object> load_mesh_from_file(const std::string& path);

    /// @brief Charge une configuration lidar (avec LidarFactory::createFromJsonConfig) 
    std::shared_ptr<LidarConfig> load_lidar_from_file(const std::string& path);
};

#endif