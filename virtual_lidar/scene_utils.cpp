#include "scene_utils.hpp"
#include "lidar_factory.hpp"
#include "units.hpp"

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

bool SceneLoader::load_scene_from_json(const std::string& filepath, Scene& world, AssetManager& assets){
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
            auto shared_mesh = assets.get_mesh(item.at("mesh_path"));
            std::string name = item.at("name");
            auto staticEnt = std::make_shared<StaticEntity>(name, shared_mesh, pose);
            world.add_entity(staticEnt);
        }
    }else{
        std::cerr << "Erreur aucun objet lu dans le fichier scène : "<< filepath << std::endl;
        return false;
    }

    if(j.contains("lidar_entities")){
        for(const auto& item : j["lidar_entities"]){
            Pose pose = parse_pose_from_json(item.at("pose"));
            unsigned int step_index = item.at("step_index");
            auto shared_lidar = assets.get_lidar_config(item.at("lidar_config"));
            auto lidarEnt = std::make_shared<LidarEntity>(shared_lidar, step_index, 360, pose);
            world.add_entity(lidarEnt);
        }
    }else{
        std::cerr << "Erreur aucun lidar lu dans le fichier scène : "<< filepath << std::endl;
        return false;
    }

    return true;
}

bool SceneLoader::save_scene_to_json(const std::string& filepath, Scene& world){
    nlohmann::json j;
    j["static_entities"] = nlohmann::json::array();
    j["lidar_entities"] = nlohmann::json::array();      

    for(const auto& ent : world.entities()){
        if(auto staticEnt = std::dynamic_pointer_cast<StaticEntity>(ent)){
            nlohmann::json item;
            item["pose"] = pose_to_json(staticEnt->pose());
            j["static_entities"].push_back(item);
        }
        else if(auto lidarEnt = std::dynamic_pointer_cast<LidarEntity>(ent)){
            nlohmann::json item;
            item["step_index"] = lidarEnt->step_index();
            item["pose"] = pose_to_json(lidarEnt->pose());
            j["lidar_entities"].push_back(item);
        }
    }

    std::ofstream file(filepath);
    if(!file.is_open()) return false;
    file << j.dump(4);
    return true;
}