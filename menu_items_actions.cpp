#include "menu_items_actions.h"

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
        Gtk::FileChooserDialog dialog("Choisir le dossier de sortie pour l'ensemble de données", Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER);
        dialog.set_transient_for(as_window());
        dialog.add_button("Annuler", Gtk::RESPONSE_CANCEL);
        dialog.add_button("Sélectionner", Gtk::RESPONSE_OK);

        if (dialog.run() != Gtk::RESPONSE_OK) return;

        std::string folder = dialog.get_filename();

        Gtk::Dialog nb_dialog("Paramètres de génération", as_window(), true);
        nb_dialog.add_button("Annuler", Gtk::RESPONSE_CANCEL);
        nb_dialog.add_button("Générer", Gtk::RESPONSE_OK);

        auto* content = nb_dialog.get_content_area();
        auto* box = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL, 8);
        box->set_margin_start(16); box->set_margin_end(16);
        box->set_margin_top(12);   box->set_margin_bottom(12);

        auto* label_nb = Gtk::make_managed<Gtk::Label>("Nombre de scans à générer :");
        Gtk::SpinButton spin_nb;
        spin_nb.set_range(1, 1000);
        spin_nb.set_increments(1, 10);
        spin_nb.set_value(10);

        auto* label_cls = Gtk::make_managed<Gtk::Label>("Classe (ex: humain, non_humain) :");
        Gtk::Entry entry_cls;
        entry_cls.set_text("humain");

        box->pack_start(*label_nb, Gtk::PACK_SHRINK);
        box->pack_start(spin_nb, Gtk::PACK_SHRINK);
        box->pack_start(*label_cls, Gtk::PACK_SHRINK);
        box->pack_start(entry_cls, Gtk::PACK_SHRINK);
        content->pack_start(*box, Gtk::PACK_SHRINK);
        nb_dialog.show_all_children();

        if (nb_dialog.run() != Gtk::RESPONSE_OK) return;

        int nb_scans = static_cast<int>(spin_nb.get_value());
        std::string cls = entry_cls.get_text();

        std::string folder_cls = folder + "/" + cls;
        std::filesystem::create_directories(folder_cls);

        int succes = 0;
        for (int i = 0; i < nb_scans; ++i) {
            std::string chemin = folder_cls + "/" + cls + "_" + std::to_string(i + 1) + ".ply";
            try {
                m_gl->run_scan(m_gl->get_lidar_config(), chemin);
                succes++;
            } 
            catch (const std::exception& e) {
                std::cerr << "Erreur scan " << i+1 << " : " << e.what() << "\n";
            }
        }

        Gtk::MessageDialog msg(as_window(),
            "Génération terminée !\n" +
            std::to_string(succes) + " / " + std::to_string(nb_scans) +
            " scans générés dans :\n" + folder_cls,
            false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK, true);
        msg.run();
    });
}

void MenuItemsActions::launch_recognition() {
    _sub->signal_activate().connect([this]() {
        Gtk::FileChooserDialog dialog("Sélectionner un fichier .ply à analyser", Gtk::FILE_CHOOSER_ACTION_OPEN);
        dialog.set_transient_for(as_window());
        dialog.add_button("Annuler", Gtk::RESPONSE_CANCEL);
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
        std::string chemin_ply = dialog.get_filename();
        dialog.hide();

        std::string script = "training_model/test_ai.py";
        std::string modele = "training_model/modele_laser.pth";

        if (!std::filesystem::exists(script)) {
            Gtk::MessageDialog err(as_window(),
                "Script introuvable : " + script + "\n"
                "Vérifiez que le dossier training_model/ est présent.",
                false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);
            err.run(); return;
        }
        if (!std::filesystem::exists(modele)) {
            Gtk::MessageDialog err(as_window(),
                "Modèle introuvable : " + modele + "\n"
                "Lancez d'abord train_ai.py pour générer 'modele_laser.pth'.",
                false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);
            err.run(); return;
        }

        std::string cmd = "cd training_model && python3 -c \""
            "import sys; sys.path.insert(0, '.'); "
            "from test_ai import predire_un_fichier; "
            "import torch; "
            "from train_ai import PointNetClassifieur; "
            "import os; "
            "modele = PointNetClassifieur(); "
            "modele.load_state_dict(torch.load('modele_laser.pth', map_location='cpu')); "
            "modele.eval(); "
            "chemin = os.environ.get('PLY_PATH',''); "
            "verdict = predire_un_fichier(chemin, modele); "
            "print(verdict)\" 2>&1";

        setenv("PLY_PATH", chemin_ply.c_str(), 1);

        FILE* pipe = popen(cmd.c_str(), "r");
        if (!pipe) {
            Gtk::MessageDialog err(as_window(), "Impossible de lancer Python.",
                false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);
            err.run(); return;
        }

        std::string exit;
        char buffer[256];
        while (fgets(buffer, sizeof(buffer), pipe))
            exit += buffer;
        int ret = pclose(pipe);
        unsetenv("PLY_PATH");

        std::string verdict;
        {
            std::istringstream iss(exit);
            std::string line;
            while (std::getline(iss, line))
                if (!line.empty()) verdict = line;
        }

        bool is_human = (verdict.find("Humain") != std::string::npos && verdict.find("Non") == std::string::npos);

        std::string filename = std::filesystem::path(chemin_ply).filename().string();
        std::string message =
            "Fichier analysé : " + filename + "\n\n"
            "Résultat : " + (verdict.empty() ? "(aucune sortie)" : verdict) + "\n";

        if (ret != 0 || verdict.find("Erreur") != std::string::npos) {
            message += "\nDétails :\n" + exit;
            Gtk::MessageDialog result(as_window(), message,
                false, Gtk::MESSAGE_WARNING, Gtk::BUTTONS_OK, true);
            result.set_title("Analyse IA — Erreur");
            result.run();
        } 
        else {
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

void MenuItemsActions::exit_app() {
    _sub->signal_activate().connect([this]() {
        as_window().hide();
    });
}