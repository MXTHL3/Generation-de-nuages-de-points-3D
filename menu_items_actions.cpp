#include "menu_items_actions.h"
#include "pipeline.h"
#include "asset_manager.h"
#include "scene_utils.h"
#include "noise_model.h"
#include "point_cloud_exporter.h"
#include "logger.h"
#include <filesystem>

void MenuItemsActions::open_3d_model() {
    _sub->signal_activate().connect([this]() {
        m_handle_file.open_model(as_window(), [this](const std::string& path) {
            m_gl->load_file(path);
            update_markers();
        });
    });
}

void MenuItemsActions::load_scan() {
    _sub->signal_activate().connect([this]() {
        Gtk::FileChooserDialog dialog("Charger un nuage de points", Gtk::FILE_CHOOSER_ACTION_OPEN);
        dialog.set_transient_for(as_window());
        dialog.add_button("Annuler", Gtk::RESPONSE_CANCEL);
        dialog.add_button("Ouvrir",  Gtk::RESPONSE_OK);

        auto filter_ply = Gtk::FileFilter::create();
        filter_ply->set_name("Nuages de points (*.ply, *.las, *.laz)");
        filter_ply->add_pattern("*.ply");
        filter_ply->add_pattern("*.las");
        filter_ply->add_pattern("*.laz");
        dialog.add_filter(filter_ply);

        auto filter_all = Gtk::FileFilter::create();
        filter_all->set_name("Tous les fichiers");
        filter_all->add_pattern("*");
        dialog.add_filter(filter_all);

        if (dialog.run() == Gtk::RESPONSE_OK)
            m_gl->load_scan(dialog.get_filename());
    });
}

void MenuItemsActions::load_json_scene() {
    _sub->signal_activate().connect([this]() {

        Gtk::FileChooserDialog dialog(
            "Charger une scène JSON",
            Gtk::FILE_CHOOSER_ACTION_OPEN);

        dialog.set_transient_for(as_window());

        dialog.add_button("Annuler", Gtk::RESPONSE_CANCEL);
        dialog.add_button("Ouvrir",  Gtk::RESPONSE_OK);

        auto filter_json = Gtk::FileFilter::create();
        filter_json->set_name("Scènes JSON (*.json)");
        filter_json->add_pattern("*.json");
        dialog.add_filter(filter_json);

        auto filter_all = Gtk::FileFilter::create();
        filter_all->set_name("Tous les fichiers");
        filter_all->add_pattern("*");
        dialog.add_filter(filter_all);

        if (std::filesystem::exists("test_scene.json"))
            dialog.set_filename("test_scene.json");

        if (dialog.run() != Gtk::RESPONSE_OK)
            return;

        const std::string filepath = dialog.get_filename();
        std::filesystem::path base_dir = std::filesystem::canonical("/proc/self/exe").parent_path();

        Scene world;
        AssetManager assets;

        if (!SceneLoader::load_scene_from_json(filepath, world, assets))
        {
            Gtk::MessageDialog err(
                as_window(),
                "Impossible de charger la scène :\n" + filepath,
                false,
                Gtk::MESSAGE_ERROR,
                Gtk::BUTTONS_OK,
                true);

            err.run();
            return;
        }

        world.build();

        m_gl->reset_scene();

        int entity_idx = 1; 
        for (const auto& ent : world.entities()) {
            if (auto staticEnt = std::dynamic_pointer_cast<StaticEntity>(ent)) {
                std::filesystem::path abs_path = base_dir / staticEnt->mesh_path();
                m_gl->load_file(abs_path.string());

                const Pose& p = staticEnt->pose();
                const Point3& pos = p.pos();

                ModelTransform tr;
                tr.pos_x = static_cast<float>(pos.x());
                tr.pos_y = static_cast<float>(pos.y());
                tr.pos_z = static_cast<float>(pos.z());
                tr.angle_x = static_cast<float>(p.rx());
                tr.angle_y = static_cast<float>(p.ry());
                tr.angle_z = static_cast<float>(p.rz());
                tr.use_offset = false;

                m_gl->set_transform(entity_idx, tr);
                ++entity_idx;
            }
        }

        for (const auto& ent : world.entities()) {
            if (auto lidarEnt = std::dynamic_pointer_cast<LidarEntity>(ent)) {
                const Pose& p = lidarEnt->pose();
                const Point3& pos = p.pos();

                m_gl->set_camera(
                    static_cast<float>(pos.x()),
                    static_cast<float>(pos.y()),
                    static_cast<float>(pos.z()),
                    static_cast<float>(p.rx()),
                    static_cast<float>(p.ry()),
                    5.0f  
                );
                break; 
            }
        }

        update_markers();

        Gtk::MessageDialog ok(
            as_window(),
            "Scène chargée avec succès :\n" + filepath,
            false,
            Gtk::MESSAGE_INFO,
            Gtk::BUTTONS_OK,
            true);

        ok.run();
    });
}

