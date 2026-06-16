#include "scene.hpp"
#include "logger.hpp"
#include "units.hpp"

#include <iterator>

Scene::Scene(const Scene& scene){
    for(const auto& entity : scene.m_entities){
        if(entity){
            // TODO:: a virer rapidement
            auto cloned_entity = entity->clone();

            if(!cloned_entity){
                SIM_WARNING("Echec du clonage d'une entité !");
            }else{
                auto static_entity = std::unique_ptr<StaticEntity>(dynamic_cast<StaticEntity*>(cloned_entity.release()));
                if(!static_entity){
                    SIM_WARNING("dynamic cast de static_entity echoué !");
                }else{
                    m_entities.push_back(std::move(static_entity));
                }
            }
        }
    }
            SIM_DEBUG("Scene Copiée ! {} entités", m_entities.size());
}


void Scene::add_static_entity(std::unique_ptr<StaticEntity> ent) { 
    m_entities.push_back(std::move(ent));
}

void Scene::build(){
    m_triangles.clear();
    for(auto& ent : m_entities) {
        Transform3 xform = ent->transform();
        const auto& local_tris = ent->meshTriangles();

        for(const auto& tri : local_tris) {
            m_triangles.push_back(tri.transform(xform));
        }
    }

    // opti ou pas mais fait pas de mal
    m_triangles.shrink_to_fit();

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

/*std::vector<Point3> Scene::scan(std::size_t lidar_id, double parameter) const{
    if(lidar_id < m_lidars.size()){
        
        std::vector<Point3> pointCloud;
        std::shared_ptr<LidarEntity> lidar_ent = m_lidars[lidar_id];

        std::shared_ptr<LidarConfig> lc = lidar_ent->config();

        std::shared_ptr<MechanicalLidarConfig> config = std::dynamic_pointer_cast<MechanicalLidarConfig>(lc);


        for (double hr = 0.0; hr < 360.0; hr += config->m_h_step[2])
        {
            std::vector<Ray3> rays = lidar_ent->scan(hr);
            for (const Ray3 &ray : rays)
            {
                auto hit = intersect(ray);

                if (hit)
                {
                    if (hit->distance >= config->m_min_dist && hit->distance <= config->m_max_dist)
                    {

                        double noisy_dist = lidar_ent->noisy_distance(hit->distance);
                        
                        Vector3 dir = ray.to_vector();

                        dir = dir / std::sqrt(CGAL::to_double(dir.squared_length()));
                        
                        Point3 noisy_point = ray.source() + (dir * noisy_dist);
                        
                        pointCloud.push_back(noisy_point);
                    }
                }
            }
        }
        if(pointCloud.empty()){
            SIM_WARNING("Aucune intersection trouvée !");
        }
        return pointCloud;
    }
    throw std::runtime_error("L'index donné qui correspond à une 'entité Lidar' pour scanner la scène n'existe pas !");

}*/