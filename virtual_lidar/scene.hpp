#ifndef SCENE_HPP
#define SCENE_HPP

#include "entity.hpp"
#include <CGAL/AABB_tree.h>
#include <CGAL/AABB_traits.h>
#include <CGAL/AABB_triangle_primitive.h>

typedef CGAL::AABB_triangle_primitive<K, std::vector<Triangle3>::iterator> Primitive;
typedef CGAL::AABB_traits<K, Primitive> Traits;
typedef CGAL::AABB_tree<Traits> Tree;

/// @brief Resultat d'une intersection rayon-triangle.
struct Intersection {
    Point3 point; ///< Point d'impact dans le repère global
    double distance; ///< Ditance entre l'orgine du rayon et le point d'impact (mètres)
    size_t face_index; ///< Indice du triangle intersecté dans le tableau d'une Scene
};

/// @brief Scène 3D contenant les entités statiques et l'arbre d'accélération AABB.
/// La scène stocke les entités, contruit un arbre AABB à partir des maillages des entités transformées et fournit l'intersection rayon_triangle via CGAL.
class Scene {
public:

    Scene() = default;
    ~Scene(){
        m_triangles.clear();
        m_tree.reset();
    }
    /// @brief Constructeur de copie
    Scene(const Scene& scene);

    /// @brief Ajoute une entité statique à la scène
    void add_static_entity(std::unique_ptr<StaticEntity> ent);

    /// @brief Construit l'arbre AABB à partir des triangle transformés.
    /// Doit être appelé pour être mit à jour si des entités sont rajoutées.
    void build();

    /// @brief Lance un rayon et retourne pottentiellement l'intersection la plus proche de l'origine du rayon.
    /// @param ray Rayon à intersecter (origine et direction)
    /// @return L'intersection trouvée s'il en trouve sinon boost::none
    boost::optional<Intersection> intersect(const Ray3& ray) const;

    /// @brief Accès en lecture aux entités de la scène
    const std::vector<std::unique_ptr<StaticEntity>>& entities() const { return m_entities; }

private:
    std::vector<std::unique_ptr<StaticEntity>> m_entities; ///< Objets de la scène


    std::vector<Triangle3> m_triangles; ///< Triangles transformés des maillages de la scène
    std::unique_ptr<Tree> m_tree; ///< structure d'arbre AABB de la scene pour calculer les intersections plus rapidement O(log n) > O(n)
};

#endif