#include "main_window.h"

MainWindow::MainWindow(std::unique_ptr<Gl> gl)
    : main_box(Gtk::ORIENTATION_VERTICAL)
    , m_gl(std::move(gl))
{
    set_title("Reconnaissance 3D");
    set_default_size(1440, 810);

    m_status_label.set_use_markup(true);
    update_status_label();
    m_status_label.set_halign(Gtk::ALIGN_END);
    m_status_label.set_valign(Gtk::ALIGN_END);
    m_status_label.set_margin_end(12);
    m_status_label.set_margin_bottom(8);

    m_gl->add_overlay_widget(m_status_label);

    add(main_box);
    main_box.pack_start(menubar, Gtk::PACK_SHRINK);
    main_box.pack_start(m_gl->widget(), Gtk::PACK_EXPAND_WIDGET);

    m_gl->signal_marker_clicked.connect([this](MarkerType type) {
        if (type == MarkerType::center) {
            m_tm_id = (m_tm_id + 1) % static_cast<int>(transformation_mode.size());
            update_status_label();
        }
    });

    menu_bar_data = {
        {"Fichier",
            "Ouvrir modèle 3D", "Charger scène", "Charger scan (PLY/LAS)",
            "Sauvegarder nuage de points", "Capturer image", "Quitter"},
        {"Scène",
            "Ajouter humain", "Ajouter objet",
            "Supprimer sélection", "Réinitialiser scène"},
        {"Scanner 3D",
            "Lancer scan", "Arrêter scan", "Paramètres scanner"},
        {"Simulation",
            "Visualiser rayons laser", "Visualiser intersections",
            "Coloration par distance"},
        {"Nuage de points",
            "Afficher/masquer nuage", "Filtrage bruit",
            "Ajuster nombre de points", "Color mapping", "Exporter"},
        {"IA",
            "Générer ensemble de données"},
        {"Affichage",
            "Vue caméra", "Vue scanner", "Plein écran"},
        {"Paramètres",
            "Anticrénelage", "Thème"},
        {"Aide",
            "Documentation", "À propos"}
    };

    build_menubar();
    show_all_children();
}

void MainWindow::update_status_label()
{
    const std::string& mode = transformation_mode[m_tm_id];
    if (mode.empty()) {
        m_status_label.set_markup("");
    } else {
        m_status_label.set_markup(
            "<span foreground='red' font='12'>" + mode + "</span>"
        );
    }
}

void MainWindow::build_menubar()
{
    for (const auto& item : menu_bar_data) {
        std::vector<std::string> sub(item.begin() + 1, item.end());
        add_menu_item(item[0], sub);
    }
}

void MainWindow::add_menu_item(const std::string& menu_item,
                               const std::vector<std::string>& sub_menu_items)
{
    auto _menu_item = Gtk::make_managed<Gtk::MenuItem>(menu_item);
    auto sub_menu   = Gtk::make_managed<Gtk::Menu>();

    for (const auto& label : sub_menu_items) {
        auto _sub = Gtk::make_managed<Gtk::MenuItem>(label);

        if (label == "Ouvrir modèle 3D") {
            _sub->signal_activate().connect([this]() {
                m_handle_file.open_model(*this, [this](const std::string& path) {
                    m_gl->load_file(path);
                });
            });
        }

        sub_menu->append(*_sub);
        _sub->show();
    }

    _menu_item->set_submenu(*sub_menu);
    menubar.append(*_menu_item);
    _menu_item->show();
    sub_menu->show();
}