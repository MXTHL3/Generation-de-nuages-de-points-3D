#include "entity.h"
#include "units.h"
#include <cmath>

// STATIC ENTITY

StaticEntity::StaticEntity(std::string name, std::shared_ptr<Object> obj, Pose p, std::string mesh_path)
    : m_name(std::move(name)), m_object(obj), m_pose(p), m_mesh_path(std::move(mesh_path)) {}

std::shared_ptr<IEntity> StaticEntity::clone() const {
    return std::make_shared<StaticEntity>(*this);
}

Transform3 StaticEntity::getTransform() const { return m_pose.getTransform(); }

// LIDAR ENTITY

LidarEntity::LidarEntity(std::shared_ptr<Lidar> model, unsigned int step_index,
                         double fov_h, Pose p,
                         std::unique_ptr<NoiseModel> noise)
    : m_model(model), m_step_index(step_index),
      m_fov_h(to_radians(fov_h)), m_pose(p),
      m_noise_model(std::move(noise)) {}

std::shared_ptr<IEntity> LidarEntity::clone() const {
    return std::make_shared<LidarEntity>(m_model, m_step_index,
                                         to_degrees(m_fov_h), m_pose,
                                         std::make_unique<NullNoiseModel>());
}

Transform3 LidarEntity::getTransform() const { return m_pose.getTransform(); }

const Lidar& LidarEntity::config() const { return *m_model; }

const double& LidarEntity::h_step() const { return m_model->m_h_step.at(m_step_index); }

std::vector<Ray3> LidarEntity::scan(double h_deg) const {
    Transform3 world_xf = getTransform();
    double h_rad = to_radians(h_deg);
    std::vector<Ray3> rays;

    for (const auto& laser : m_model->m_lasers) {
        double h = h_rad + laser.h_off;

        Vector3 dir(std::cos(laser.v_rad) * std::cos(h),
                    std::cos(laser.v_rad) * std::sin(h),
                    std::sin(laser.v_rad));

        Vector3 transformed_dir(
            world_xf.m(0,0)*dir.x() + world_xf.m(0,1)*dir.y() + world_xf.m(0,2)*dir.z(),
            world_xf.m(1,0)*dir.x() + world_xf.m(1,1)*dir.y() + world_xf.m(1,2)*dir.z(),
            world_xf.m(2,0)*dir.x() + world_xf.m(2,1)*dir.y() + world_xf.m(2,2)*dir.z()
        );

        rays.push_back({m_pose.m_position, transformed_dir});
    }

    return rays;
}