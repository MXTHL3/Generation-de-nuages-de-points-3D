#include "entity.hpp"

#include "units.hpp"
#include "logger.hpp"

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

std::vector<Ray3> MechanicalLidarEntity::generate_rays(double duration) const{
    std::vector<Ray3> all_rays;
    double total_angle = 360.0;

    if(duration > 0.0){
        total_angle = duration * config().m_rotation_rate * 360.0;
        SIM_DEBUG("Angle total : {}", total_angle);
    }

    double h_step = config().horizontal_step();

    for(double hr = 0.0; hr < total_angle; hr += h_step){
        auto rays = scan(hr);
        all_rays.insert(all_rays.end(), rays.begin(), rays.end());
    }

    return all_rays;
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

std::vector<Ray3> generate_ray_grid(
    const Point3& origin,
    const Transform3& world_xf,
    double fov_h_min, double fov_h_max,
    double fov_v_min, double fov_v_max,
    int res_h, int res_v)
    {
        std::vector<Ray3> rays;

        Vector3 translation_effect = world_xf.transform(Vector3(0, 0, 0));

        double h_step = (fov_h_max - fov_h_min) / static_cast<double>(res_h);
        double v_step = (fov_v_max - fov_v_min) / static_cast<double>(res_v);

        for(int v = 0; v < res_v; v++){
            double v_angle = fov_v_min + v * v_step;
            for(int h = 0; h < res_h; h++){
                double h_angle = fov_h_min + h * h_step;

                Vector3 dir(std::cos(v_angle) * std::cos(h_angle),
                    std::cos(v_angle) * std::sin(h_angle),
                    std::sin(v_angle));
            
                Vector3 world_dir = world_xf.transform(dir) - translation_effect;
                rays.push_back({origin, world_dir});
            }
        }

        return rays;
    }

FlashLidarEntity::FlashLidarEntity(std::shared_ptr<FlashLidarConfig> config, Pose p)
    : LidarEntity(config, p) {}

std::unique_ptr<IEntity> FlashLidarEntity::clone() const {
    return std::make_unique<FlashLidarEntity>(*this);
}

std::vector<Ray3> FlashLidarEntity::generate_rays(double duration) const {
    const auto& l_config = config();
    return generate_ray_grid(m_pose.pos(), transform(),
    l_config.m_fov_h_min, l_config.m_fov_h_max,
    l_config.m_fov_v_min, l_config.m_fov_v_max,
    l_config.m_resolution_h, l_config.m_resolution_v);
}

MirroredLidarEntity::MirroredLidarEntity(std::shared_ptr<MirroredLidarConfig> config, Pose p)
    : LidarEntity(config, p){}

std::unique_ptr<IEntity> MirroredLidarEntity::clone() const{
    return std::make_unique<MirroredLidarEntity>(*this);
}

std::vector<Ray3> MirroredLidarEntity::generate_rays(double duration) const {
    if(config().is_lissajou()){
        return generate_lissajou(duration);
    }
    if(config().is_raster()){
        return generate_raster();
    }
    SIM_ERROR("Erreur lors de la generation des rayons du mirrored lidar {} : mode de scan inconnu !", m_config->m_name);
    throw std::runtime_error("Mode de scan inconnu pour " + m_config->m_name);
}

std::vector<Ray3> MirroredLidarEntity::generate_lissajou(double duration) const {
    Transform3 world_xf = transform();
    std::vector<Ray3> rays;
    const auto& cfg = config();
    const auto& lissajou_params = std::get<LissajouParams>(cfg.m_mirrored_scan_params);

    Vector3 translation_effect = world_xf.transform(Vector3(0, 0, 0));

    double time = (duration > 0.0) ? duration : cfg.m_integration_time;

    int n_points = static_cast<int>(cfg.m_points_per_second * time);
    double dt = time / static_cast<double>(n_points);

    for(int i = 0; i < n_points; i++){
        double t = i * dt;

        double h_angle = lissajou_params.m_amplitude_h * std::sin(2.0 * M_PI * lissajou_params.m_freq_h * t);
        double v_angle = lissajou_params.m_amplitude_v * std::sin(2.0 * M_PI * lissajou_params.m_freq_v * t + lissajou_params.m_phase_diff);

        if(h_angle < cfg.m_fov_h_min || h_angle > cfg.m_fov_h_max) continue; 
        if(v_angle < cfg.m_fov_v_min || v_angle > cfg.m_fov_v_max) continue;
        
        Vector3 dir(std::cos(v_angle) * std::cos(h_angle),
                std::cos(v_angle) * std::sin(h_angle),
                std::sin(v_angle));
            
        Vector3 world_dir = world_xf.transform(dir) - translation_effect;
        rays.push_back({m_pose.pos(), world_dir});
    }

    return rays;
}

std::vector<Ray3> MirroredLidarEntity::generate_raster() const {
    const auto& cfg = config();
    const auto& raster_cfg = std::get<RasterParams>(cfg.m_mirrored_scan_params);
    return generate_ray_grid(m_pose.pos(), transform(),
    cfg.m_fov_h_min, cfg.m_fov_h_max, 
    cfg.m_fov_v_min, cfg.m_fov_v_max, 
    raster_cfg.resolution_h, raster_cfg.resolution_v);
}