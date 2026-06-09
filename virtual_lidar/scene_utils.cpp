#include "scene_utils.hpp"
#include "lidar_factory.hpp"
#include "units.hpp"
#include "logger.hpp"

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

bool SceneLoader::load_scene_from_json(const std::string& filepath, Scene& world, std::vector<std::unique_ptr<LidarEntity>>& lidars_ent, AssetManager& assets){
    std::ifstream file(filepath);

    if(!file.is_open()){
        SIM_ERROR("Impossible de lire le fichier de scène : {}", filepath); 
        return false;
    }

    nlohmann::json j;
    file >> j;

    // Chargement des Objets de la scène
    if(j.contains("static_entities")) {
        for(const auto& item : j["static_entities"]){
            Pose pose = parse_pose_from_json(item.at("pose"));
            auto shared_mesh = assets.get_mesh(item.at("mesh_path"));
            std::string name = item.at("name");
            auto static_ent = std::make_unique<StaticEntity>(name, shared_mesh, pose);
            world.add_static_entity(std::move(static_ent));
        }
    }else{
        SIM_ERROR("Erreur aucun objet lu dans le fichier scène : {}", filepath);
        return false;
    }

    // Chargement des Lidars
    if(j.contains("lidar_entities")){
        for(const auto& item : j["lidar_entities"]){
            Pose pose = parse_pose_from_json(item.at("pose"));
            auto shared_lidar_config = assets.get_lidar_config(item.at("lidar_config"));
            
            // TODO:: A fixer
            if(auto mechanical_lidar_config = std::dynamic_pointer_cast<MechanicalLidarConfig>(shared_lidar_config)){
                SIM_DEBUG("{}", shared_lidar_config->to_string());
                std::unique_ptr<LidarEntity> lidar_ent = std::make_unique<MechanicalLidarEntity>(mechanical_lidar_config, pose);
                lidars_ent.push_back(std::move(lidar_ent));
            }else if(auto flash_lidar_config = std::dynamic_pointer_cast<FlashLidarConfig>(shared_lidar_config)){
                SIM_DEBUG("{}", shared_lidar_config->to_string());
            }else if(auto flash_lidar_config = std::dynamic_pointer_cast<MirroredLidarConfig>(shared_lidar_config)){
                SIM_DEBUG("{}", shared_lidar_config->to_string());
            }
        }
    }else{
        SIM_ERROR("Erreur aucun lidar lu dans le fichier scène : {}", filepath);
        return false;
    }

    return true;
}

bool SceneLoader::save_scene_to_json(const std::string& filepath, Scene& world, const std::vector<std::unique_ptr<LidarEntity>>& lidars_ent){
    nlohmann::json j;
    j["static_entities"] = nlohmann::json::array();
    j["lidar_entities"] = nlohmann::json::array();      

    // sauvegarde des objets de la scènes
    for(const auto& ent : world.entities()){
        if(ent){
            nlohmann::json item;
            item["name"] = ent->name();
            item["pose"] = pose_to_json(ent->pose());
            j["static_entities"].push_back(item);
        }else{
            return false;
        }
    }

    // sauvegarde des lidars 
    for(const auto& lidar_ent : lidars_ent){
        if(lidar_ent){
            nlohmann::json item;
            item["lidar_config"] = lidar_ent->config().m_name;
            item["pose"] = pose_to_json(lidar_ent->pose());
            j["lidar_entities"].push_back(item);
        }else{
            return false;
        }
    }

    std::ofstream file(filepath);
    if(!file.is_open()) return false;
    file << j.dump(4);
    return true;
}