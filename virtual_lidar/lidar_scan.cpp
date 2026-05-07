#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <cmath>
#include <limits>

#include <CGAL/Simple_cartesian.h>
#include <CGAL/Point_3.h>
#include <CGAL/Vector_3.h>
#include <CGAL/Polyhedron_3.h>
#include <CGAL/Ray_3.h>
#include <CGAL/intersections.h>
#include <CGAL/Aff_transformation_3.h>
#include <CGAL/Surface_mesh.h>
#include <CGAL/Polygon_mesh_processing/triangulate_faces.h>
#include <CGAL/Polygon_mesh_processing/transform.h>

typedef CGAL::Simple_cartesian<double> Kernel;
typedef Kernel::Point_3 CGAL_Point_3;
typedef Kernel::Vector_3 CGAL_Vector_3;
typedef Kernel::Ray_3 CGAL_Ray_3;
typedef Kernel::Triangle_3 CGAL_Triangle_3;
typedef CGAL::Surface_mesh<CGAL_Point_3> CGAL_Mesh;
typedef CGAL::Aff_transformation_3<Kernel> CGAL_Transformation;

using namespace std;

bool find_near_point(const CGAL_Ray_3& ray, const std::vector<CGAL_Triangle_3>& triangles,const CGAL_Point_3& origin, CGAL_Point_3& point_intersect){
    bool intersection_found = false;
    double min_distance = numeric_limits<double>::max();

    for(const CGAL_Triangle_3& triangle : triangles){
        // On doit faire un cast pour 
        CGAL::Object result = CGAL::intersection(ray, triangle);
        
        const CGAL_Point_3* p = CGAL::object_cast<CGAL_Point_3>(&result);
        
        if (p) {
            CGAL_Point_3 cur_point_intersect = *p;

            // On veut récupérer uniquement le point d'intersection la plus proche du lidar
            double distance = CGAL::squared_distance(origin, cur_point_intersect);
            if(distance < min_distance){
                min_distance = distance;
                point_intersect = cur_point_intersect;
                intersection_found = true;
            }
        }
    }
    return intersection_found;
}

