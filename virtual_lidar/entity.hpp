#ifndef ENTITY_HPP
#define ENTITY_HPP

#include "pose.hpp"
#include "lidar.hpp"
#include "noise_model.hpp"

#include <memory>
#include <vector>

typedef K::Ray_3 Ray3;
typedef K::Triangle_3 Triangle3;

// Objet présent dans la scène
class IEntity {
public:
    IEntity(Pose pose): m_pose(pose){};
    virtual ~IEntity(){};
    virtual std::unique_ptr<IEntity> clone() const = 0;
    Transform3 transform() const;    // Récupère la position dans la scene
    virtual const std::string& name() const = 0;
    const Pose& pose() const { return m_pose; }
    void pose(const Pose& pose) { m_pose = pose; }


protected:
    Pose m_pose;
};

// Contient le mesh
class Object {
public:
    std::vector<Triangle3> m_triangles; // mesh sous forme de triangles
};

// Objet statique dans la scène à voir si l'on doit avoir des objets en mouvement
class StaticEntity : public IEntity {
public:
    StaticEntity(std::string name, std::shared_ptr<Object> obj, Pose p);
    std::unique_ptr<IEntity> clone() const override; 
    const std::string& name() const override final {return m_name;};

    const std::vector<Triangle3>& meshTriangles() const { return m_object->m_triangles; }
    
    void update_mesh(std::shared_ptr<Object> new_obj) { m_object = new_obj; }

private:
    std::string m_name;
    std::shared_ptr<Object> m_object; // Ref du Mesh
};

// Lidar dans la scène
class LidarEntity : public IEntity {
public:
    LidarEntity(std::shared_ptr<LidarConfig> config, Pose p);
    virtual ~LidarEntity() = default;
    
    const std::string& name() const override final { return m_config->m_name; };

    const LidarConfig& config() const { return *m_config; };

        // génère les rayons à lancer
    virtual std::vector<Ray3> generate_rays() const = 0;

    void noise_model(const NoiseModel& model) {m_noise_model = model; }

    double noisy_distance(double d) const{
        return m_noise_model.apply(d);
    }

protected:
    std::shared_ptr<LidarConfig> m_config;
    NoiseModel m_noise_model;
};

class MechanicalLidarEntity : public LidarEntity {
public:
    MechanicalLidarEntity(std::shared_ptr<MechanicalLidarConfig> config, Pose p);

    const MechanicalLidarConfig& config() const{ return static_cast<const MechanicalLidarConfig&>(*m_config); }
    std::unique_ptr<IEntity> clone() const override;

    std::vector<Ray3> scan(double parameter) const;
    std::vector<Ray3> generate_rays() const override;
};

std::vector<Ray3> generate_ray_grid(
    const Point3& origin,
    const Transform3& wolrd_xf,
    double fov_h, double fov_v,
    int res_h, int res_v
);

class FlashLidarEntity : public LidarEntity {
public:
    FlashLidarEntity(std::shared_ptr<FlashLidarConfig> config, Pose p);

    const FlashLidarConfig& config() const{ return static_cast<const FlashLidarConfig&>(*m_config); };

    std::unique_ptr<IEntity> clone() const override;

    std::vector<Ray3> generate_rays() const override;
};

class MirroredLidarEntity : public LidarEntity {
public:
    MirroredLidarEntity(std::shared_ptr<MirroredLidarConfig> config, Pose p);

    const MirroredLidarConfig& config() const{ return static_cast<const MirroredLidarConfig&>(*m_config); };

    std::unique_ptr<IEntity> clone() const override;
    
    std::vector<Ray3> generate_rays() const override;

private:
    std::vector<Ray3> generate_lissajou() const;
    std::vector<Ray3> generate_raster() const;
};


#endif