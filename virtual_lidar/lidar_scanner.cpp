#include "lidar_scanner.hpp"
#include "logger.hpp"

std::vector<Point3> LidarScanner::scan(const LidarEntity& lidar_ent, const Scene& scene, bool apply_noise, double duration) const
{

    std::vector<Point3> pointCloud;

    const LidarConfig& config = lidar_ent.config();

    std::vector<Ray3> rays = lidar_ent.generate_rays(duration);

    for(const Ray3 &ray : rays){
        auto hit = scene.intersect(ray);

        if (hit){
            if (hit->distance >= config.m_min_dist && hit->distance <= config.m_max_dist){

                double noisy_dist = apply_noise ? lidar_ent.noisy_distance(hit->distance) : hit->distance ;

                Vector3 dir = ray.to_vector();

                dir = dir / std::sqrt(CGAL::to_double(dir.squared_length()));

                Point3 noisy_point = ray.source() + (dir * noisy_dist);

                pointCloud.push_back(noisy_point);
            }
        }
    }

    if (pointCloud.empty())
    {
        SIM_WARNING("Aucune intersection trouvée !");
    }
    return pointCloud;
}