void MenuItemsActions::capture_image() {
    _sub->signal_activate().connect([this]() {
        Gtk::FileChooserDialog dialog("Enregistrer l'image", Gtk::FILE_CHOOSER_ACTION_SAVE);
        dialog.set_transient_for(as_window());
        dialog.add_button("Annuler", Gtk::RESPONSE_CANCEL);
        dialog.add_button("Enregistrer", Gtk::RESPONSE_OK);
        dialog.set_do_overwrite_confirmation(true);
        dialog.set_current_name("capture.png");

        auto filter_png = Gtk::FileFilter::create();
        filter_png->set_name("Images PNG (*.png)");
        filter_png->add_pattern("*.png");
        dialog.add_filter(filter_png);

        auto filter_all = Gtk::FileFilter::create();
        filter_all->set_name("Tous les fichiers");
        filter_all->add_pattern("*");
        dialog.add_filter(filter_all);

        if (dialog.run() == Gtk::RESPONSE_OK)
            m_gl->capture_image(dialog.get_filename());
    });
}

void MenuItemsActions::launch_scan() {
    _sub->signal_activate().connect([this]() {
        Gtk::FileChooserDialog dialog("Enregistrer le nuage de points", Gtk::FILE_CHOOSER_ACTION_SAVE);
        dialog.set_transient_for(as_window());
        dialog.add_button("Annuler", Gtk::RESPONSE_CANCEL);
        dialog.add_button("Enregistrer", Gtk::RESPONSE_OK);

        auto filter_ply = Gtk::FileFilter::create();
        filter_ply->set_name("PLY (*.ply)");
        filter_ply->add_pattern("*.ply");

        auto filter_las = Gtk::FileFilter::create();
        filter_las->set_name("LAS (*.las)");
        filter_las->add_pattern("*.las");

        dialog.add_filter(filter_ply);
        dialog.add_filter(filter_las);
        dialog.set_filter(filter_ply);
        dialog.set_current_name("scan.ply");

        if (dialog.run() == Gtk::RESPONSE_OK) {
            m_gl->run_scan(
                m_gl->get_lidar_config(),
                dialog.get_filename());
        }
    });
}

void MenuItemsActions::display_cloud() {
    _sub->signal_activate().connect([this]() {
        m_gl->toggle_point_cloud();
    });
}

