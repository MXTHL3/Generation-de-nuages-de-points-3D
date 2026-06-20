#include "manifest.hpp"

Manifest::Manifest(const std::string &filepath)
:m_filepath(filepath){
    m_data["scans"] = nlohmann::json::array();
}

void Manifest::add_scan(int scan_index, const std::string &output_file, 
    const std::string &lidar_name, const Pose &lidar_pose, 
    const Scene &scene, size_t n_points)
{
    nlohmann::json scan_entry;
    scan_entry["scan_index"] = scan_index;
    scan_entry["output_file"] = output_file;
    scan_entry["lidar"] = lidar_name;
    scan_entry["n_points"] = n_points;

    // Pose du lidar
    scan_entry["lidar_pose"] = {
        {
            {"position", {
                lidar_pose.pos().x(),
                lidar_pose.pos().y(),
                lidar_pose.pos().z()
            }},
            {"rotation", {
                to_degrees(lidar_pose.rx()), to_degrees(lidar_pose.ry()),  to_degrees(lidar_pose.rz())
            }}
        }
    };

    // Entités de la scène
    scan_entry["entites"] = nlohmann::json::array();
    for(const auto& ent : scene.entities()){
        nlohmann::json ent_entry;
        ent_entry["entitie_name"] = ent->name();
        ent_entry["mesh_path"] = ent->mesh_path();
        ent_entry["position"] = {
            
            {"position", {
                lidar_pose.pos().x(),
                lidar_pose.pos().y(),
                lidar_pose.pos().z()
            }},
            {"rotation", {
                to_degrees(lidar_pose.rx()), to_degrees(lidar_pose.ry()),  to_degrees(lidar_pose.rz())
            }}
        };
    }

    m_data["scans"].push_back(scan_entry);
}

void Manifest::save(){
    m_data["total_scans"] = m_data["scans"].size();

    std::ofstream file(m_filepath);
    if(!file.is_open()){
        return;
    }
    file << m_data.dump(2);
}

void Manifest::flush_if_equal(int interval){
    if(m_data["scans"].size() % interval == 0){
        save();
    }
}
