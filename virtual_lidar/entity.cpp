#include "entity.hpp"

#include "units.hpp"

// STATIC ENTITY
std::shared_ptr<IEntity> StaticEntity::clone() const {
    return std::make_shared<StaticEntity>(*this);
}

StaticEntity::StaticEntity(std::string name, std::shared_ptr<Object> obj, Pose p)
    : m_name(name), m_object(obj), m_pose(p) {}

Transform3 StaticEntity::transform() const { return m_pose.transform(); }

// LIDAR ENTITY
std::shared_ptr<IEntity> LidarEntity::clone() const {
    return std::make_shared<LidarEntity>(*this);
}

LidarEntity::LidarEntity(std::shared_ptr<Lidar> model, unsigned int step_index, double fov_h, Pose p)
    : m_model(model), m_step_index(step_index), m_fov_h(to_radians(fov_h)), m_pose(p), m_noise_model(new OusterOS2Noise){}

Transform3 LidarEntity::transform() const { return m_pose.transform(); }

const Lidar& LidarEntity::config() const { return *m_model; }

const double& LidarEntity::h_step() const { return m_model->m_h_step.at(m_step_index); }

const double& LidarEntity::fov_h() const { return m_fov_h; }

std::vector<Ray3> LidarEntity::scan(double h_rad) const {
    Transform3 world_xf = transform();
    std::vector<Ray3> rays;

    for(const auto& laser : m_model->m_lasers) {
        double h = h_rad + laser.h_off;

        // Conversion coordonnées sphériques en cartésiennes
        Vector3 dir(std::cos(laser.v_rad) * std::cos(h),
                    std::cos(laser.v_rad) * std::sin(h),
                    std::sin(laser.v_rad));

        rays.push_back({m_pose.pos(), world_xf.transform(dir)});
    }

    return rays;
};