int main(){
    string in_filepath, out_filepath;
    double mesh_co_x, mesh_co_y, mesh_co_z;
    // Rotation mesh
    double angle_x, angle_y, angle_z;
    // FOV domaine vertical et horizontal
    double fov_v_min, fov_v_max, fov_h_min, fov_h_max;

    
    std::cout << "Bienvenue sur le scan lidar virtuel !" << std::endl;
    std::cout << "Veuillez donner le path du fichier du maillage à transformer en nuage de point :"<<std::endl;

    std::cin >> in_filepath;

    CGAL_Mesh mesh;

    if(!CGAL::IO::read_polygon_mesh(in_filepath, mesh, CGAL::parameters::verbose(true))){
        return -1;
    }
    
    // On veut que des triangles
    if(!CGAL::is_triangle_mesh(mesh)){
        CGAL::Polygon_mesh_processing::triangulate_faces(mesh);
    }

    // x,y,z du mesh
    std::cout<< "Veuillez donner les coordonnées de votre mesh X,Y,Z par rapport au lidar" << std::endl;
    std::cin >> mesh_co_x >> mesh_co_y >> mesh_co_z;

    std::cout << "Lu :" << " X:" << mesh_co_x << " Y:" << mesh_co_y << " Z:" << mesh_co_z << std::endl;

    // agnles rotation du mesh
    std::cout<< "Veuillez donner les angles de rotation en degrés de votre objet (ex: 45 90 0)" << std::endl;
    std::cin >> angle_x >> angle_y >> angle_z;
    
    std::cout << "Lu :" << " X:" << angle_x << " Y:" << angle_y << " Z:" << angle_z << std::endl;

    std::cout << "Veuillez donner le domaine de l'angle horizontal du FOV (ex: -180 180)" << std::endl;
    std::cin >> fov_h_min >> fov_h_max;

    std::cout << "Lu :" << " H MIN: " << fov_h_min << " H MAX:" << fov_h_max << std::endl;

    std::cout << "Veuillez donner le domaine de l'angle vertical du FOV (ex: -180 180)" << std::endl;
    std::cin >> fov_v_min >> fov_v_max;

    std::cout << "Lu :" << " V MIN:" << fov_v_min << " V MAX:" << fov_v_max << std::endl;

    std::cout << "Veuillez donner le path du fichier du nuage de points de sortie"<<std::endl;
    std::cin >> out_filepath;

    double angle_rad_x = angle_x * (M_PI / 180.0);
    double angle_rad_y = angle_y * (M_PI / 180.0);
    double angle_rad_z = angle_z * (M_PI / 180.0);

    // Translation
    CGAL_Transformation mesh_translate(CGAL::TRANSLATION, CGAL_Vector_3(mesh_co_x, mesh_co_y, mesh_co_z));

    // Rotations
    // A voir CGAL::Rotation existe mais pas documentée donc on utilise matrice de rotation définie à la main  
    double c = cos(angle_rad_x);
    double s = sin(angle_rad_x);
    CGAL_Transformation mesh_rotate_x(1, 0, 0,
                                0, c, -s,
                                0, s, c);
    
    c = cos(angle_rad_y);
    s = sin(angle_rad_y);                            
    CGAL_Transformation mesh_rotate_y(c, 0, -s,
                                0, 1, 0,
                                s, 0, c);
    
    c = cos(angle_rad_z);
    s = sin(angle_rad_z);
    CGAL_Transformation mesh_rotate_z(c, -s, 0,
                                s, c, 0,
                                0, 0, 1);                            
    
    CGAL_Transformation mesh_transform = mesh_translate * mesh_rotate_x * mesh_rotate_y * mesh_rotate_z;

    CGAL::Polygon_mesh_processing::transform(mesh_transform, mesh);


    // resolutions lidar à définir comme paramètre ou alors le nbr de points total sur le scan mais un peu dur à calculer la disposition
    double res_h = 0.5;
    double res_v = 0.5;
    
    std::vector<CGAL_Triangle_3> mesh_triangles;

    // mesh.faces() donne uniquement un index !
    for(const auto& f : mesh.faces()){
        auto h = mesh.halfedge(f);

        CGAL_Point_3 p0 = mesh.point(mesh.source(h));
        CGAL_Point_3 p1 = mesh.point(mesh.target(h));
        CGAL_Point_3 p2 = mesh.point(mesh.target(mesh.next(h)));

        mesh_triangles.push_back(CGAL_Triangle_3(p0, p1, p2));
    }

    CGAL_Point_3 lidar_origin{0, 0, 0};

    vector<CGAL_Point_3> points_cloud;

    for(double angle_h = fov_h_min; angle_h <= fov_h_max; angle_h += res_h){
        double cur_angle_h_rad = angle_h * (M_PI / 180.0);
        
        for(double angle_v = fov_v_min; angle_v <= fov_v_max; angle_v += res_v){
            
            double cur_angle_v_rad = angle_v * (M_PI / 180.0);
            
            // Conversion en coordonnées sphériques
            double dx = cos(cur_angle_v_rad) * cos(cur_angle_h_rad);
            double dy = cos(cur_angle_v_rad) * sin(cur_angle_h_rad);
            double dz = sin(cur_angle_v_rad);

            CGAL_Vector_3 ray_dir{dx, dy, dz};
            CGAL_Ray_3 ray{lidar_origin, ray_dir};
            
            CGAL_Point_3 point_intersect;

            if(find_near_point(ray, mesh_triangles, lidar_origin, point_intersect)){
                points_cloud.push_back(point_intersect);
            }
        }
    }

    std::ofstream out_file(out_filepath);

    // En tete .PLY
    out_file << "ply\n"
            << "format ascii 1.0\n"
            << "element vertex " << points_cloud.size() << "\n"
            << "property float x\n"
            << "property float y\n"
            << "property float z\n"
            << "end_header\n";
    
    // Ecriture valeurs des points X Y Z        
    for(const CGAL_Point_3 p : points_cloud){
        out_file << p.x() << " " << p.y() << " " << p.z() << "\n";
    }
    out_file.close();

    std::cout << "Scan Terminé !" << std::endl;
    std::cout << "Nombre de points : " << points_cloud.size() << std::endl;
    std::cout << "Dans le fichier : " << out_filepath << std::endl;
}

