#include <iostream>
#include <fstream>
#include <vector>
#include <memory>

#include "scene.hpp"
#include "scene_utils.hpp"
#include "point_cloud_exporter.hpp"
#include "pipeline.hpp"
#include "logger.hpp"

std::string scene_path = "scenes_config/test_scene.json";

int main(int argc, char* argv[]) {
    try {
        Logger::init();
        Scene world;
        AssetManager assets;

        if(argc > 1) {

            scene_path = argv[1];
            return 1;

        }else{
            if(SceneLoader::load_scene_from_json(scene_path, world, assets)){
                Pipeline pipeline;
                std::shared_ptr<SpatialLayoutAugmentation> spatial_augmentation = std::make_shared<SpatialLayoutAugmentation>(360, 42);
                pipeline.add_step(spatial_augmentation);
                
                std::vector<std::shared_ptr<Scene>> scenes = pipeline.execute(std::make_shared<Scene>(world), assets);

                PlyExporter exporter;
                
                int i = 0;
                for(const auto& scene : scenes){
                    std::vector<Point3> resultCloud = scene->scan(0);

                    std::string result_name = "test_" + std::to_string(++i) + ".ply";

                    exporter.save(result_name, resultCloud);
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