void MenuItemsActions::generate_dataset() {
    _sub->signal_activate().connect([this]() {
        Gtk::FileChooserDialog dialog(
            "Sélectionner les scans à ajouter au dataset",
            Gtk::FILE_CHOOSER_ACTION_OPEN);
        dialog.set_transient_for(as_window());
        dialog.set_select_multiple(true);
        dialog.add_button("Annuler",       Gtk::RESPONSE_CANCEL);
        dialog.add_button("Sélectionner",  Gtk::RESPONSE_OK);
 
        auto filter_pc = Gtk::FileFilter::create();
        filter_pc->set_name("Nuages de points (*.ply, *.las, *.laz)");
        filter_pc->add_pattern("*.ply");
        filter_pc->add_pattern("*.las");
        filter_pc->add_pattern("*.laz");
        dialog.add_filter(filter_pc);
 
        auto filter_all = Gtk::FileFilter::create();
        filter_all->set_name("Tous les fichiers");
        filter_all->add_pattern("*");
        dialog.add_filter(filter_all);
 
        if (dialog.run() != Gtk::RESPONSE_OK) return;
        std::vector<std::string> fichiers = dialog.get_filenames();
        dialog.hide();
 
        if (fichiers.empty()) return;
 
        Gtk::Dialog cls_dialog("Classe des scans sélectionnés", as_window(), true);
        cls_dialog.add_button("Annuler",   Gtk::RESPONSE_CANCEL);
        cls_dialog.add_button("Confirmer", Gtk::RESPONSE_OK);
 
        auto* content = cls_dialog.get_content_area();
        auto* box = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL, 8);
        box->set_margin_start(16); box->set_margin_end(16);
        box->set_margin_top(12);   box->set_margin_bottom(12);
 
        auto* label_cls = Gtk::make_managed<Gtk::Label>(
            std::to_string(fichiers.size()) + " fichier(s) sélectionné(s).\n"
            "Classe (ex: humain, non_humain) :");
        Gtk::Entry entry_cls;
        entry_cls.set_text("humain");
 
        box->pack_start(*label_cls, Gtk::PACK_SHRINK);
        box->pack_start(entry_cls,  Gtk::PACK_SHRINK);
        content->pack_start(*box,   Gtk::PACK_SHRINK);
        cls_dialog.show_all_children();
 
        if (cls_dialog.run() != Gtk::RESPONSE_OK) return;
        std::string cls = entry_cls.get_text();
        if (cls.empty()) cls = "inconnu";
        cls_dialog.hide();  
 
        Gtk::FileChooserDialog dir_dialog(
            "Choisir le dossier de destination du dataset",
            Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER);
        dir_dialog.set_transient_for(as_window());
        dir_dialog.add_button("Annuler",      Gtk::RESPONSE_CANCEL);
        dir_dialog.add_button("Sélectionner", Gtk::RESPONSE_OK);
 
        if (dir_dialog.run() != Gtk::RESPONSE_OK) return;
        std::string folder     = dir_dialog.get_filename();
        std::string folder_cls = folder + "/" + cls;
        dir_dialog.hide();  
 
        std::filesystem::create_directories(folder_cls);
 
        int succes = 0;
        for (const auto& src : fichiers) {
            std::string nom  = std::filesystem::path(src).filename().string();
            std::string dest = folder_cls + "/" + nom;
            try {
                std::filesystem::copy_file(src, dest,
                    std::filesystem::copy_options::overwrite_existing);
                succes++;
            } catch (const std::exception& e) {
                std::cerr << "Erreur copie " << nom << " : " << e.what() << "\n";
            }
        }
 
        Gtk::MessageDialog msg(as_window(),
            "Dataset mis à jour !\n" +
            std::to_string(succes) + " / " + std::to_string(fichiers.size()) +
            " fichier(s) copiés dans :\n" + folder_cls,
            false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK, true);
        msg.run();
        msg.hide();  
 
        std::filesystem::path base_dir =
            std::filesystem::canonical("/proc/self/exe").parent_path();
        std::string script_dir   = (base_dir / "training_model").string();
        std::string train_script = (base_dir / "training_model/train_ai.py").string();
 
        if (!std::filesystem::exists(train_script)) {
            std::cerr << "train_ai.py introuvable : " << train_script << "\n";
            return;
        }
 
        Gtk::MessageDialog train_ask(as_window(),
            "Voulez-vous lancer l'entrainement IA maintenant\navec les nouvelles donnees ?",
            false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_YES_NO, true);
        int train_rep = train_ask.run();
        train_ask.hide();
        if (train_rep != Gtk::RESPONSE_YES) return;
 
        setenv("DATASET_DIR", folder.c_str(), 1);
 
        std::string cmd = "cd \"" + script_dir + "\" && python3 \"" + train_script + "\" 2>&1";
 
        FILE* pipe = popen(cmd.c_str(), "r");
        if (!pipe) {
            Gtk::MessageDialog err(as_window(), "Impossible de lancer Python.",
                false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);
            err.run();
            unsetenv("DATASET_DIR");
            return;
        }
 
        std::string output;
        char buffer[256];
        while (fgets(buffer, sizeof(buffer), pipe))
            output += buffer;
        pclose(pipe);
 
        unsetenv("DATASET_DIR");
 
        Gtk::MessageDialog done(as_window(),
            "Entraînement terminé :\n\n" + output,
            false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK, true);
        done.run();
    });
}

