#pragma once

#include "pose.h"
#include "lidar.h"
#include "noise_model.h"
#include <memory>
#include <vector>
#include <string>

typedef K::Ray_3 Ray3;
typedef K::Triangle_3 Triangle3;

// Objet présent dans la scène
class IEntity {
public:
    virtual ~IEntity() {};
    virtual std::shared_ptr<IEntity> clone() const = 0;
    virtual Transform3 getTransform() const = 0;    // Récupère la position dans la scène
    virtual void update(double dt) = 0;             // Update de la position
    virtual const std::string& name() const = 0;
    virtual const Pose& pose() const = 0;
    virtual void pose(const Pose& p) = 0;         
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
    Transform3 getTransform() const override;
    void update(double) override {}

    const std::string& name() const override { return m_name; }
    const Pose& pose() const override { return m_pose; }
    void pose(const Pose& p) override { m_pose = p; }

    std::shared_ptr<Object> getMesh() const { return m_object; }
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
    LidarEntity(std::shared_ptr<Lidar> model, unsigned int step_index,
                double fov_h, Pose p,
                std::unique_ptr<NoiseModel> noise = std::make_unique<NullNoiseModel>());

    std::shared_ptr<IEntity> clone() const override;
    Transform3 getTransform() const override;
    void update(double) override {}

    const std::string& name() const override { return m_name; }
    const Pose& pose() const override { return m_pose; }
    void pose(const Pose& p) override { m_pose = p; }

    // génère les rayons à lancer
    std::vector<Ray3> scan(double h_deg) const;
    const Lidar& config() const;
    const double& h_step() const;
    const double& fov_h() const { return m_fov_h; }
    const unsigned int& step_index() const { return m_step_index; }

private:
    std::string m_name = "lidar";
    std::shared_ptr<Lidar> m_model; // Ref du Lidar
    unsigned int m_step_index;      // Indice du step horizontal
    double m_fov_h;
    Pose m_pose;                    // Position du Lidar
    std::unique_ptr<NoiseModel> m_noise_model;

    friend class Scene;
};