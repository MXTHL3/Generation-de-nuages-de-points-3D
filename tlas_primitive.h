#pragma once

/// Architecture BLAS/TLAS
/// Le lancé de rayons utilise deux niveaux d'arbres AABB :
///    - BLAS (Bottom Level) : un arbre AABB par maillage/liste de triangles (Object), construit une seule fois qui utilise l'arbre AABB par défaut qui a un AABB_triangle_primitive.
///    - TLAS (Top Level): un arbre AABB de boîte englobantes (1 par entité de la scène), reconstruit pour chaque scène. Il utilise une primitive custom un Iso_Cuboid_3 qui est une boite 3D dont les faces sont parralèles aux X,Y,Z.
///      Bbox_3 est juste du stockage et n'est pas forcément parralèle aux axes X, Y, Z du repère monde alors que cette structure peut intersecter un rayon.

/// Pour être acceptée par CGAL comme primitive custom, on a créé une classe qui satisfait le concept de AABBprimitive définie dans les exemples :
/// https://doc.cgal.org/latest/AABB_tree/AABB_tree_2AABB_custom_example_8cpp-example.html
/// https://doc.cgal.org/latest/AABB_tree/AABB_tree_2AABB_polyhedron_facet_intersection_example_8cpp-example.html

/// La primitive doit fournir :
///    - Id : type identifiant
///    - Point : type point pour le calcul du referentiel (placé au milieu de la boite)
///    - Datum : type geometrique interserctable avec un rayon
///    - id() : retourne l'Id
///    - datum() : retourne l'objet de type Datum
///    - reference_point(): retourne le point représentatif

#include "entity.h"
#include <CGAL/AABB_tree.h>
#include <CGAL/AABB_traits.h>
#include <CGAL/Bbox_3.h>

#include <optional>

typedef CGAL::Simple_cartesian<double> K;
typedef CGAL::Iso_cuboid_3<K> Iso_cuboid_3;


class TlasPrimitive {
public:

    /// Types requis par l'AABBPrimitive
    typedef size_t Id;
    typedef Iso_cuboid_3 Datum;
    typedef K::Point_3 Point;

    /// @brief Constructeur par défaut requis
    TlasPrimitive(): m_id(0){}

    /// @brief Construit une primitive depuis un index d'entité et sa bbox dans le repère de la scène
    /// @param id Index de l'entité dans le vecteur d'entités de la scène. 
    /// @param bbox Boite englobante de l'entité en coordonnées de la scène.
    TlasPrimitive(size_t id, const CGAL::Bbox_3& bbox)
        :m_id(id),
        m_cuboid(Point(bbox.xmin(), bbox.ymin(), bbox.zmin()),
                Point(bbox.xmax(), bbox.ymax(), bbox.zmax())) {}
    
    /// @brief Constructeur depuis un itérateur requis
    template<typename Iterator>
    TlasPrimitive(Iterator it) : TlasPrimitive(*it) {}
    
    /// @brief Retourne l'index de l'entité
    Id id() const;

    /// @brief  Retourne la boite englobante pour l'intersection 
    Datum datum() const;

    /// @brief Retourne le point de reference : le centre de la boite
    Point reference_point() const;
private:
    Id m_id; ///< Index de l'entité dans la scène
    Datum m_cuboid; ///< Boit eneglobante en coordonnées de la scène
};

// Structures customs créées à partir de notre primitive custom
typedef CGAL::AABB_traits<K, TlasPrimitive> TlasTraits;
typedef CGAL::AABB_tree<TlasTraits> TlasTree;