#include "pipeline.hpp"
#include "pose.hpp"
#include "logger.hpp"

std::vector<std::shared_ptr<Scene>> SpatialLayoutAugmentation::process(const std::vector<std::shared_ptr<Scene>>& input_scenes, AssetManager& assets){
    std::vector<std::shared_ptr<Scene>> output_scenes;

    // en dur pour l'instant à voir avec distance max lidar ?

    SIM_INFO("Lancement de la génération des variations de scènes en fonction des positions aléatoires");
    SIM_INFO("Traitement de {} scènes sources ", input_scenes.size());

    std::uniform_real_distribution<double> dist_x(-20.0, 20.0);
    std::uniform_real_distribution<double> dist_y(-20.0, 20.0);
    std::uniform_real_distribution<double> rot_z(0.0, 360.0);

    size_t scene_index = 0;

    for(const auto& base_scene : input_scenes){
        for(size_t i = 0; i < m_number_of_variations; i++){
            SIM_DEBUG("Génération d'une variation spatiale pour la scène n° {} :", scene_index++);
            
            auto current_spatial_variation = std::make_shared<Scene>(*base_scene);

            size_t ent_index = 0;
            for(const auto& entity : current_spatial_variation->entities()){
                if(std::dynamic_pointer_cast<StaticEntity>(entity)){
                    double x = dist_x(m_gen);
                    double y = dist_y(m_gen);
                    double r_z = rot_z(m_gen);

                    bool is_too_close = true;

                    while(is_too_close){
                        x = x < 5 && x > -5 ? dist_x(m_gen) : x;
                        y = y < 5 && y > -5 ? dist_y(m_gen) : y;
                        
                        if(!(x < 5 && x > -5) || !(y < 5 && y > -5))is_too_close = false;
                    }

                    Pose random_pose({x, y, 0.0}, 0.0, 0.0, r_z);
                    SIM_DEBUG("Entité copié n° {} et positionnnée en X: {:.2f}, Y: {:.2f}, Rotation Z: {:.2f}", ent_index, x, y, r_z);
                    
                    entity->pose(random_pose);
                }

            }
        
            // TODO :: à voir la responsabilité est plus au Pipeline ~~ BuildAll()
            SIM_INFO("Reconstruction de l'arbre pour la variation {}", ent_index);
            current_spatial_variation->build();
            output_scenes.push_back(current_spatial_variation);
        }
    }

    SIM_INFO("Génération des variations de scenes terminées : {} scènes générées !", output_scenes.size());
    return output_scenes;
}

void Pipeline::add_step(std::shared_ptr<PipelineStep> step){
        m_steps.push_back(step);
    }

std::vector<std::shared_ptr<Scene>> Pipeline::execute(std::shared_ptr<Scene> input_scene, AssetManager& assets){
    SIM_INFO("Démarrage du pieline de génération de scènes !");

    std::vector<std::shared_ptr<Scene>> output_scenes = {std::move(input_scene)};
        
    for(auto& step : m_steps){
        output_scenes = step->process(output_scenes, assets);
    }
        
    return output_scenes;
}