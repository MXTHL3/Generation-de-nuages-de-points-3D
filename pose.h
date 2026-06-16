#pragma once

#include <CGAL/Simple_cartesian.h>

typedef CGAL::Simple_cartesian<double> K;
typedef K::Point_3 Point3;
typedef K::Vector_3 Vector3;
typedef CGAL::Aff_transformation_3<K> Transform3;

/// @brief Position et rotation d'un objet dans une scène 3D.
/// Une position (x, y, z) et une rotation (x, y, z) avec angles en radians.
class Pose {
public:

    /// @param p Position (x, y, z) en mètres dans le repère global
    /// @param rx Rotation autour de x (en degrés converti en radians)
    /// @param ry Rotation autour de y (en degrés converti en radians)
    /// @param rz Rotation autour de z (en degrés converti en radians)
    Pose(Point3 p = Point3(0, 0, 0), double rx = 0.0, double ry = 0.0, double rz = 0.0);
    
    /// @brief Calcule la matrice de transformation (translation + rotation)
    Transform3 transform() const;

    double rx() const { return m_rx; }
    double ry() const { return m_ry; }
    double rz() const { return m_rz; }
    const Point3& pos() const { return m_position; }

private:
    Point3 m_position;          // Position X, Y, Z
    double m_rx, m_ry, m_rz;    // Rotation X, Y, Z 
};