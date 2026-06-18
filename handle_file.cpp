#include "handle_file.h"

void HandleFile::open_model(Gtk::Window& parent, std::function<void(const std::string&)> on_loaded)
{
    Gtk::FileChooserDialog dialog("Ouvrir un modèle 3D",
                                  Gtk::FILE_CHOOSER_ACTION_OPEN);
    dialog.set_transient_for(parent);
    dialog.set_modal(true);
    dialog.set_select_multiple(true);

    dialog.add_button("Annuler", Gtk::RESPONSE_CANCEL);
    dialog.add_button("Ouvrir",  Gtk::RESPONSE_OK);

    auto filter_mesh = Gtk::FileFilter::create();
    filter_mesh->set_name("Modèles 3D (*.obj, *.ply, *.las, *.laz)");
    filter_mesh->add_pattern("*.obj");
    filter_mesh->add_pattern("*.ply");
    filter_mesh->add_pattern("*.las");
    filter_mesh->add_pattern("*.laz");
    dialog.add_filter(filter_mesh);

    auto filter_all = Gtk::FileFilter::create();
    filter_all->set_name("Tous les fichiers");
    filter_all->add_pattern("*");
    dialog.add_filter(filter_all);

    int result = dialog.run();

    if (result == Gtk::RESPONSE_OK) {
        std::vector<std::string> paths = dialog.get_filenames(); 
        for (const auto& path : paths) {
            m_loaded_models.push_back(path);
            if (on_loaded) on_loaded(path);
            std::cout << "Modèle chargé : " << path << std::endl;
        }
        std::cout << "Total modèles chargés : " << m_loaded_models.size() << std::endl;
    }
}