#pragma once

#include "pose.h"
#include "lidar.h"
#include <memory>
#include <vector>

typedef K::Ray_3 Ray3;
typedef K::Triangle_3 Triangle3;

// Objet présent dans la scène
class IEntity {
public:
    virtual ~IEntity() {};
    virtual Transform3 getTransform() const = 0;    // Récupère la position dans la scène
    virtual void update(double dt) = 0;             // Update de la position         
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
    Transform3 getTransform() const override;
    void update(double) override;

    std::shared_ptr<Object> getMesh() const { return m_object; }
private:
    std::shared_ptr<Object> m_object; // Ref du Mesh
    Pose m_pose;                      // Position du Mesh
};

// Lidar dans la scène
class LidarEntity : public IEntity {
public:
        LidarEntity(std::shared_ptr<Lidar> model, unsigned int step_index, Pose p);
        Transform3 getTransform() const override;
        void update(double dt) override;

        // génère les rayons à lancer
        std::vector<Ray3> scan(double h_deg) const;
        const Lidar& config() const;
        const double& h_step() const;

private:
    std::shared_ptr<Lidar> m_model; // Ref du Lidar
    unsigned int m_step_index;      // Indice du step horizontal
    Pose m_pose;                    // Position du Lidar
};