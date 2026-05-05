#include "main_window.h"

MainWindow::MainWindow() : //main_box{Gtk::ORIENTATION_VERTICAL}, 
draw_box{Gtk::ORIENTATION_VERTICAL}
{
    set_title("Reconnaissance 3D");
    set_default_size(1440, 810);

    /*add(main_box);
    main_box.pack_start(menubar, Gtk::PACK_SHRINK);

    menu_bar = {
        {"Fichier", "Ouvrir modèle 3D", "Charger scène", "Charger scan (PLY/LAS)",
        "Sauvegarder nuage de points", "Capturer image", "Quitter"},
        {"Scène", "Ajouter humain", "Ajouter objet", "Supprimer sélection", "Réinitialiser scène"},
        {"Scanner 3D", "Lancer scan", "Arrêter scan", "Paramètres scanner"},
        {"Simulation", "Visualiser rayons laser", "Visualiser intersections", "Coloration par distance"},
        {"Nuage de points", "Afficher/masquer nuage", "Filtrage bruit", "Ajuster nombre de points",
        "Color mapping", "Exporter"},
        {"IA", "Générer ensemble de données"},
        {"Affichage", "Vue caméra", "Vue scanner", "Plein écran"},
        {"Paramètres", "Anticrénelage", "Thème"},
        {"Aide", "Documentation", "À propos"}
    };

    for (const auto& menu : menu_bar)
    {
        std::vector<std::string> sub(menu.begin() + 1, menu.end());
        add_menu_item(menu[0], sub);
    }*/

    draw_box.pack_start(main_gl, Gtk::PACK_EXPAND_WIDGET);
    add(draw_box);

    show_all_children();
}

void MainWindow::add_menu_item(std::string menu_item, std::vector<std::string> sub_menu_items) 
{
    auto _menu_item = Gtk::make_managed<Gtk::MenuItem>(menu_item);
    auto sub_menu = Gtk::make_managed<Gtk::Menu>();

    for (int i = 0; static_cast<std::size_t>(i) < sub_menu_items.size(); ++i) {
        auto _sub_menu_item = Gtk::make_managed<Gtk::MenuItem>(sub_menu_items[i]);
        sub_menu->append(*_sub_menu_item);
        _sub_menu_item->show();
    }

    _menu_item->set_submenu(*sub_menu);
    menubar.append(*_menu_item);

    _menu_item->show();
    sub_menu->show();
}