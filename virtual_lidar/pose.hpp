#ifndef POSE_HPP
#define POSE_HPP

#include <CGAL/Simple_cartesian.h>

typedef CGAL::Simple_cartesian<double> K;
typedef K::Point_3 Point3;
typedef K::Vector_3 Vector3;
typedef CGAL::Aff_transformation_3<K> Transform3;

// Represente la position et la rotation d'un objet de la scene 3D
class Pose {
public:

    Pose(Point3 p = Point3(0, 0, 0), double rx = 0.0, double ry = 0.0, double rz = 0.0);
    
    // Calcule la matrice de transformation
    Transform3 transform() const;

    double rx() const { return m_rx; }
    double ry() const { return m_ry; }
    double rz() const { return m_rz; }
    const Point3& pos() const { return m_position; }

private:
    Point3 m_position;          // Position X, Y, Z
    double m_rx, m_ry, m_rz;    // Rotation X, Y, Z 
};

#endif