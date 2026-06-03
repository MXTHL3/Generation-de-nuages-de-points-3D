#pragma once

#include <map>
#include <memory>
#include <string>
#include "entity.h"

class AssetManager {
public:
    std::shared_ptr<Object> get_mesh(const std::string& path);
    std::shared_ptr<Lidar> get_lidar_config(const std::string& path);

private:
    std::map<std::string, std::shared_ptr<Object>> m_mesh_cache;
    std::map<std::string, std::shared_ptr<Lidar>> m_lidar_config_cache;

    std::shared_ptr<Object> load_mesh_from_file(const std::string& path);
    std::shared_ptr<Lidar> load_lidar_from_file(const std::string& path);
};