#include "pipeline.h"
#include "units.h"
#include "pose.h"
#include "logger.h"

std::vector<std::unique_ptr<Scene>> RotationLayoutAugmentation::process(std::vector<std::unique_ptr<Scene>> input_scenes, AssetManager& assets){
    (void)assets;
    std::vector<std::unique_ptr<Scene>> output_scenes;

    SIM_INFO("Rotation uniforme : {} variations", m_number_of_variations);

    for(const auto& base_scene : input_scenes){
        SIM_INFO("Génération de variations de rotation sur la scène {}", "nom des scènes à def");
        for(size_t i = 0; i < m_number_of_variations; i++){
            double r_x = rand() % 361;
            double r_y = rand() % 361;
            double r_z = rand() % 361;
            auto current_variation = std::make_unique<Scene>(*base_scene);

            for(const auto& entity : current_variation->entities()){
                if(entity->name() == m_object_name){
                    Pose rotated_pose(entity->pose().pos(), r_x, r_y, r_z);
                    entity->pose(rotated_pose);
                }
            }

            output_scenes.push_back(std::move(current_variation));
        }
    }

    SIM_INFO("{} scènes générées", output_scenes.size());
    return output_scenes;
}

std::vector<std::unique_ptr<Scene>> PositionLayoutAugmentation::process(std::vector<std::unique_ptr<Scene>> input_scenes, AssetManager& assets)
{
    (void)assets;
    std::vector<std::unique_ptr<Scene>> output_scenes;

    SIM_INFO("Lancement de la génération des variations de scènes en fonction de positions aléatoires");
    SIM_INFO("Traitement de {} scènes sources ", input_scenes.size());

    std::uniform_real_distribution<double> dist_x(-20.0, 20.0);
    std::uniform_real_distribution<double> dist_y(-20.0, 20.0);
    std::uniform_real_distribution<double> dist_z(-20.0, 20.0);

    size_t scene_index = 0;
    for(const auto& base_scene : input_scenes){
        for(size_t i = 0; i < m_number_of_variations; i++){
            SIM_DEBUG("Génération d'une variation spatiale pour la scène n° {} :", scene_index++);
            
            auto current_spatial_variation = std::make_unique<Scene>(*base_scene);

            size_t ent_index = 0;
            for(const auto& entity : current_spatial_variation->entities()){
                double x = dist_x(m_gen);
                double y = dist_y(m_gen);
                double z = dist_y(m_gen);

                bool is_too_close = true;

                while(is_too_close){
                    x = x < 5 && x > -5 ? dist_x(m_gen) : x;
                    y = y < 5 && y > -5 ? dist_y(m_gen) : y;
                    z = z < 5 && z > -5 ? dist_z(m_gen) : z;
                        
                    if(!(x < 5 && x > -5) && !(y < 5 && y > -5) && !(z < 5 && z > -5)) is_too_close = false;
                }

                Pose random_pose({x, y, z}, 
                    to_degrees(entity->pose().rx()),
                    to_degrees(entity->pose().ry()),
                    to_degrees(entity->pose().rz())
                );
                entity->pose(random_pose);
                SIM_DEBUG("Entité copié : {} et positionnnée en X: {:.2f}, Y: {:.3f}, Z: {:.4f}", entity->name(), random_pose.pos().x(), random_pose.pos().y(), random_pose.pos().z());
                ent_index++;
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
            auto mesh = assets.get_mesh(keyframe_path);
            if(mesh){
                ent->update_mesh(mesh);
            }else{
                // Pour l'instant on génère la scène quand meme si cela fait une scene doublon
                SIM_WARNING("Fichier de la keyframe : {} vide pour {}", keyframe_path, entity_name);
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
    SIM_INFO("Démarrage du pipeline de génération de scènes !");

    //"transfert de propriété"
    std::vector<std::unique_ptr<Scene>> output_scenes;
    output_scenes.push_back(std::move(input_scene));
        
    for(auto& step : m_steps){
        output_scenes = step->process(std::move(output_scenes), assets);
    }
        
    return output_scenes;
}

Pipeline PipelineFactory::create_from_json(const nlohmann::json& data){
    SIM_INFO("Chargement d'un Pipeline");
    
    Pipeline pipeline;

    if(!data.contains("pipeline")){
        SIM_WARNING("Pas de pipeline défini dans le json !");
        return pipeline;
    }

    for(const auto& step_json : data["pipeline"]){
        auto step = parse_step(step_json);
        if(step){
            pipeline.add_step(std::move(step));
        }else{
            SIM_WARNING("step de pipeline trouvée mais illisible ! : {}", step_json.dump(4));
        }
    }

    SIM_INFO("Pipeline chargé : {} étapes", data["pipeline"].size());
    return pipeline;
}

std::unique_ptr<PipelineStep> PipelineFactory::parse_step(const nlohmann::json& step){
    std::string type = step.at("type").get<std::string>();

    if(type == "rotation")          return parse_rotation(step);
    if(type == "grid_position")     return parse_grid_position(step);
    if(type == "keyframe")          return parse_keyframe(step); 
    if(type == "linear_position")   return parse_linear_position(step);

    SIM_ERROR("Type d'étape de pipeline inconnu ! {}", type);
    return nullptr;
}

std::unique_ptr<PipelineStep> PipelineFactory::parse_rotation(const nlohmann::json& rotation_step){
    size_t n = rotation_step.at("n_variations").get<size_t>();
    std::string object_name = rotation_step.at("entity_name").get<std::string>();
    SIM_DEBUG("Etape rotation : {} instances (pas de {:.2f}°)", n, 360.0 /n);
    return std::make_unique<RotationLayoutAugmentation>(object_name, n);
}

std::unique_ptr<PipelineStep> PipelineFactory::parse_grid_position(const nlohmann::json& grid_position_step){
    return std::make_unique<GridPositionLayoutAugmentation>(
        grid_position_step.at("entity_name").get<std::string>(),
        grid_position_step.at("min_x").get<double>(),
        grid_position_step.at("max_x").get<double>(),
        grid_position_step.at("step_x").get<double>(),
        grid_position_step.at("min_y").get<double>(),
        grid_position_step.at("max_y").get<double>(),
        grid_position_step.at("step_y").get<double>(),
        grid_position_step.at("min_z").get<double>(),
        grid_position_step.at("max_z").get<double>(),
        grid_position_step.at("step_z").get<double>(),
        grid_position_step.value("exclusion_radius", 0.0)
    );
}

std::unique_ptr<PipelineStep> PipelineFactory::parse_keyframe(const nlohmann::json& keyframe_step){
    std::vector<KeyframesConfig> configs;

    for(const auto& scenario : keyframe_step.at("scenarios")){
        KeyframesConfig kc;
        kc.scenario_name = scenario.at("scenario_name").get<std::string>();

        for(const auto& obj : scenario.at("objects")){
            ObjectAnimation anim;
            anim.object_name = obj.at("entity_name").get<std::string>();

            std::string dir = obj.at("keyframes_directory").get<std::string>();
            std::string ext_ply = obj.value("extension", ".ply");

            for(const auto& file : std::filesystem::directory_iterator(dir)){
                if(file.is_regular_file() && file.path().extension() == ext_ply){
                    anim.keyframes_paths.push_back(file.path().string());
                }
            }

            SIM_DEBUG(" {} : {} keyframes trouvées depuis {}", anim.object_name, anim.keyframes_paths.size(), dir);
            kc.objects_animation.push_back(anim);
        }
        configs.push_back(kc);
    }

    return std::make_unique<KeyframeLayoutAugmentation>(configs);
}

std::unique_ptr<PipelineStep> PipelineFactory::parse_linear_position(const nlohmann::json &linear_position_step)
{   
    std::string temp = linear_position_step.at("entity_name").get<std::string>();
    return std::make_unique<LinearPositionLayoutAugmentation>(
        linear_position_step.at("entity_name").get<std::string>(),
        Axis::X,
        linear_position_step.at("min").get<double>(),
        linear_position_step.at("max").get<double>(),
        linear_position_step.at("step").get<double>()
    );
}

GridPositionLayoutAugmentation::GridPositionLayoutAugmentation(std::string object_name, 
    double min_x, double max_x, double step_x, 
    double min_y, double max_y, double step_y, 
    double min_z, double max_z, double step_z,
    double exclusion_radius)
    : m_object_name(object_name), m_min_x(min_x), m_max_x(max_x), m_step_x(step_x),
    m_min_y(min_y), m_max_y(max_y), m_step_y(step_y), m_min_z(min_z), m_max_z(max_z), m_step_z(step_z),
    m_exclusion_radius(exclusion_radius){}

std::vector<std::unique_ptr<Scene>> GridPositionLayoutAugmentation::process(std::vector<std::unique_ptr<Scene>> input_scenes, AssetManager &assets)
{
    (void)assets;
    // les points sur la grille
    std::vector<std::tuple<double, double, double>> grid_points;

    for(double x = m_min_x; x <= m_max_x; x += m_step_x){
        for(double y = m_min_y; y <= m_max_y; y += m_step_y){
            for(double z = m_min_z; z <= m_max_z; z += m_step_z){
                if(m_exclusion_radius > 0.0 && std::sqrt(x*x + y*y + z*z) < m_exclusion_radius){
                    continue;
                }
                grid_points.push_back({x, y, z});
            }
        }
    }

    std::vector<std::unique_ptr<Scene>> output;

    for(const auto& base_scene : input_scenes) {
        auto variation_scene = std::make_unique<Scene>(*base_scene);

        const auto& p = grid_points[rand() % grid_points.size()];

        for(const auto& entity : variation_scene->entities()) {
            if(entity->name() == m_object_name) {

                Pose grid_pos(
                    { std::get<0>(p),
                    std::get<1>(p),
                    std::get<2>(p) },
                    to_degrees(entity->pose().rx()),
                    to_degrees(entity->pose().ry()),
                    to_degrees(entity->pose().rz()));

                entity->pose(grid_pos);
                break;
            }
        }

        output.push_back(std::move(variation_scene));
    }

    return output;
}

LinearPositionLayoutAugmentation::LinearPositionLayoutAugmentation(std::string entity_name, Axis axis, double min, double max, double step)
    : m_entity_name(entity_name), m_axis{axis}, m_min(min), m_max(max), m_step(step){}

std::vector<std::unique_ptr<Scene>> LinearPositionLayoutAugmentation::process(std::vector<std::unique_ptr<Scene>> input_scenes, AssetManager &assets)
{
    (void)assets;
    std::vector<std::unique_ptr<Scene>> output_scenes;
    for(const auto& scene : input_scenes){
        for(double pos = m_min; pos <= m_max ;pos+= m_step){
            auto variation_scene = std::make_unique<Scene>(*scene);
            for(const auto& ent : variation_scene->entities()) {
                if(ent->name() != m_entity_name) continue;
                const auto& current = ent->pose();
                Pose p = current;

                switch(m_axis) {
                    case Axis::X:
                        p = Pose({pos, current.pos().y(), current.pos().z()}, current.rx(), current.ry(), current.rz());
                        break;
                    case Axis::Y:
                        p = Pose({current.pos().x(), pos, current.pos().z()}, current.rx(), current.ry(), current.rz());
                        break;
                    case Axis::Z:
                        p = Pose({current.pos().x(), current.pos().y(), pos}, current.rx(), current.ry(), current.rz());
                        break;
                }

                ent->pose(p);
                break;
            }

            output_scenes.push_back(std::move(variation_scene));
        }
    }
    return output_scenes;
}