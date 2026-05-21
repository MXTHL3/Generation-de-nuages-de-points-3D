#pragma once

#include "entity.h"
#include <CGAL/AABB_tree.h>
#include <CGAL/AABB_traits.h>
#include <CGAL/AABB_triangle_primitive.h>

typedef CGAL::AABB_triangle_primitive<K, std::vector<Triangle3>::iterator> Primitive;
typedef CGAL::AABB_traits<K, Primitive> Traits;
typedef CGAL::AABB_tree<Traits> Tree;

class Scene {
public:
    // ajoute un objet à la scène
    // TODO:: pas sur pour le shared_ptr
    void addEntity(std::shared_ptr<IEntity> ent);
    // ajoute des meshs dans la scène
    void build();
    // retourne le point d'intersection du rayon
    boost::optional<Point3> intersect(const Ray3& ray, double& distance) const;
    size_t triangle_count() const { return m_triangles.size(); }

private:
    // les entités à rendre dans la scène
    std::vector<std::shared_ptr<IEntity>> m_entities;
    // les triangles à rendre dans la scène
    std::vector<Triangle3> m_triangles;
    // structure d'arbre de la scene pour calculer un rendu plus rapide O(log n) > O(n)
    std::unique_ptr<Tree> m_tree;
};