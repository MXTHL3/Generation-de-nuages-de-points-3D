#include "lidar_scanner.h"
#include "logger.h"

std::vector<Point3> LidarScanner::scan(const LidarEntity& lidar_ent, const Scene& scene) const
{
    std::vector<Point3> pointCloud;
    const LidarConfig& config = lidar_ent.config();
    std::vector<Ray3> rays = lidar_ent.generate_rays();

    for(const Ray3 &ray : rays){
        auto hit = scene.intersect(ray);

        if (hit){
            if (hit->distance >= config.m_min_dist && hit->distance <= config.m_max_dist){
                double noisy_dist = lidar_ent.noisy_distance(hit->distance);
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