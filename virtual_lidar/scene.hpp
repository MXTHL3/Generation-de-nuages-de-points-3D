#ifndef SCENE_HPP
#define SCENE_HPP

#include "entity.hpp"
#include <CGAL/AABB_tree.h>
#include <CGAL/AABB_traits.h>
#include <CGAL/AABB_triangle_primitive.h>

typedef CGAL::AABB_triangle_primitive<K, std::vector<Triangle3>::iterator> Primitive;
typedef CGAL::AABB_traits<K, Primitive> Traits;
typedef CGAL::AABB_tree<Traits> Tree;

// Interressant pour la récupération des faces intersectées pour l'interface graphique
struct Intersection {
    Point3 point;
    double distance;
    size_t face_index;
};


class Scene {
public:
    // ajoute un objet à la scène
    // TODO:: pas sur pour le shared_ptr
    void addEntity(std::shared_ptr<StaticEntity> ent);

    void addLidar(std::shared_ptr<LidarEntity> lidar_ent);

    std::vector<Point3> scan(size_t lidar_id = 0) const;

    // ajoute des meshs dans la scène
    void build();

    // retourne le point d'intersection du rayon
    boost::optional<Intersection> intersect(const Ray3& ray) const;

private:
    // les objets à rendre dans la scène
    std::vector<std::shared_ptr<StaticEntity>> m_objects;

    // les capteurs de la scène
    std::vector<std::shared_ptr<LidarEntity>> m_lidars;

    // les triangles à rendre dans la scène
    std::vector<Triangle3> m_triangles;
    // structure d'arbre de la scene pour calculer un rendu plus rapide O(log n) > O(n)
    std::unique_ptr<Tree> m_tree;
};

#endif