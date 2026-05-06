#include "handle_file.h"

void HandleFile::open_model(Gtk::Window& parent, std::function<void(const std::string&)> on_loaded)
{
    Gtk::FileChooserDialog dialog("Ouvrir un modèle 3D",
                                  Gtk::FILE_CHOOSER_ACTION_OPEN);
    dialog.set_transient_for(parent);
    dialog.set_modal(true);

    dialog.add_button("Annuler", Gtk::RESPONSE_CANCEL);
    dialog.add_button("Ouvrir",  Gtk::RESPONSE_OK);

    auto filter_obj = Gtk::FileFilter::create();
    filter_obj->set_name("Modèles OBJ (*.obj)");
    filter_obj->add_pattern("*.obj");
    dialog.add_filter(filter_obj);

    auto filter_all = Gtk::FileFilter::create();
    filter_all->set_name("Tous les fichiers");
    filter_all->add_pattern("*");
    dialog.add_filter(filter_all);

    int result = dialog.run();

    if (result == Gtk::RESPONSE_OK) {
        std::string path = dialog.get_filename();
        m_loaded_models.push_back(path);
        if (on_loaded) on_loaded(path);
        std::cout << "Modèle chargé : " << path << std::endl;
        std::cout << "Total modèles chargés : " << m_loaded_models.size() << std::endl;
    }
}