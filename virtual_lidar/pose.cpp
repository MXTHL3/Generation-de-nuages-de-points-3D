#include "pose.hpp"

Pose::Pose(Point3 p, double rx, double ry, double rz)
    : m_position(p), m_rx(rx), m_ry(ry), m_rz(rz){ 
    }

Transform3 Pose::getTransform() const {
    // TODO :: à voir un passage à glm
    double rx_rad = m_rx * M_PI / 180.0;
    double ry_rad = m_ry * M_PI / 180.0;
    double rz_rad = m_rz * M_PI / 180.0;

    Transform3 rotX(1.0, 0.0, 0.0,
                    0.0, std::cos(rx_rad), -std::sin(rx_rad),
                    0.0, std::sin(rx_rad), std::cos(rx_rad));
    
    Transform3 rotY(std::cos(ry_rad), 0.0, std::sin(ry_rad),
                    0.0, 1, 0.0,
                    -std::sin(ry_rad), 0.0, std::cos(ry_rad));

    Transform3 rotZ(std::cos(rz_rad), -std::sin(rz_rad), 0.0,
                    std::sin(rz_rad), std::cos(rz_rad), 0.0,
                    0.0, 0.0, 1.0);

    return Transform3(CGAL::TRANSLATION, m_position - Point3(0, 0, 0)) * rotX * rotY * rotZ;
}
