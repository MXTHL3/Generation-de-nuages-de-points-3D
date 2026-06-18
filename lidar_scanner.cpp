#include "lidar_scanner.h"
#include "logger.h"

std::vector<Point3> LidarScanner::scan(const LidarEntity& lidar_ent,
    const Scene& scene, bool apply_noise,
    double duration, unsigned int number_of_thread) const
{
    const LidarConfig& config = lidar_ent.config();
    std::vector<Ray3> rays = lidar_ent.generate_rays(duration);

    size_t chunk_size = rays.size() / number_of_thread;
    std::vector<std::future<std::vector<Point3>>> futures;

    for (size_t t = 0; t < number_of_thread; ++t) {
        size_t start = t * chunk_size;
        if (start >= rays.size()) break;
        size_t end = std::min(start + chunk_size, rays.size());

        futures.push_back(std::async(std::launch::async,
            [start, end, &rays, &scene, &config, apply_noise, &lidar_ent]() {
                std::vector<Point3> chunk;
                chunk.reserve((end - start) / 10);

                for (size_t i = start; i < end; ++i) {
                    const Ray3& ray = rays[i];
                    auto hit = scene.intersect(ray);
                    if (hit) {
                        if (hit->distance >= config.m_min_dist
                            && hit->distance <= config.m_max_dist) {
                            double d = apply_noise
                                ? lidar_ent.noisy_distance(hit->distance)
                                : hit->distance;
                            Vector3 dir = ray.to_vector();
                            chunk.push_back(ray.source() + dir * d);
                        }
                    }
                }
                return chunk;
            }));
    }

    std::vector<Point3> pointCloud;
    for (auto& f : futures) {
        auto chunk = f.get();
        pointCloud.insert(pointCloud.end(), chunk.begin(), chunk.end());
    }

    if (pointCloud.empty())
        SIM_WARNING("Aucune intersection trouvée !");

    return pointCloud;
}