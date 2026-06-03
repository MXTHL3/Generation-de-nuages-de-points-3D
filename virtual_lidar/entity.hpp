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
    virtual ~IEntity() {};
    virtual std::shared_ptr<IEntity> clone() const = 0;
    virtual Transform3 transform() const = 0;    // Récupère la position dans la scène
    virtual const std::string& name() const = 0;
    virtual const Pose& pose() const = 0;
    virtual void pose(const Pose& pose) = 0;      
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
    std::shared_ptr<IEntity> clone() const override; 
    const std::string& name() const override {return m_name;};
    Transform3 transform() const override;

    const Pose& pose() const override{ return m_pose; }
    void pose(const Pose& pose) override{ m_pose = pose; }

    const std::vector<Triangle3>& meshTriangles() const { return m_object->m_triangles; }
    
    void update_mesh(std::shared_ptr<Object> new_obj) { m_object = new_obj; }

private:
    std::string m_name;
    std::shared_ptr<Object> m_object; // Ref du Mesh
    Pose m_pose;                      // Position du Mesh
};

// Lidar dans la scène
class LidarEntity : public IEntity {
public:
    LidarEntity(std::shared_ptr<LidarConfig> config, Pose p);
    virtual ~LidarEntity() = default;
    
    const std::string& name() const override {return m_config->m_name; };
        Transform3 transform() const override;

    std::shared_ptr<LidarConfig> config() const { return m_config; }

        void pose(const Pose& pose) override { m_pose = pose; };
        const Pose& pose() const override{ return m_pose; }

        // génère les rayons à lancer
    virtual std::vector<Ray3> scan(double parameter) const = 0;

        void noise_model(std::shared_ptr<NoiseModel> model) {m_noise_model = model; }
        double noisy_distance(double d) const {
            return m_noise_model ? m_noise_model->apply(d) : d;
        }

protected:
    std::shared_ptr<LidarConfig> m_config;
    Pose m_pose;                    // Position du Lidar
    std::shared_ptr<NoiseModel> m_noise_model = std::make_shared<OusterOS2Noise>(); //Modele de génération de bruit à voir pour le placer directement dans le JSON
};

class MechanicalLidarEntity : public LidarEntity {
private:
    std::shared_ptr<MechanicalLidarConfig> m_config;

public:
    MechanicalLidarEntity(std::shared_ptr<MechanicalLidarConfig> config, Pose p);

    std::shared_ptr<IEntity> clone() const override;

    std::vector<Ray3> scan(double parameter) const override;
};


#endif