void MenuItemsActions::generate_dataset_from_json() {
    _sub->signal_activate().connect([this]() {
        Gtk::FileChooserDialog dialog("Choisir la scène de base (JSON)",
                                      Gtk::FILE_CHOOSER_ACTION_OPEN);
        dialog.set_transient_for(as_window());
        dialog.add_button("Annuler", Gtk::RESPONSE_CANCEL);
        dialog.add_button("Ouvrir",  Gtk::RESPONSE_OK);
        auto filter = Gtk::FileFilter::create();
        filter->set_name("Scènes JSON (*.json)");
        filter->add_pattern("*.json");
        dialog.add_filter(filter);

        if (dialog.run() != Gtk::RESPONSE_OK) return;
        std::string scene_path = dialog.get_filename();

        AssetManager assets;

        auto base_scene = std::make_unique<Scene>();
        if (!SceneLoader::load_scene_from_json(scene_path, *base_scene, assets)) {
            SIM_ERROR("Impossible de charger la scène : {}", scene_path);
            return;
        }
        SIM_INFO("Scène de base chargée depuis {}", scene_path);

        Pipeline pipeline;

        pipeline.add_step(std::make_unique<PositionLayoutAugmentation>(5, /*seed=*/42));
        pipeline.add_step(std::make_unique<RotationLayoutAugmentation>(4, 0.0, 360.0, /*seed=*/123));

        auto augmented_scenes = pipeline.execute(std::move(base_scene), assets);
        SIM_INFO("{} scènes générées par le pipeline", augmented_scenes.size());

        std::filesystem::create_directories("dataset_output");
        int exported = 0;

        for (size_t i = 0; i < augmented_scenes.size(); ++i) {
            auto& scene = augmented_scenes[i];
            scene->build();

            auto cloud = scene->scan(0); 

            if (cloud.empty()) {
                SIM_WARNING("Scène {} : aucun point scanné, ignorée", i);
                continue;
            }

            std::string out_path = "dataset_output/scan_" + std::to_string(i) + ".ply";
            PlyExporter exporter;
            try {
                exporter.save(out_path, cloud);
                SIM_INFO("Exporté : {} ({} points)", out_path, cloud.size());
                exported++;
            } catch (const std::exception& e) {
                SIM_ERROR("Erreur export {} : {}", out_path, e.what());
            }
        }

        std::string msg = std::to_string(exported) + " fichiers PLY générés dans dataset_output/";
        Gtk::MessageDialog result(as_window(), msg, false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK, true);
        result.set_title("Génération terminée");
        result.run();
    });
}

