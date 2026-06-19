#include "scene.hpp"
#include "logger.hpp"
#include "units.hpp"

#include <iterator>

Scene::Scene(const Scene& scene){
    for(const auto& entity : scene.m_entities){
        if(entity){
            m_entities.push_back(std::make_unique<StaticEntity>(*entity));
        }
    }
            SIM_DEBUG("Scene Copiée ! {} entités", m_entities.size());
}


void Scene::add_static_entity(std::unique_ptr<StaticEntity> ent) { 
    m_entities.push_back(std::move(ent));
}

void Scene::build(){

    m_tlas_primitives.clear();
    m_tlas_primitives.reserve(m_entities.size());

    for(size_t i = 0; i < m_entities.size(); i++){
        CGAL::Bbox_3 bbox = m_entities[i]->world_bbox();
        m_tlas_primitives.emplace_back(i, bbox);
    }

    m_tlas_tree = std::make_unique<TlasTree>(m_tlas_primitives.begin(), m_tlas_primitives.end());
    SIM_DEBUG("TLAS build : {} entités", m_entities.size());
}

std::optional<Intersection> Scene::intersect(const Ray3& ray) const {
    if(!m_tlas_tree || m_tlas_tree->empty()) return std::nullopt;

    std::optional<Intersection> best_hit = std::nullopt;
    double min_dist = std::numeric_limits<double>::max();

    std::vector<size_t> hit_ids;

    // Jpp savoir avec uniquement avec la première bbox trouvée malheureusement !
    m_tlas_tree->all_intersected_primitives(ray, std::back_inserter(hit_ids));

    for(size_t entity_id : hit_ids){
        auto hit = m_entities[entity_id]->intersect(ray);
        if(hit && hit->distance < min_dist){
            min_dist = hit->distance;
            best_hit = hit;
        }
    }

    return best_hit;
}