#pragma once

#include "entity.h"
#include <CGAL/AABB_tree.h>
#include <CGAL/AABB_traits.h>
#include <CGAL/AABB_triangle_primitive.h>

typedef CGAL::AABB_triangle_primitive<K, std::vector<Triangle3>::iterator> Primitive;
typedef CGAL::AABB_traits<K, Primitive> Traits;
typedef CGAL::AABB_tree<Traits> Tree;

/// @brief Résultat d'une intersection rayon/triangle dans la scène.
/// Retourné par Scene::intersect, regroupe le point d'impact,
/// la distance depuis l'origine du rayon et l'index de la face touchée.
struct Intersection {
    Point3 point; ///< Coordonnées 3D du point d'impact.
    double distance; ///< Distance entre l'origine du rayon et le point d'impact (mètres).
    size_t face_index; ///< Index du triangle intersecté dans le vecteur de triangles de la scène.
};

/// @brief Scène 3D : ensemble d'entités statiques et arbre AABB pour les intersections.
/// Après ajout des entités, build() triangule la scène et construit l'arbre CGAL AABB
/// permettant des requêtes d'intersection rayon/maillage
class Scene {
public:
    Scene() = default;

    /// @brief Détruit la scène en libérant les triangles et l'arbre AABB.
    ~Scene(){
        m_triangles.clear();
        m_tree.reset();
    }

    /// @brief Constructeur de copie : clone toutes les entités statiques.
    /// @param scene Scène source à copier.
    Scene(const Scene& scene);

    /// @brief Ajoute une entité statique à la scène.
    /// @param ent Entité à ajouter (prend la propriété).
    void add_static_entity(std::unique_ptr<StaticEntity> ent);

    std::vector<Point3> scan(size_t lidar_id = 0, double parameter = 0.0) const;

    /// @brief Triangule toutes les entités et construit l'arbre AABB.
    /// Doit être appelé après tous les add_static_entity et avant tout intersect.
    void build();

    /// @brief Calcule la première intersection d'un rayon avec la scène.
    /// @param ray Rayon à tester.
    /// @return Intersection (point, distance, index de face) ou boost::none si aucune.
    boost::optional<Intersection> intersect(const Ray3& ray) const;

    /// @brief Retourne les entités statiques de la scène (accès en écriture).
    std::vector<std::unique_ptr<StaticEntity>>& entities() { return m_entities; }

    /// @brief Retourne les entités statiques de la scène (accès en lecture).
    const std::vector<std::unique_ptr<StaticEntity>>& entities() const { return m_entities; }

private:
    std::vector<std::unique_ptr<StaticEntity>> m_entities;  ///< Entités statiques de la scène.
    std::vector<Triangle3> m_triangles; ///< Triangles aplatis issus de toutes les entités (après build).
    std::unique_ptr<Tree> m_tree; ///< Arbre AABB CGAL pour les requêtes d'intersection.
};