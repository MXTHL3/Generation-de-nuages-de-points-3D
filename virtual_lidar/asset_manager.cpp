#include "asset_manager.hpp"
#include "lidar_factory.hpp"
#include "logger.hpp"

#include <iostream>

#include <CGAL/Simple_cartesian.h>
#include <CGAL/Point_3.h>
#include <CGAL/Triangle_3.h>
#include <CGAL/Polygon_mesh_processing/IO/polygon_mesh_io.h>
#include <CGAL/Surface_mesh.h>
#include <CGAL/Polygon_mesh_processing/triangulate_faces.h>
#include <CGAL/Polygon_mesh_processing/transform.h>


typedef CGAL::Simple_cartesian<double> Kernel;
typedef Kernel::Point_3 CGAL_Point_3;
typedef Kernel::Vector_3 CGAL_Vector_3;
typedef Kernel::Ray_3 CGAL_Ray_3;
typedef Kernel::Triangle_3 CGAL_Triangle_3;
typedef CGAL::Surface_mesh<CGAL_Point_3> CGAL_Mesh;

std::shared_ptr<Object> AssetManager::get_mesh(const std::string& path){
    if(m_mesh_cache.find(path) == m_mesh_cache.end()){
        m_mesh_cache[path] = load_mesh_from_file(path);
    }else{
        SIM_DEBUG("Le fichier lidar : {} est déjà chargé !", path);
    }

    return m_mesh_cache[path];
}

std::shared_ptr<Object> AssetManager::load_mesh_from_file(const std::string& path){
    auto obj = std::make_shared<Object>();

    CGAL_Mesh mesh;


    if (!CGAL::IO::read_polygon_mesh(path, mesh, CGAL::parameters::verbose(true)))
    {
        throw std::runtime_error("Le modèle : "+ path + " ne peut être lu !");
    }

    if (!CGAL::is_triangle_mesh(mesh))
    {
        CGAL::Polygon_mesh_processing::triangulate_faces(mesh);
        SIM_DEBUG("le maillage a été triangulé !");
    }

    for (const auto &f : mesh.faces())
    {
        auto h = mesh.halfedge(f);

        CGAL_Point_3 p0 = mesh.point(mesh.source(h));
        CGAL_Point_3 p1 = mesh.point(mesh.target(h));
        CGAL_Point_3 p2 = mesh.point(mesh.target(mesh.next(h)));

        obj->m_triangles.push_back(Triangle3(p0, p1, p2));
    }

    SIM_INFO("Le fichier mesh : {} a été chargé !", path);
    
    return obj;
}

std::shared_ptr<Lidar> AssetManager::get_lidar_config(const std::string& path){
    if(m_lidar_config_cache.find(path) == m_lidar_config_cache.end()){
        m_lidar_config_cache[path] = load_lidar_from_file(path);
    }else{
        SIM_DEBUG("Le fichier lidar : {} est déjà chargé !", path);
    }
    return m_lidar_config_cache[path];
}

std::shared_ptr<Lidar> AssetManager::load_lidar_from_file(const std::string& path){
    return LidarFactory::createFromJsonConfig(path);
}
