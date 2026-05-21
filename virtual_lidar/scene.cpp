#include "scene.hpp"

#include <iterator>

void Scene::add_entity(std::shared_ptr<IEntity> ent) { 
    m_entities.push_back(ent);

    if(auto lidar = std::dynamic_pointer_cast<LidarEntity>(ent)){
        m_lidars.push_back(lidar.get());
    }
}

void Scene::build(){
    m_triangles.clear();
    for(auto& ent : m_entities) {
        auto staticEnt = std::dynamic_pointer_cast<StaticEntity>(ent);

        if(staticEnt) {
            Transform3 xform = staticEnt->transform();
            const auto& local_tris = staticEnt->meshTriangles();

            for(const auto& tri : local_tris) {
                m_triangles.push_back(tri.transform(xform));
            }
        }
    }

    m_tree = std::make_unique<Tree>(m_triangles.begin(), m_triangles.end());

    m_tree->accelerate_distance_queries();
}

boost::optional<Intersection> Scene::intersect(const Ray3& ray) const {
    if(!m_tree || m_tree->empty()) return boost::none;

    auto impact = m_tree->first_intersection(ray);

    if(!impact) return boost::none;
    
    const Point3* impact_point = boost::get<Point3>(&(impact->first));
    
    if(!impact_point) return boost::none;

    size_t triangle_index = impact->second.base() - m_triangles.data();

    Vector3 diff = *impact_point - ray.source();
    double distance = std::sqrt(CGAL::to_double(diff.squared_length()));

    return Intersection{ *impact_point, distance, triangle_index };
}

std::vector<Point3> Scene::scan(std::size_t lidar_id) const{
    if(lidar_id < m_lidars.size()){
        
        std::vector<Point3> pointCloud;
        LidarEntity* lidar_ent = m_lidars[lidar_id];

        double fov_h = lidar_ent->fov_h();
        double step = lidar_ent->h_step();
        auto config = lidar_ent->config();

        for(double hr = 0.0; hr < fov_h; hr += step){
            std::vector<Ray3> rays = lidar_ent->scan(hr);
            
            for(const Ray3& ray : rays){
                auto hit = intersect(ray);

                if(hit){
                    if(hit->distance >= config.m_min_dist && hit->distance){

                        double noisy_dist = lidar_ent->noisy_distance(hit->distance);
                        
                        Vector3 dir = ray.to_vector();

                        dir = dir / std::sqrt(CGAL::to_double(dir.squared_length()));
                        
                        Point3 noisy_point = ray.source() + (dir * noisy_dist);
                        
                        pointCloud.push_back(noisy_point);
                    }
                }
            }
        }

        return pointCloud;
    }
    throw std::runtime_error("L'index donné qui correspond à une 'entité Lidar' pour scanner la scène n'existe pas !");

}