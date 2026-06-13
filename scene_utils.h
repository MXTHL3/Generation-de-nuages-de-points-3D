#pragma once

#include <nlohmann/json.hpp>

#include "scene.h"
#include "entity.h"
#include "asset_manager.h"

#include <CGAL/Polygon_mesh_processing/triangulate_faces.h>

class SceneLoader {
public:
    static bool load_scene_from_json(const std::string& filepath, Scene& world,
        std::vector<std::unique_ptr<LidarEntity>>& lidars, AssetManager& assets);

    static bool save_scene_to_json(const std::string& filepath, Scene& world,
        const std::vector<std::unique_ptr<LidarEntity>>& lidars);

private:
    static Pose parse_pose_from_json(const nlohmann::json& j);
    static nlohmann::json pose_to_json(const Pose& p);

    static std::unique_ptr<LidarEntity> make_lidar_entity(std::shared_ptr<LidarConfig> config, const Pose& pose);
};