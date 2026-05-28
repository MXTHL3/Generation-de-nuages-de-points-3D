#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>
#include <memory>
#include <filesystem>

#include <CGAL/config.h>

#include "scene.hpp"
#include "scene_utils.hpp"
#include "point_cloud_exporter.hpp"
#include "pipeline.hpp"
#include "logger.hpp"

std::string scene_path = "scenes_config/test_scene.json";

std::string human_base_path = "ply_models/human_base/human_base_";

int main(int argc, char* argv[]) {
    try {
        Logger::init();
        Scene world;
        AssetManager assets;
        std::vector<KeyframesConfig> keyframe_configs;
        
        KeyframesConfig c;
        c.scenario_name = "augmentation_run_humain";
        ObjectAnimation human_anim;
        human_anim.object_name = "human_1";
        
        ObjectAnimation human_anim_2;
        human_anim.object_name = "human_2";

        for(int i = 0; i <= 15; i++){
            human_anim.keyframes_paths.push_back(human_base_path + std::to_string(i) + ".ply");
            human_anim_2.keyframes_paths.push_back(human_base_path + std::to_string(i) + ".ply");
        }

        c.objects_animation.push_back(human_anim);
        c.objects_animation.push_back(human_anim_2);
        keyframe_configs.push_back(c);

        if(argc > 1) {

            scene_path = argv[1];
            return 1;

        }else{
            if(SceneLoader::load_scene_from_json(scene_path, world, assets)){
                Pipeline pipeline;
                std::shared_ptr<KeyframeLayoutAugmentation> keyframe_augmentation = std::make_shared<KeyframeLayoutAugmentation>(keyframe_configs);
                pipeline.add_step(keyframe_augmentation);

                std::shared_ptr<SpatialLayoutAugmentation> spatial_augmentation = std::make_shared<SpatialLayoutAugmentation>(100, 42);
                pipeline.add_step(spatial_augmentation);
                
                std::vector<std::shared_ptr<Scene>> scenes = pipeline.execute(std::make_shared<Scene>(world), assets);

                PlyExporter exporter;
                std::vector<Point3> resultCloud;
                int i = 0;
                for(auto& scene : scenes){
                    scene->build();
                    resultCloud = scene->scan(0);
                    // on relache la scene pour libérer le cache mémoire en mémoire sinon il sera uniquement relaché après la boucle
                    // TODO :: soluce temporaire il faut surement retirer les shared_ptr car pas utile pour scene soit (unique_ptr?)
                    scene.reset();


                    i++;
                    int part_n = i / 500;
                    std::string dir = "dataset/p"+ std::to_string(part_n);
                    std::filesystem::create_directories(dir);
                    exporter.save(dir + "/test_" + std::to_string(i) + ".ply", resultCloud);
                    
                }
            }
            return 0;
        }
    } catch(const std::exception& e){
        std::cerr << "Erreur : " << e.what() << std::endl;
        return 1;
    }
    return 0;
}