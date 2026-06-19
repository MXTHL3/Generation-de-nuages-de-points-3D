#include <CGAL/Simple_cartesian.h>
#include <CGAL/AABB_tree.h>
#include <CGAL/AABB_traits.h>
#include <CGAL/AABB_triangle_primitive.h>
#include <CGAL/Triangle_3.h>
#include <CGAL/Bbox_3.h>

typedef CGAL::Simple_cartesian<double> K;
typedef K::Point_3 Point;
typedef K::Ray_3 Ray3;
typedef K::Vector_3 Vector3;
typedef CGAL::Triangle_3<K> Triangle3;
typedef CGAL::Aff_transformation_3<K> Transform3;
typedef CGAL::Bbox_3 Bbox_3;

typedef CGAL::AABB_triangle_primitive<K, std::vector<Triangle3>::iterator> Primitive;
typedef CGAL::AABB_traits<K, Primitive> Traits;
typedef CGAL::AABB_tree<Traits> Tree;

/// @brief Resultat d'une intersection rayon-triangle.
struct Intersection {
    Point point; ///< Point d'impact dans le repère global
    double distance; ///< Ditance entre l'orgine du rayon et le point d'impact (mètres)
    size_t face_index; ///< Indice du triangle intersecté dans le tableau d'une Scene
};


/// @brief Maillage 3D sous forme de triangles.
/// Le maillage peut être partagé entre les entités.
class Object {
public:
    /// @brief Construit l'arbre AABB local (une seule fois au chargement).
    void build_tree();

    /// @brief  Retourne la boite englobante du mesh dans le repère local
    Bbox_3 local_bbox();

    /// @brief Retourne l'Intersection rayon-triangle en repère local
    std::optional<Intersection> intersect(const Ray3& ray) const;

    std::vector<Triangle3> m_triangles; ///< Faces triangulaires du maillage.
private:
    std::unique_ptr<Tree> m_blas_tree; ///< Arbre AABB des triangles complexité 0(log N) en moyenne
};