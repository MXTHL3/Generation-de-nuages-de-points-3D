#pragma once

#include "entity.h"
#include "tlas_primitive.h"
#include <iterator>
#include <optional>

/// @brief Scène 3D contenant les entités statiques et l'arbre d'accélération AABB.
/// La scène stocke les entités, contruit un arbre AABB à partir des maillages des entités transformées et fournit l'intersection rayon_triangle via CGAL.
class Scene {
public:

    Scene() = default;
    ~Scene(){
        m_tlas_primitives.clear();
        m_tlas_tree.reset();
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
    std::optional<Intersection> intersect(const Ray3& ray) const;

    /// @brief Accès en lecture aux entités de la scène
    const std::vector<std::unique_ptr<StaticEntity>>& entities() const { return m_entities; }

private:
    std::vector<std::unique_ptr<StaticEntity>> m_entities; ///< Objets de la scène


    std::vector<TlasPrimitive> m_tlas_primitives; ///<  boites englobantes des objets de la scène
    std::unique_ptr<TlasTree> m_tlas_tree; ///< structure d'arbre AABB TLAS de la scene pour calculer les intersections des boites englobantes avant d'intersecter les maillages BLAS
};