void MenuItemsActions::launch_recognition() {
    _sub->signal_activate().connect([this]() {

        Gtk::FileChooserDialog dialog(
            "Sélectionner un fichier .ply à analyser",
            Gtk::FILE_CHOOSER_ACTION_OPEN);
        dialog.set_transient_for(as_window());
        dialog.add_button("Annuler",  Gtk::RESPONSE_CANCEL);
        dialog.add_button("Analyser", Gtk::RESPONSE_OK);
 
        auto filter_ply = Gtk::FileFilter::create();
        filter_ply->set_name("Nuages de points (*.ply)");
        filter_ply->add_pattern("*.ply");
        dialog.add_filter(filter_ply);
 
        auto filter_all = Gtk::FileFilter::create();
        filter_all->set_name("Tous les fichiers");
        filter_all->add_pattern("*");
        dialog.add_filter(filter_all);
 
        if (dialog.run() != Gtk::RESPONSE_OK) return;
        std::string ply_path = dialog.get_filename();
        dialog.hide();
 
        std::filesystem::path base_dir =
            std::filesystem::canonical("/proc/self/exe").parent_path();
        std::string script_dir = (base_dir / "training_model").string();
        std::string modele     = (base_dir / "training_model/modele_laser.pth").string();
        std::string script     = (base_dir / "training_model/test_ai.py").string();
 
        std::cout << "base_dir       = " << base_dir << "\n"
                  << "script         = " << script   << "\n"
                  << "modele         = " << modele   << "\n"
                  << "exists(script) = " << std::filesystem::exists(script) << "\n"
                  << "exists(modele) = " << std::filesystem::exists(modele) << "\n";
 
        if (!std::filesystem::exists(script)) {
            Gtk::MessageDialog err(as_window(),
                "Script introuvable : " + script,
                false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);
            err.run(); return;
        }
        if (!std::filesystem::exists(modele)) {
            Gtk::MessageDialog err(as_window(),
                "Modèle introuvable : " + modele +
                "\nLancez d'abord train_ai.py pour générer 'modele_laser.pth'.",
                false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);
            err.run(); return;
        }
 
        setenv("PLY_PATH", ply_path.c_str(), 1);
        setenv("MODELE_PATH", modele.c_str(), 1);
 
        std::string cmd =
            "cd \"" + script_dir + "\" && python3 \"" + script + "\" 2>&1";
 
        FILE* pipe = popen(cmd.c_str(), "r");
        if (!pipe) {
            Gtk::MessageDialog err(as_window(), "Impossible de lancer Python.",
                false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);
            err.run();
            unsetenv("PLY_PATH");
            unsetenv("MODELE_PATH");
            return;
        }
 
        std::string output;
        char buffer[256];
        while (fgets(buffer, sizeof(buffer), pipe))
            output += buffer;
        int ret = pclose(pipe);
 
        unsetenv("PLY_PATH");
        unsetenv("MODELE_PATH");
 
        std::string verdict;
        {
            std::istringstream iss(output);
            std::string line;
            while (std::getline(iss, line))
                if (!line.empty()) verdict = line;
        }
 
        bool is_human = (verdict.find("Humain") != std::string::npos &&
                         verdict.find("Non") == std::string::npos);
 
        std::string filename = std::filesystem::path(ply_path).filename().string();
        std::string message  = "Fichier analysé : " + filename +
                               "\n\nRésultat : " +
                               (verdict.empty() ? "(aucune sortie)" : verdict) + "\n";
 
        if (ret != 0 || verdict.find("Erreur") != std::string::npos) {
            message += "\nDétails :\n" + output;
            Gtk::MessageDialog result(as_window(), message,
                false, Gtk::MESSAGE_WARNING, Gtk::BUTTONS_OK, true);
            result.set_title("Analyse IA — Erreur");
            result.run();
        } else {
            Gtk::MessageDialog result(as_window(), message,
                false,
                is_human ? Gtk::MESSAGE_INFO : Gtk::MESSAGE_QUESTION,
                Gtk::BUTTONS_OK, true);
            result.set_title(is_human ? "Analyse IA — Humain détecté"
                                      : "Analyse IA — Non-Humain");
            result.run();
        }
    });
}


void MenuItemsActions::to_fullscreen() {
    _sub->signal_activate().connect([this]() {
        auto state = as_window().get_window()->get_state();

        if (!(state & Gdk::WINDOW_STATE_FULLSCREEN))
            as_window().fullscreen();
        else
            as_window().unfullscreen();
    });
}

