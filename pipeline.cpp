#include "pipeline.h"
#include "pose.h"
#include "logger.h"

std::vector<std::unique_ptr<Scene>> RotationLayoutAugmentation::process(std::vector<std::unique_ptr<Scene>> input_scenes, AssetManager& assets)
{
    (void)assets; // à enlever
    std::vector<std::unique_ptr<Scene>> output_scenes;

    SIM_INFO("Lancement de la génération des variations de scènes en fonction de rotations aléatoires sur [{}, {}]", m_start_angle, m_end_angle);
    SIM_INFO("Traitement de {} scènes sources ", input_scenes.size());

    std::uniform_real_distribution<double> rot_z(m_start_angle, m_end_angle);


    size_t scene_index = 0;
    for(const auto& base_scene : input_scenes){
        for(size_t i = 0; i < m_number_of_variations; i++){
            SIM_DEBUG("Génération d'une variation spatiale pour la scène n° {} :", scene_index++);
            
            auto current_spatial_variation = std::make_unique<Scene>(*base_scene);

            size_t ent_index = 0;
            for(const auto& entity : current_spatial_variation->entities()){
                if(std::dynamic_pointer_cast<StaticEntity>(entity)){
                    double r_z = rot_z(m_gen);

                    Pose random_pose({entity->pose().pos()}, 0.0, 0.0, r_z);
                    entity->pose(random_pose);
                    SIM_DEBUG("Entité copié : {} et oritentée en Z: {:.2f}", entity->name(), random_pose.rz());
                    ent_index++;
                
                }

            }
        
            SIM_INFO("Reconstruction de l'arbre pour la variation {}", ent_index);
            output_scenes.push_back(std::move(current_spatial_variation));
        }
    }
    return output_scenes;
}

std::vector<std::unique_ptr<Scene>> PositionLayoutAugmentation::process(std::vector<std::unique_ptr<Scene>> input_scenes, AssetManager& assets)
{
    (void)assets; // à enlever
    std::vector<std::unique_ptr<Scene>> output_scenes;

    SIM_INFO("Lancement de la génération des variations de scènes en fonction de positions aléatoires");
    SIM_INFO("Traitement de {} scènes sources ", input_scenes.size());

    std::uniform_real_distribution<double> dist_x(-20.0, 20.0);
    std::uniform_real_distribution<double> dist_y(-20.0, 20.0);


    size_t scene_index = 0;
    for(const auto& base_scene : input_scenes){
        for(size_t i = 0; i < m_number_of_variations; i++){
            SIM_DEBUG("Génération d'une variation spatiale pour la scène n° {} :", scene_index++);
            
            auto current_spatial_variation = std::make_unique<Scene>(*base_scene);

            size_t ent_index = 0;
            for(const auto& entity : current_spatial_variation->entities()){
                if(std::dynamic_pointer_cast<StaticEntity>(entity)){
                    double x = dist_x(m_gen);
                    double y = dist_y(m_gen);

                    bool is_too_close = true;

                    while(is_too_close){
                        x = x < 5 && x > -5 ? dist_x(m_gen) : x;
                        y = y < 5 && y > -5 ? dist_y(m_gen) : y;
                        
                        if(!(x < 5 && x > -5) || !(y < 5 && y > -5))is_too_close = false;
                    }

                    Pose random_pose({x, y, 0.0});
                    entity->pose(random_pose);
                    SIM_DEBUG("Entité copié : {} et positionnnée en X: {:.2f}, Y: {:.2f}", entity->name(), random_pose.pos().x(), random_pose.pos().y());
                    ent_index++;
                }

            }
        
            SIM_INFO("Reconstruction de l'arbre pour la variation {}", ent_index);
            output_scenes.push_back(std::move(current_spatial_variation));
        }
    }
    return output_scenes;
}

std::unique_ptr<Scene> KeyframeLayoutAugmentation::apply_keyframe(const Scene& input_scene,const std::string& entity_name, const std::string& keyframe_path, AssetManager& assets){
    auto new_scene = std::make_unique<Scene>(input_scene);

    if(keyframe_path.empty()) {
        SIM_WARNING("Path de la keyframe vide donnée  pour {}!", entity_name);
        return new_scene; // Pour l'instant on renvoie une copie de la scène
    }

    for(auto& ent : new_scene->entities()){
        if(ent->name() == entity_name){
            if(auto static_ent = std::dynamic_pointer_cast<StaticEntity>(ent)){
                auto mesh = assets.get_mesh(keyframe_path);
                if(mesh){
                    static_ent->update_mesh(mesh);
                }else{
                    // Pour l'instant on génère la scène quand meme si cela fait une scene doublon
                    SIM_WARNING("Fichier de la keyframe : {} vide pour {}", keyframe_path, entity_name);
                }
            }
            break;
        }
    }
    return new_scene;
}

std::vector<std::unique_ptr<Scene>> KeyframeLayoutAugmentation::process(std::vector<std::unique_ptr<Scene>> input_scenes, AssetManager& assets){
    std::vector<std::unique_ptr<Scene>> output_scenes;

    SIM_INFO("Lancement de la génération des variations de scènes en fonction des keyframes des entitees");
    SIM_INFO("Traitement de {} scènes sources ", input_scenes.size());

    for(const auto& base_scene : input_scenes){
        for(const auto& config : m_keyframes_configs){
            SIM_INFO("Generation des Scenes pour le scenario : {}", config.scenario_name);
            auto combinations = generate_combinations(*base_scene, config, assets);
            for(auto& new_scene : combinations){
                output_scenes.push_back(std::move(new_scene));
            }
        }
    }

    SIM_INFO("{} scènes générées ", output_scenes.size());
    return output_scenes;
}

std::vector<std::unique_ptr<Scene>> KeyframeLayoutAugmentation::generate_combinations(const Scene& input_scene, const KeyframesConfig& config, AssetManager& assets){
    std::vector<std::unique_ptr<Scene>> output_scenes;
    output_scenes.push_back(std::make_unique<Scene>(input_scene));

    // parcours en largeur
    // On génère toutes les configs de scènes uniques 
    for(const auto& target : config.objects_animation){
        
        if(target.keyframes_paths.empty()) continue;


        std::vector<std::unique_ptr<Scene>> new_combinations;

        SIM_INFO("Generation de Scenes pour l'animation de l'objet : {} ", target.object_name);

        for(const auto& current_scene : output_scenes){
            for(const std::string& keyframe_path : target.keyframes_paths){
                
                // generation nouvelle variante de scene
                auto new_scene = apply_keyframe(*current_scene, target.object_name, keyframe_path, assets);
                new_combinations.push_back(std::move(new_scene));
            }
        }

        // les nouvelles scènes générées sont ajoutées 
        output_scenes = std::move(new_combinations);
    }

    return output_scenes;
}

void Pipeline::add_step(std::unique_ptr<PipelineStep> step){
        m_steps.push_back(std::move(step));
    }

std::vector<std::unique_ptr<Scene>> Pipeline::execute(std::unique_ptr<Scene> input_scene, AssetManager& assets){
    SIM_INFO("Démarrage du pieline de génération de scènes !");

    //"transfert de propriété"
    std::vector<std::unique_ptr<Scene>> output_scenes;
    output_scenes.push_back(std::move(input_scene));
        
    for(auto& step : m_steps){
        output_scenes = step->process(std::move(output_scenes), assets);
    }
        
    return output_scenes;
}