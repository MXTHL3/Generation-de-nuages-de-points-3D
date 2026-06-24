#include "scene_utils.h"
#include "lidar_factory.h"
#include "units.h"
#include "logger.h"

#include <fstream>
#include <filesystem>

Pose SceneLoader::parse_pose_from_json(const nlohmann::json& j){
    auto pos = j.at("position");
    auto rot = j.at("rotation");
    return Pose({pos[0], pos[1], pos[2]}, rot[0], rot[1], rot[2]);
}

nlohmann::json SceneLoader::pose_to_json(const Pose& p){
    nlohmann::json j;
    const Point3& point = p.pos();
    j["position"] = {point.x(), point.y(), point.z()};
    j["rotation"] = {to_degrees(p.rx()), to_degrees(p.ry()), to_degrees(p.rz())};
    return j;
}

std::unique_ptr<LidarEntity> SceneLoader::make_lidar_entity(std::shared_ptr<LidarConfig> config, const Pose& pose){
    if(auto mech = std::dynamic_pointer_cast<MechanicalLidarConfig>(config)){
        return std::make_unique<MechanicalLidarEntity>(mech, pose);
    }
    if(auto flash = std::dynamic_pointer_cast<FlashLidarConfig>(config)){
        return std::make_unique<FlashLidarEntity>(flash, pose);
    }
    if(auto mirrored = std::dynamic_pointer_cast<MirroredLidarConfig>(config)){
        return std::make_unique<MirroredLidarEntity>(mirrored, pose);
    }
    return nullptr;
}

bool SceneLoader::load_scene_from_json(const std::string& filepath, Scene& world,
    std::vector<std::unique_ptr<LidarEntity>>& lidars, AssetManager& assets){

    std::ifstream file(filepath);

    if(!file.is_open()){
        std::cerr << "Erreur impossible de lire de fichier de scene : "<< filepath << std::endl;
        return false;
    }

    nlohmann::json j;
    file >> j;

    if(j.contains("static_entities")) {
        for(const auto& item : j["static_entities"]){
            Pose pose = parse_pose_from_json(item.at("pose"));
            std::string mesh_path = item.at("mesh_path").get<std::string>();
            auto shared_mesh = assets.get_mesh(mesh_path);
            std::string name = item.value("name", mesh_path);

            world.add_static_entity(std::make_unique<StaticEntity>(name, shared_mesh, pose, mesh_path));
        }
    }else{
        std::cerr << "Erreur aucun objet lu dans le fichier scène : "<< filepath << std::endl;
        return false;
    }

    if(j.contains("lidar_entities")){
        for(const auto& item : j["lidar_entities"]){
            Pose pose = parse_pose_from_json(item.at("pose"));
            std::string config_path = item.at("lidar_config").get<std::string>();

            auto shared_lidar = assets.get_lidar_config(config_path);
            if(!shared_lidar){
                SIM_WARNING("Configuration lidar introuvable : {}", config_path);
                continue;
            }

            auto lidar_ent = make_lidar_entity(shared_lidar, pose);
            if(lidar_ent){
                lidars.push_back(std::move(lidar_ent));
            }else{
                SIM_WARNING("Type de lidar non reconnu pour : {}", config_path);
            }
        }
    }

    return true;
}

bool SceneLoader::save_scene_to_json(const std::string& filepath, Scene& world,
    const std::vector<std::unique_ptr<LidarEntity>>& lidars, Gl* gl) {

    nlohmann::json j;
    j["static_entities"] = nlohmann::json::array();
    j["lidar_entities"] = nlohmann::json::array();

    for(const auto& ent : world.entities()) {   
        if (ent->mesh_path().empty()) continue;
        nlohmann::json item;
        item["name"] = ent->name();
        item["mesh_path"] = ent->mesh_path();
        item["pose"] = pose_to_json(ent->pose());
        j["static_entities"].push_back(item);
    }

    for(const auto& lidar_ent : lidars){       
        std::string config_path = gl->get_lidar_config();

        if (!LidarFactory::saveToJson(config_path, lidar_ent->config())){
            SIM_WARNING("Impossible de sauvegarder la configuration du lidar {}", lidar_ent->config().m_name);
            continue;
        }

        nlohmann::json item;
        item["lidar_config"] = config_path;
        item["pose"] = pose_to_json(lidar_ent->pose());
        j["lidar_entities"].push_back(item);
    }

    std::ofstream file(filepath);
    if(!file.is_open()) return false;
    file << j.dump(4);
    return true;
}