void MenuItemsActions::open_docs() {
    _sub->signal_activate().connect([this]() {
        std::string doc_path = std::filesystem::absolute("html/index.html").string();

        if (doc_path.rfind("/mnt/", 0) == 0) {
            doc_path = doc_path.substr(5);           
            doc_path[0] = std::toupper(doc_path[0]); 
            doc_path.insert(1, ":\\");               
            doc_path = doc_path.substr(0, 3) + doc_path.substr(3); 
            for (char& c : doc_path)
                if (c == '/') c = '\\';              
        }

        std::string cmd = "cmd.exe /c start \"\" \"" + doc_path + "\"";
        std::thread([cmd]() {
            int ret = std::system(cmd.c_str());
            (void)ret;
        }).detach();
    });
}

void MenuItemsActions::reset_scene() {
    _sub->signal_activate().connect([this]() {
        Gtk::MessageDialog confirm(as_window(),
            "Supprimer tous les modèles chargés ?\nLe cube par défaut sera conservé.",
            false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_YES_NO, true);
        if (confirm.run() != Gtk::RESPONSE_YES) return;
 
        m_gl->reset_scene();
        m_handle_file = HandleFile{};   
        update_markers();
    });
}

void MenuItemsActions::scanner_settings() {
    _sub->signal_activate().connect([this]() {
        struct Defaults {
            double min_range = 1.0;
            double max_range = 1000.0;
            double h_step0 = 0.703125;
            double h_step1 = 0.3515626;
            double h_step2 = 0.1757825;
            double accuracy = 0.01;
        };
        static const Defaults DEF;
 
        Gtk::Dialog dlg("Paramètres scanner", as_window(), true);
        dlg.set_default_size(420, -1);
        dlg.set_resizable(false);
        dlg.add_button("Fermer", Gtk::RESPONSE_CLOSE);
 
        auto* area = dlg.get_content_area();
        auto* vbox = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL, 10);
        vbox->set_margin_start(16); vbox->set_margin_end(16);
        vbox->set_margin_top(12); vbox->set_margin_bottom(12);
 
        {
            auto* row = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL, 8);
            auto* lbl = Gtk::make_managed<Gtk::Label>("Modèle LiDAR :");
            lbl->set_xalign(0.0f);
            lbl->set_size_request(140, -1);
            row->pack_start(*lbl, Gtk::PACK_SHRINK);
 
            auto* combo = Gtk::make_managed<Gtk::ComboBoxText>();
            combo->append("lidars_config/ouster_os1_64.json", "Ouster OS1-64");
            combo->append("lidars_config/ouster_os2_128.json", "Ouster OS2-128");
            combo->append("lidars_config/vedolyne_vlp16.json", "Velodyne VLP-16");
            combo->append("lidars_config/velodyne_vlp32c.json", "Velodyne VLP-32C");
            combo->set_active_id(m_gl->get_lidar_config());
            if (combo->get_active_row_number() < 0) combo->set_active(0);
 
            combo->signal_changed().connect([this, combo]() {
                std::string chosen = combo->get_active_id();
                if (!chosen.empty()) {
                    m_gl->set_lidar_config(chosen);
                    m_gl->clear_lidar_override();  
                }
            });
 
            row->pack_start(*combo, Gtk::PACK_EXPAND_WIDGET);
            vbox->pack_start(*row, Gtk::PACK_SHRINK);
        }
 
        vbox->pack_start(*Gtk::make_managed<Gtk::Separator>(Gtk::ORIENTATION_HORIZONTAL),
                         Gtk::PACK_SHRINK);
 
        auto make_slider = [&](const std::string& label, double vmin, double vmax, double step, double value,
            int decimals, std::function<void(double)> on_change) {
            auto* row  = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL, 8);
            auto* lbl  = Gtk::make_managed<Gtk::Label>(label + " :");
            lbl->set_xalign(0.0f);
            lbl->set_size_request(140, -1);
 
            auto* scale = Gtk::make_managed<Gtk::Scale>(Gtk::ORIENTATION_HORIZONTAL);
            scale->set_range(vmin, vmax);
            scale->set_increments(step, step * 10);
            scale->set_value(value);
            scale->set_digits(decimals);
            scale->set_hexpand(true);
            scale->set_draw_value(true);
            scale->set_value_pos(Gtk::POS_RIGHT);
 
            scale->signal_value_changed().connect([scale, on_change]() {
                on_change(scale->get_value());
            });
 
            row->pack_start(*lbl,   Gtk::PACK_SHRINK);
            row->pack_start(*scale, Gtk::PACK_EXPAND_WIDGET);
            vbox->pack_start(*row,  Gtk::PACK_SHRINK);
            return scale;
        };
 
        auto cur = m_gl->get_lidar_override();
 
        double init_min = cur ? cur->m_min_dist : DEF.min_range;
        double init_max = cur ? cur->m_max_dist : DEF.max_range;
        double init_hs0 = cur ? (cur->m_h_step.size() > 0 ? cur->m_h_step[0] : DEF.h_step0) : DEF.h_step0;
        double init_hs1 = cur ? (cur->m_h_step.size() > 1 ? cur->m_h_step[1] : DEF.h_step1) : DEF.h_step1;
        double init_hs2 = cur ? (cur->m_h_step.size() > 2 ? cur->m_h_step[2] : DEF.h_step2) : DEF.h_step2;
        double init_acc = cur ? cur->m_accuracy : DEF.accuracy;
 
        auto* s_min = make_slider("Min range (m)", 0.1, 50.0, 0.1, init_min, 1, [this](double v){ m_gl->lidar_override_set_min(v); });
        auto* s_max = make_slider("Max range (m)", 10.0, 2000.0, 5.0, init_max, 0, [this](double v){ m_gl->lidar_override_set_max(v); });
        auto* s_hs0 = make_slider("H-step fin (°)", 0.05, 5.0, 0.01, init_hs0, 3, [this](double v){ m_gl->lidar_override_set_hstep(0, v); });
        auto* s_hs1 = make_slider("H-step moyen (°)", 0.05, 5.0, 0.01, init_hs1, 3, [this](double v){ m_gl->lidar_override_set_hstep(1, v); });
        auto* s_hs2 = make_slider("H-step large (°)", 0.05, 5.0, 0.01, init_hs2, 3, [this](double v){ m_gl->lidar_override_set_hstep(2, v); });
        auto* s_acc = make_slider("Précision (m)", 0.001, 0.5, 0.001, init_acc, 3, [this](double v){ m_gl->lidar_override_set_accuracy(v); });
 
        vbox->pack_start(*Gtk::make_managed<Gtk::Separator>(Gtk::ORIENTATION_HORIZONTAL), Gtk::PACK_SHRINK);
 
        auto* btn_reset = Gtk::make_managed<Gtk::Button>("Réinitialiser");
        btn_reset->signal_clicked().connect([=]() {
            s_min->set_value(DEF.min_range);
            s_max->set_value(DEF.max_range);
            s_hs0->set_value(DEF.h_step0);
            s_hs1->set_value(DEF.h_step1);
            s_hs2->set_value(DEF.h_step2);
            s_acc->set_value(DEF.accuracy);
        });
 
        auto* btn_box = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL);
        btn_box->pack_end(*btn_reset, Gtk::PACK_SHRINK);
        vbox->pack_start(*btn_box, Gtk::PACK_SHRINK);
 
        area->pack_start(*vbox, Gtk::PACK_SHRINK);
        dlg.show_all_children();
        dlg.run();
    });
}



void MenuItemsActions::exit_app() {
    _sub->signal_activate().connect([this]() {
        as_window().hide();
    });
}