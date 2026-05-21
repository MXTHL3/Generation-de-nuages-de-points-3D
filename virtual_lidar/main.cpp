#include <iostream>
#include <fstream>
#include <vector>
#include <memory>

#include "scene.hpp"
#include "scene_utils.hpp"
#include "point_cloud_exporter.hpp"

std::string scene_path = "scenes_config/test_scene.json";

int main(int argc, char* argv[]) {
    try {
        Scene world;
        AssetManager assets;

        if(argc > 1) {

            scene_path = argv[1];
            return 1;

        }else{
            if(SceneLoader::load_scene_from_json(scene_path, world, assets)){
                world.build();

                std::vector<Point3> resultCloud = world.scan(0);

                PlyExporter exporter;
                exporter.save("test.ply", resultCloud);
            }
            return 0;
        }
    } catch(const std::exception& e){
        std::cerr << "Erreur : " << e.what() << std::endl;
        return 1;
    }
    return 0;
}