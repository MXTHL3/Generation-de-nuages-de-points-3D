#include <iostream>
#include <fstream>
#include <vector>
#include <memory>

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

#include "pose.hpp"
#include "lidar.hpp"
#include "entity.hpp"
#include "scene.hpp"
#include "lidar_factory.hpp"
#include "point_cloud_exporter.hpp"

int main(int argc, char* argv[]) {
    try {
        Scene world;
        auto mesh_triangles = std::make_shared<Object>();

        if(argc > 1) {
            std::string file_in = argv[1];

            CGAL::Surface_mesh<CGAL_Point_3> mesh;

            if(!CGAL::IO::read_polygon_mesh(file_in, mesh, CGAL::parameters::verbose(true))){
                return -1;
            }
    
            // On veut que des triangles
            if(!CGAL::is_triangle_mesh(mesh)){
                CGAL::Polygon_mesh_processing::triangulate_faces(mesh);
            }

            // mesh.faces() donne uniquement un index !
            for(const auto& f : mesh.faces()){
                auto h = mesh.halfedge(f);

                CGAL_Point_3 p0 = mesh.point(mesh.source(h));
                CGAL_Point_3 p1 = mesh.point(mesh.target(h));
                CGAL_Point_3 p2 = mesh.point(mesh.target(mesh.next(h)));

                mesh_triangles->m_triangles.push_back(Triangle3(p0, p1, p2));
            }

        }else{

            mesh_triangles->m_triangles.push_back(Triangle3(
                Point3(5, -5, 0), Point3(5, 5, 0), Point3(5, 5, 10)
            ));

            mesh_triangles->m_triangles.push_back(Triangle3(
                Point3(5, -5, 0), Point3(5, 5, 10), Point3(5, -5, 10)
            ));
        }
        auto human1Entity = std::make_shared<StaticEntity>(mesh_triangles, Pose({10, 0, 0}, 0, 90, 0));
        auto human2Entity = std::make_shared<StaticEntity>(mesh_triangles, Pose({8, 5, 0}, 0, 0, 0));
        auto human3Entity = std::make_shared<StaticEntity>(mesh_triangles, Pose({0, -7, 0}, 90, 0, 45));
        auto human4Entity = std::make_shared<StaticEntity>(mesh_triangles, Pose({16, 2, 0}, 0, 0, 0));
        auto human5Entity = std::make_shared<StaticEntity>(mesh_triangles, Pose({1, 5, 0}, 0, 0, 45));
        auto human6Entity = std::make_shared<StaticEntity>(mesh_triangles, Pose({-4, -7, 0}, 45, 0, 90));
        auto human7Entity = std::make_shared<StaticEntity>(mesh_triangles, Pose({-12, -5, 0}, 0, 45, 0));
        auto human8Entity = std::make_shared<StaticEntity>(mesh_triangles, Pose({-8, 5, 0}, 0, 0, 0));
        auto human9Entity = std::make_shared<StaticEntity>(mesh_triangles, Pose({-3, 2, 0}, 90, 0, 180));
        auto human10Entity = std::make_shared<StaticEntity>(mesh_triangles, Pose({10, 9, 0}, 0, 0, 0));
        
        world.addEntity(human1Entity);
        world.addEntity(human2Entity);
        world.addEntity(human3Entity);
        world.addEntity(human4Entity);
        world.addEntity(human5Entity);
        world.addEntity(human6Entity);
        world.addEntity(human7Entity);
        world.addEntity(human8Entity);
        world.addEntity(human9Entity);
        world.addEntity(human10Entity);
        


        auto ousterModel = LidarFactory::createFromJsonConfig("lidars_config/ouster_os2_128.json");
        
        Pose lidarPose(Point3(0, 0, 2));
        LidarEntity myLidar(ousterModel, 2, lidarPose);

        world.build();

        std::vector<Point3> resultCloud;
        double h_step = myLidar.h_step();

        for(double hr = 0.0; hr < 360.0; hr += h_step) {
            std::vector<Ray3> rays = myLidar.scan(hr);
            for(const Ray3& ray : rays){
                double dist;
                if(world.intersect(ray, dist)){
                    if(dist >= ousterModel->m_min_dist && dist <= ousterModel->m_max_dist){
                        resultCloud.push_back(ray.point(dist));
                    }
                }
            }
        }

        PlyExporter exporter;
        exporter.save("test.ply", resultCloud);
    } catch(const std::exception& e){
        std::cerr << "Erreur : " << e.what() << std::endl;
        return 1;
    }
    return 0;
}