#ifndef SCENE_UTILS_HPP
#define SCENE_UTILS_HPP

#include <nlohmann/json.hpp>

#include "scene.hpp"
#include "entity.hpp"
#include "asset_manager.hpp"

#include <CGAL/Polygon_mesh_processing/triangulate_faces.h>

/// @brief Chargement et sauvegarde de scènes 3D depuis/vers des fichiers json.
class SceneLoader {
public:
    /// @brief Charge une scène complète (objets + lidars) depuis un fichier json.
    /// @param filepath Chemin vers le fichier json de scène.
    /// @param world Scene à remplir.
    /// @param lidars Vecteur de lidar à remplir.
    /// @param assets Gestionnaire de ressources.
    /// @return vrai si le chargement à réussi, faux sinon.
    static bool load_scene_from_json(const std::string& filepath, Scene& world, std::vector<std::unique_ptr<LidarEntity>>& lidars, AssetManager& assets);
    
    /// @brief Sauvegarde d'une scène dans un fichier json.
    /// @param filepath chemin du fichier json de scene.
    /// @param world scene à écrire dans le fichier json.
    /// @param lidars_ent entités lidar à écrire dans le json.
    static bool save_scene_to_json(const std::string& filepath, Scene& world, const std::vector<std::unique_ptr<LidarEntity>>& lidars_ent);
private:
    /// @brief Parse un bloc "pose" json en objet Pose.
    static Pose parse_pose_from_json(const nlohmann::json& j);
    /// @brief Retourne un objet Pose en json.
    static nlohmann::json pose_to_json(const Pose& p);
};
#endif