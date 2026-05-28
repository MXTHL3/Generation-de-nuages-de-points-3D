#include "point_cloud_exporter.hpp"
#include "logger.hpp"

#include <fstream>

void PlyExporter::save(const std::string& filename, const std::vector<Point3>& points) {
    std::ofstream out_file(filename);
    
    if(!out_file.is_open()){
        throw std::runtime_error("Erreur: le fichier de nuage de points .PLY ne peut être créé !");
    }

    // Header

    out_file << "ply\n"
            << "format ascii 1.0\n"
            << "element vertex " << points.size() << "\n"
            << "property float x\n"
            << "property float y\n"
            << "property float z\n"
            << "end_header\n";
    
    // Contenu : X Y Z

    for(const auto& p : points) {
        out_file << p.x() << " " << p.y() << " " << p.z() << "\n";
    }

    SIM_INFO("Fichier PLY : {} écrit !", filename);

    out_file.close();
}