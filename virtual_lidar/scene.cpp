#include "scene.hpp"

void Scene::addEntity(std::shared_ptr<IEntity> ent) { m_entities.push_back(ent); }

void Scene::build(){
    m_triangles.clear();
    for(auto& ent : m_entities) {
        auto staticEnt = std::dynamic_pointer_cast<StaticEntity>(ent);

        if(staticEnt && staticEnt->getMesh()) {
            Transform3 xform = staticEnt->getTransform();
            const auto& local_tris = staticEnt->getMesh()->m_triangles;

            for(const auto& tri : local_tris) {
                m_triangles.push_back(tri.transform(xform));
            }
        }
    }

    m_tree = std::make_unique<Tree>(m_triangles.begin(), m_triangles.end());

    m_tree->accelerate_distance_queries();
}

boost::optional<Point3> Scene::intersect(const Ray3& ray, double& distance) const {
    if(!m_tree && m_tree->empty()) return boost::none;

    auto impact = m_tree->first_intersection(ray);

    if(impact) {
        const Point3* impact_point = boost::get<Point3>(&(impact->first));

        if(impact_point) {
            Vector3 diff = *impact_point - ray.source();
            distance = std::sqrt(CGAL::to_double(diff.squared_length()));
            return *impact_point;
        }
    }

    return boost::none;
}