#include "entity.hpp"

#include "units.hpp"

// STATIC ENTITY
std::unique_ptr<IEntity> StaticEntity::clone() const {
    return std::make_unique<StaticEntity>(*this);
}

StaticEntity::StaticEntity(std::string name, std::shared_ptr<Object> obj, Pose p)
    : m_name(name), m_object(obj), m_pose(p) {}

Transform3 StaticEntity::transform() const { return m_pose.transform(); }

Transform3 LidarEntity::transform() const { return m_pose.transform(); }

LidarEntity::LidarEntity(std::shared_ptr<LidarConfig> config, Pose p)
    : m_config(config), m_pose(p) {}

MechanicalLidarEntity::MechanicalLidarEntity(std::shared_ptr<MechanicalLidarConfig> config, Pose p)
    : LidarEntity(config, p), m_config(config){}


std::unique_ptr<IEntity> MechanicalLidarEntity::clone() const {
    return std::make_unique<MechanicalLidarEntity>(*this);
}

std::vector<Ray3> MechanicalLidarEntity::scan(double parameter) const {
    Transform3 world_xf = transform();
    std::vector<Ray3> rays;

    double h_rad = to_radians(parameter);

    for(const auto& laser : m_config->m_lasers) {
        double h = h_rad + laser.h_off;
        // Conversion coordonnées sphériques en cartésiennes
        Vector3 dir(std::cos(laser.v_rad) * std::cos(h),
                    std::cos(laser.v_rad) * std::sin(h),
                    std::sin(laser.v_rad));

        rays.push_back({m_pose.pos(), world_xf.transform(dir)});
    }

    return rays;
}