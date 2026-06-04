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
#include "lidar_scanner.hpp"

std::string scene_path = "scenes_config/test_scene.json";

std::string human_base_anims_paths = "dataset/animations";

int main(int argc, char *argv[])
{
    try
    {
        Logger::init();
        Scene world;
        AssetManager assets;
        std::vector<KeyframesConfig> keyframe_configs;

        KeyframesConfig c;
        c.scenario_name = "augmentation_animation_humain";
        ObjectAnimation human_anim;
        human_anim.object_name = "human_1";

        for (const auto &file : std::filesystem::directory_iterator(human_base_anims_paths))
        {
            if (file.is_regular_file() && file.path().extension() == ".ply")
            {
                std::string file_path = file.path().string();
                human_anim.keyframes_paths.push_back(file_path);
                SIM_INFO("Keyframe trouvée : {}", file_path);
            }
        }

        c.objects_animation.push_back(human_anim);
        // c.objects_animation.push_back(human_anim_2);
        keyframe_configs.push_back(c);

        if (argc > 1)
        {

            scene_path = argv[1];
        }

        std::vector<std::shared_ptr<LidarEntity>> lidars;
        if (SceneLoader::load_scene_from_json(scene_path, world, lidars, assets))
        {
            SIM_INFO("ETAPE 1 - JSON chargé. Entités : {}", world.entities().size());
            Pipeline pipeline;

            auto scene_initiale = std::make_unique<Scene>(world);
            SIM_INFO("ETAPE 2 - Copie Initiale. Entités : {}, Lidars : {}", scene_initiale->entities().size());

            // pipeline.add_step(std::make_unique<PositionLayoutAugmentation>(100, 42));

            pipeline.add_step(std::make_unique<RotationLayoutAugmentation>(2, 0.0, 360.0, 42));

            pipeline.add_step(std::make_unique<KeyframeLayoutAugmentation>(keyframe_configs));

            std::vector<std::unique_ptr<Scene>> scenes = pipeline.execute(std::move(scene_initiale), assets);
            SIM_INFO("ETAPE 3 - Fin du Pipeline. Nombre de scènes générées : {}", scenes.size());

            PlyExporter exporter;
            std::vector<Point3> resultCloud;
            LidarScanner scanner;

            auto ouster_lidar = std::dynamic_pointer_cast<MechanicalLidarEntity>(lidars[0]);

            int i = 0;
            for (auto &scene : scenes)
            {
                scene->build();
                resultCloud = scanner.scan(ouster_lidar, *scene);
                // on relache la scene pour libérer le cache mémoire en mémoire sinon il sera uniquement relaché après la boucle
                // TODO :: soluce temporaire il faut surement retirer les shared_ptr car pas utile pour scene soit (unique_ptr?)
                scene.reset();

                SIM_INFO("Le nombre de points du nuages est : {}", resultCloud.size());

                i++;
                int part_n = i / 100000;
                std::string dir = "dataset/p" + std::to_string(part_n);
                std::filesystem::create_directories(dir);
                exporter.save(dir + "/test_" + std::to_string(i) + ".ply", resultCloud);
            }
        }
        else
        {
            SIM_ERROR("Le chargement de la scène {} a échoué", scene_path);
        }
        return 0;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Erreur : " << e.what() << std::endl;
        return 1;
    }
    return 0;
}