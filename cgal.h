#pragma once

#include <CGAL/Simple_cartesian.h>
#include <CGAL/Surface_mesh.h>
#include <vector>

typedef CGAL::Simple_cartesian<double> Kernel;
typedef Kernel::Point_3 Point;
typedef CGAL::Surface_mesh<Point> SurfaceMesh;

/// @brief Interface de base pour la construction et la manipulation d'un maillage CGAL.
/// Fournit les opérations bas-niveau (ajout de sommets, de faces) et l'export
/// des données de rendu. Les sous-classes implémentent les méthodes de construction.
class Cgal {
public:
    virtual ~Cgal() = default;

    /// @brief Construit un maillage cubique unitaire.
    virtual void build_cube_mesh() = 0;

    /// @brief Construit un maillage à partir d'un fichier 3D.
    /// @param filename Chemin vers le fichier source (OBJ, PLY, OFF, STL…).
    virtual void build_mesh_from_file(const std::string& filename) = 0;

    /// @brief Convertit le maillage en tableau de flottants (positions XYZ par triangle).
    /// @return Vecteur de flottants, 9 valeurs par face (3 sommets × 3 coordonnées).
    std::vector<float> to_vertex_data() const;

    /// @brief Accès en lecture au maillage CGAL sous-jacent.
    /// @return Référence constante vers le SurfaceMesh interne.
    const SurfaceMesh& mesh() const { return m_mesh; }

protected:
    SurfaceMesh m_mesh; ///< Maillage CGAL interne.

    /// @brief Ajoute un sommet au maillage.
    /// @param x Coordonnée X.
    /// @param y Coordonnée Y.
    /// @param z Coordonnée Z.
    /// @return Index du sommet créé.
    SurfaceMesh::Vertex_index add_vertex(double x, double y, double z);

    /// @brief Ajoute une face triangulaire au maillage.
    /// @param a Index du premier sommet.
    /// @param b Index du deuxième sommet.
    /// @param c Index du troisième sommet.
    void add_triangle(SurfaceMesh::Vertex_index a,
                      SurfaceMesh::Vertex_index b,
                      SurfaceMesh::Vertex_index c);
};