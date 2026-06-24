#include "lidar_scanner.h"
#include "logger.h"

std::vector<Point3> LidarScanner::scan(const LidarEntity& lidar_ent, const Scene& scene, bool apply_noise, double duration, unsigned int number_of_thread) const
{
    const LidarConfig& config = lidar_ent.config();

    std::vector<Ray3> rays = lidar_ent.generate_rays(duration);

    /// inspiré de the Cherno https://www.youtube.com/watch?v=5HWCsmE9DrE et des cours de Mr Morin

    unsigned int num_threads = number_of_thread;
    size_t chunck_size = rays.size() / num_threads;

    std::vector<std::future<std::vector<Point3>>> futures;
    size_t size_point_cloud = 0;

    for(size_t t = 0; t < num_threads; ++t){
        size_t start = t * chunck_size;
        if(start >= rays.size()) break; // plus de rayons
        size_t end = std::min(start + chunck_size, rays.size());

        futures.push_back(std::async(std::launch::async, 
            [start, end, 
            &rays, &scene, 
            &config, apply_noise, &lidar_ent, &size_point_cloud]()
            {
                std::vector<Point3> chunk_point_cloud;
                chunk_point_cloud.reserve((end-start)/10);// Arbitraire

                for(size_t i = start; i < end; ++i){
                    const Ray3 &ray = rays[i];
                    auto hit = scene.intersect(ray);

                    if (hit.has_value()){
                        if (hit->distance >= config.m_min_dist 
                            && hit->distance <= config.m_max_dist){

                            double noisy_dist = apply_noise ? lidar_ent.noisy_distance(hit->distance) : hit->distance ;

                            Vector3 dir = ray.to_vector();

                            Point3 noisy_point = ray.source() + (dir * noisy_dist);

                            chunk_point_cloud.push_back(noisy_point);
                        }
                    }
                }

                size_point_cloud += chunk_point_cloud.size();

                return chunk_point_cloud;
            }));
    }


    /// Wait les futures
    std::vector<Point3> pointCloud;

    pointCloud.reserve(size_point_cloud);
    for(auto& f : futures){
        std::vector<Point3> chunck_point_cloud = f.get();
        pointCloud.insert(pointCloud.end(), chunck_point_cloud.begin(), chunck_point_cloud.end());
    }


    if (pointCloud.empty())
    {
        SIM_WARNING("Aucune intersection trouvée !");
    }
    return pointCloud;
}