#ifndef ASSET_MANAGER_HPP
#define ASSET_MANAGER_HPP

#include <map>
#include <memory>
#include <string>
#include "entity.hpp"

class AssetManager {
public:
    std::shared_ptr<Object> get_mesh(const std::string& path);
    std::shared_ptr<LidarConfig> get_lidar_config(const std::string& path);

private:
    std::map<std::string, std::shared_ptr<Object>> m_mesh_cache;
    std::map<std::string, std::shared_ptr<LidarConfig>> m_lidar_config_cache;

    std::shared_ptr<Object> load_mesh_from_file(const std::string& path);
    std::shared_ptr<LidarConfig> load_lidar_from_file(const std::string& path);
};

#endif