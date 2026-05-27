#include "entity.h"
// STATIC ENTITY
StaticEntity::StaticEntity(std::shared_ptr<Object> obj, Pose p)
    : m_object(obj), m_pose(p) {}

Transform3 StaticEntity::getTransform() const { return m_pose.getTransform(); }

void StaticEntity::update(double) {}

// LIDAR ENTITY
LidarEntity::LidarEntity(std::shared_ptr<Lidar> model, unsigned int step_index, Pose p)
    : m_model(model), m_step_index(step_index), m_pose(p) {}

Transform3 LidarEntity::getTransform() const { return m_pose.getTransform(); }

void LidarEntity::update(double) {}

const Lidar& LidarEntity::config() const { return *m_model; }

const double& LidarEntity::h_step() const { return m_model->m_h_step.at(m_step_index); }

std::vector<Ray3> LidarEntity::scan(double h_deg) const {
    Transform3 world_xf = getTransform();
    double h_rad = h_deg * M_PI / 180.0;
    std::vector<Ray3> rays;

    for(const auto& laser : m_model->m_lasers) {
        double h = h_rad + laser.h_off;

        // Conversion coordonnées sphériques en cartésiennes
        /*
        Vector3 dir(std::cos(laser.v_rad) * std::cos(h),
                    std::cos(laser.v_rad) * std::sin(h),
                    std::sin(laser.v_rad));
        */
       // Forward = -Z, Up = Y, Right = X (convention OpenGL)
        Vector3 dir(-std::cos(laser.v_rad) * std::sin(h),   // X
                    std::sin(laser.v_rad),                   // Y (up)
                    -std::cos(laser.v_rad) * std::cos(h));   // Z (forward = -Z)

        rays.push_back({m_pose.m_position, world_xf.transform(dir)});
    }

    return rays;
};
