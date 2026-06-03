#include "scene.h"
#include "logger.h"

Scene::Scene(const Scene& other) {
    for (const auto& entity : other.m_entities) {
        if (!entity) continue;
        auto cloned = entity->clone();
        m_entities.push_back(cloned);
        if (auto lidar = std::dynamic_pointer_cast<LidarEntity>(cloned))
            m_lidars.push_back(lidar);
    }
}

void Scene::add_entity(std::shared_ptr<IEntity> ent) {
    m_entities.push_back(ent);
    if (auto lidar = std::dynamic_pointer_cast<LidarEntity>(ent))
        m_lidars.push_back(lidar);
}

void Scene::build() {
    m_triangles.clear();
    for (auto& ent : m_entities) {
        auto staticEnt = std::dynamic_pointer_cast<StaticEntity>(ent);
        if (!staticEnt) continue;

        Transform3 xform = staticEnt->getTransform();
        for (const auto& tri : staticEnt->meshTriangles())
            m_triangles.push_back(tri.transform(xform));
    }
    m_triangles.shrink_to_fit();
    m_tree = std::make_unique<Tree>(m_triangles.begin(), m_triangles.end());
    m_tree->accelerate_distance_queries();
}

boost::optional<Intersection> Scene::intersect(const Ray3& ray) const {
    if (!m_tree || m_tree->empty()) return boost::none;

    auto impact = m_tree->first_intersection(ray);
    if (!impact) return boost::none;

    const Point3* pt = boost::get<Point3>(&(impact->first));
    if (!pt) return boost::none;

    size_t face_index = impact->second.base() - m_triangles.data();
    Vector3 diff = *pt - ray.source();
    double distance = std::sqrt(CGAL::to_double(diff.squared_length()));

    return Intersection{ *pt, distance, face_index };
}

std::vector<Point3> Scene::scan(size_t lidar_id) const {
    std::vector<Point3> cloud;
    if (lidar_id >= m_lidars.size()) return cloud;

    const auto& lidar_ent = m_lidars[lidar_id];
    double fov_h  = lidar_ent->fov_h();
    double step   = lidar_ent->h_step();
    const Lidar& cfg = lidar_ent->config();

    for (double hr = 0.0; hr < fov_h; hr += step) {
        for (const Ray3& ray : lidar_ent->scan(hr)) {
            auto hit = intersect(ray);
            if (!hit) continue;
            if (hit->distance >= cfg.m_min_dist && hit->distance <= cfg.m_max_dist) {
                double noisy_dist = lidar_ent->m_noise_model->apply(hit->distance);
                Vector3 dir = ray.direction().vector();
                double len = std::sqrt(CGAL::to_double(dir.squared_length()));
                cloud.push_back(ray.source() + (dir / len) * noisy_dist);
            }
        }
    }
    return cloud;
}