#pragma once

#include "cgal.h"
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <CGAL/Polygon_mesh_processing/IO/polygon_mesh_io.h>
#include <CGAL/Polygon_mesh_processing/triangulate_faces.h>

/// @brief Implémentation concrète de Cgal pour la construction de maillages.
/// Prend en charge la génération d'un cube procédural et le chargement
/// de fichiers 3D (OBJ en natif, PLY/OFF/STL via CGAL I/O).
class CgalShape : public Cgal {
public:
    /// @brief Construit un cube unitaire centré à l'origine (12 triangles).
    void build_cube_mesh() override;

    /// @brief Charge un maillage depuis un fichier 3D.
    /// Les fichiers OBJ sont parsés manuellement ; PLY, OFF et STL
    /// utilisent CGAL::IO::read_polygon_mesh avec triangulation automatique.
    /// @param filename Chemin vers le fichier source.
    void build_mesh_from_file(const std::string& filename) override;
};