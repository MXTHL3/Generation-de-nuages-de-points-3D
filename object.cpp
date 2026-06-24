#include "object.h"

void Object::build_tree(){
    m_blas_tree = std::make_unique<Tree>(m_triangles.begin(), m_triangles.end());
    m_blas_tree->accelerate_distance_queries();
}

CGAL::Bbox_3 Object::local_bbox()
{
    if (!m_blas_tree)
    {
        SIM_ERROR(
            "local_bbox() appelé sur un objet sans arbre AABB. "
            "Triangles={}",
            m_triangles.size());

        throw std::runtime_error("m_blas_tree est nullptr");
    }

    return m_blas_tree->bbox();
}

std::optional<Intersection> Object::intersect(const Ray3& ray) const {
    if(!m_blas_tree || m_blas_tree->empty()) return std::nullopt;

    //auto impact = m_blas_tree->first_intersection(ray);
    decltype(m_blas_tree->first_intersection(ray)) impact = boost::none;

    impact = m_blas_tree->first_intersection(ray);

    if(!impact) return std::nullopt;
        
    const Point* impact_point = boost::get<Point>(&(impact->first));
        
    if(!impact_point) return std::nullopt;

    size_t triangle_index = impact->second.base() - m_triangles.data();
        
    Vector3 diff = *impact_point - static_cast<Point>(ray.source());
        
    double dist = std::sqrt(CGAL::to_double(diff.squared_length()));
    return Intersection{ *impact_point, dist, triangle_index };
}