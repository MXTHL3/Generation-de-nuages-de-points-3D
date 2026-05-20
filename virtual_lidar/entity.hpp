#ifndef ENTITY_HPP
#define ENTITY_HPP

#include "pose.hpp"
#include "lidar.hpp"
#include <memory>
#include <vector>

typedef K::Ray_3 Ray3;
typedef K::Triangle_3 Triangle3;

// Objet présent dans la scène
class IEntity {
public:
    virtual ~IEntity() {};
    virtual Transform3 transform() const = 0;    // Récupère la position dans la scène
    virtual const Pose& pose() const = 0;      
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
    Transform3 transform() const override;
    const Pose& pose() const override{ return m_pose; }

    const std::vector<Triangle3>& meshTriangles() const { return m_object->m_triangles; }

private:
    std::shared_ptr<Object> m_object; // Ref du Mesh
    Pose m_pose;                      // Position du Mesh
};

// Lidar dans la scène
class LidarEntity : public IEntity {
public:
        LidarEntity(std::shared_ptr<Lidar> model, unsigned int step_index, double m_fov_h, Pose p);
        Transform3 transform() const override;
        const Pose& pose() const override{ return m_pose; }
        unsigned int step_index() const { return m_step_index;}

        // génère les rayons à lancer
        std::vector<Ray3> scan(double h_deg) const;
        const Lidar& config() const;
        const double& h_step() const;
        const double& fov_h() const;

private:
    std::shared_ptr<Lidar> m_model; // Ref du Lidar
    unsigned int m_step_index;      // Indice du step horizontal
    double m_fov_h;
    Pose m_pose;                    // Position du Lidar
};

#endif