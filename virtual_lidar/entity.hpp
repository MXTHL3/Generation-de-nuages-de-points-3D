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
    StaticEntity(std::shared_ptr<Object> obj, Pose p);
    std::shared_ptr<IEntity> clone() const override; 
    
    Transform3 transform() const override;
    const Pose& pose() const override{ return m_pose; }
    void pose(const Pose& pose) override{ m_pose = pose; }

    const std::vector<Triangle3>& meshTriangles() const { return m_object->m_triangles; }
    
    void update_mesh(std::shared_ptr<Object> new_obj) { m_object = new_obj; }

private:
    std::shared_ptr<Object> m_object; // Ref du Mesh
    Pose m_pose;                      // Position du Mesh
};

// Lidar dans la scène
class LidarEntity : public IEntity {
public:
        LidarEntity(std::shared_ptr<Lidar> model, unsigned int step_index, double m_fov_h, Pose p);
        std::shared_ptr<IEntity> clone()const override;
        Transform3 transform() const override;
        void pose(const Pose& pose) override { m_pose = pose; };
        const Pose& pose() const override{ return m_pose; }
        unsigned int step_index() const { return m_step_index;}

        // génère les rayons à lancer
        std::vector<Ray3> scan(double h_deg) const;
        const Lidar& config() const;
        const double& h_step() const;
        const double& fov_h() const;

        void noise_model(std::shared_ptr<NoiseModel> model) {m_noise_model = model; }
        double noisy_distance(double d) const {
            return m_noise_model ? m_noise_model->apply(d) : d;
        }

private:
    std::shared_ptr<Lidar> m_model; // Ref du Lidar
    unsigned int m_step_index;      // Indice du step horizontal
    double m_fov_h;
    Pose m_pose;                    // Position du Lidar
    std::shared_ptr<NoiseModel> m_noise_model = std::make_shared<OusterOS2Noise>(); //Modele de génération de bruit à voir pour le placer directement dans le JSON
};

#endif