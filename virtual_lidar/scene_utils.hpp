#ifndef SCENE_UTILS_HPP
#define SCENE_UTILS_HPP

#include <nlohmann/json.hpp>

#include "scene.hpp"
#include "entity.hpp"
#include "asset_manager.hpp"

#include <CGAL/Polygon_mesh_processing/triangulate_faces.h>

class SceneLoader {
public:
    static bool load_scene_from_json(const std::string& filepath, Scene& world, std::vector<std::shared_ptr<LidarEntity>>& lidars, AssetManager& assets);
    static bool save_scene_to_json(const std::string& filepath, Scene& world, const std::vector<std::shared_ptr<LidarEntity>>& lidars_ent);
private:
    static Pose parse_pose_from_json(const nlohmann::json& j);
    static nlohmann::json pose_to_json(const Pose& p);
};
#endif