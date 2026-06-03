#pragma once

#include "entity.h"
#include <CGAL/AABB_tree.h>
#include <CGAL/AABB_traits.h>
#include <CGAL/AABB_triangle_primitive.h>
#include <boost/optional.hpp>

typedef CGAL::AABB_triangle_primitive<K, std::vector<Triangle3>::iterator> Primitive;
typedef CGAL::AABB_traits<K, Primitive> Traits;
typedef CGAL::AABB_tree<Traits> Tree;

struct Intersection {
    Point3 point;
    double distance;
    size_t face_index;
};

class Scene {
public:
    Scene() = default;
    ~Scene() { m_triangles.clear(); m_triangles.shrink_to_fit(); m_tree.reset(); }
    Scene(const Scene& other);

    // ajoute un objet à la scène
    // TODO:: pas sur pour le shared_ptr
    void add_entity(std::shared_ptr<IEntity> ent);

    void addEntity(std::shared_ptr<IEntity> ent) { add_entity(ent); }

    // ajoute des meshs dans la scène
    void build();
    
    // retourne le point d'intersection du rayon
    boost::optional<Intersection> intersect(const Ray3& ray) const;

    std::vector<Point3> scan(size_t lidar_id = 0) const;

    const std::vector<std::shared_ptr<IEntity>>& entities() const { return m_entities; }
    size_t triangle_count() const { return m_triangles.size(); }

private:
    // les entités à rendre dans la scène
    std::vector<std::shared_ptr<IEntity>> m_entities;
    std::vector<std::shared_ptr<LidarEntity>> m_lidars;
    // les triangles à rendre dans la scène
    std::vector<Triangle3> m_triangles;
    // structure d'arbre de la scene pour calculer un rendu plus rapide O(log n) > O(n)
    std::unique_ptr<Tree> m_tree;
};