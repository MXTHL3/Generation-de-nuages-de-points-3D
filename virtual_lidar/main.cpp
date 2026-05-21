#include <iostream>
#include <fstream>
#include <vector>
#include <memory>

#include "scene.hpp"
#include "scene_utils.hpp"
#include "point_cloud_exporter.hpp"

int main(int argc, char* argv[]) {
    try {
        Scene world;
        AssetManager assets;

        if(argc > 1) {

            // chill
            return 1;

        }else{
            if(SceneLoader::load_scene_from_json("scenes_config/test.json", world, assets)){
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