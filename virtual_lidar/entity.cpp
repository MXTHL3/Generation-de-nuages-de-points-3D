#include "entity.hpp"

#include "units.hpp"

// STATIC ENTITY

std::unique_ptr<IEntity> StaticEntity::clone() const {
    return std::make_unique<StaticEntity>(*this);
}
Transform3 IEntity::transform() const { return m_pose.transform(); }

StaticEntity::StaticEntity(std::string name, std::shared_ptr<Object> obj, Pose p)
    : IEntity(p), m_name(name), m_object(obj) {}

LidarEntity::LidarEntity(std::shared_ptr<LidarConfig> config, Pose p)
    : IEntity(p), m_config(config), m_noise_model(config->m_noise_profile){}

MechanicalLidarEntity::MechanicalLidarEntity(std::shared_ptr<MechanicalLidarConfig> config, Pose p)
    : LidarEntity(config, p){}


std::unique_ptr<IEntity> MechanicalLidarEntity::clone() const {
    return std::make_unique<MechanicalLidarEntity>(*this);
}

std::vector<Ray3> MechanicalLidarEntity::scan(double parameter) const {
    Transform3 world_xf = transform();
    std::vector<Ray3> rays;

    double h_rad = to_radians(parameter);

    for(const auto& laser : config().m_lasers) {
        double h = h_rad + laser.h_off;
        // Conversion coordonnées sphériques en cartésiennes
        Vector3 dir(std::cos(laser.v_rad) * std::cos(h),
                    std::cos(laser.v_rad) * std::sin(h),
                    std::sin(laser.v_rad));

        Vector3 world_dir = world_xf.transform(dir) - world_xf.transform(Vector3(0, 0, 0));
        rays.push_back({m_pose.pos(), world_dir});
    }

    return rays;
}