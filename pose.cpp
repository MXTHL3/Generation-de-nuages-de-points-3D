#include "pose.h"
#include "units.h"

Pose::Pose(Point3 p, double rx, double ry, double rz)
    : m_position(p), m_rx(to_radians(rx)), m_ry(to_radians(ry)), m_rz(to_radians(rz)){ 
    }

Transform3 Pose::transform() const {
    Transform3 rotX(1.0, 0.0, 0.0,
                    0.0, std::cos(m_rx), -std::sin(m_rx),
                    0.0, std::sin(m_rx), std::cos(m_rx));
    
    Transform3 rotY(std::cos(m_ry), 0.0, std::sin(m_ry),
                    0.0, 1, 0.0,
                    -std::sin(m_ry), 0.0, std::cos(m_ry));

    Transform3 rotZ(std::cos(m_rz), -std::sin(m_rz), 0.0,
                    std::sin(m_rz), std::cos(m_rz), 0.0,
                    0.0, 0.0, 1.0);

    return Transform3(CGAL::TRANSLATION, m_position - Point3(0, 0, 0)) * rotX * rotY * rotZ;
}
