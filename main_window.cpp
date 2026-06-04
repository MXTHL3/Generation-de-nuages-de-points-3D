#include "main_window.h"

MainWindow::MainWindow(std::unique_ptr<Gl> gl) : main_box(Gtk::ORIENTATION_VERTICAL) {
    m_gl = std::move(gl);
    
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

    Gtk::Box* lidar_bar = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL, 6);
    lidar_bar->set_margin_start(8);
    lidar_bar->set_margin_end(8);
    lidar_bar->set_margin_top(2);
    lidar_bar->set_margin_bottom(2);

    auto* lidar_label = Gtk::make_managed<Gtk::Label>("Modèle LiDAR :");
    m_lidar_combo.append("lidars_config/ouster_os1_64.json", "Ouster OS1-64");
    m_lidar_combo.append("lidars_config/ouster_os2_128.json", "Ouster OS2-128");
    m_lidar_combo.append("lidars_config/vedolyne_vlp16.json", "Velodyne VLP-16");
    m_lidar_combo.append("lidars_config/velodyne_vlp32c.json", "Velodyne VLP-32C");
    m_lidar_combo.set_active(0);

    m_lidar_combo.signal_changed().connect([this]() {
        std::string chosen = m_lidar_combo.get_active_id();
        if (!chosen.empty()) m_gl->set_lidar_config(chosen);
    });

    lidar_bar->pack_start(*lidar_label, Gtk::PACK_SHRINK);
    lidar_bar->pack_start(m_lidar_combo, Gtk::PACK_SHRINK);
    main_box.pack_start(*lidar_bar, Gtk::PACK_SHRINK);
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
            "Capturer image", "Quitter"},
        {"Scène",
            "Supprimer sélection", "Réinitialiser scène"},
        {"Scanner 3D",
            "Lancer scan", "Paramètres scanner"},
        {"Simulation",
            "Visualiser rayons laser", "Visualiser intersections",
            "Coloration par distance"},
        {"Nuage de points",
            "Afficher/masquer nuage", "Filtrage bruit",
            "Ajuster nombre de points", "Color mapping"},
        {"IA",
            "Générer ensemble de données", "Générer ensemble de données (à partir d'un JSON)", 
            "Lancer analyse reconnaissance"},
        {"Affichage",
            "Plein écran"},
        {"Aide",
            "Documentation"}
    };

    build_menubar();
    show_all_children();
}

void MainWindow::update_markers()
{
    if (!m_gl) return;
    const std::string& mode = transformation_mode[m_tm_id];
    for (auto& mm : m_gl->get_markers()) {
        MarkerType g = mm->get_marker_type();
        if (mode.empty()) {
            if (g == MarkerType::tx || g == MarkerType::rx) mm->set_marker_type(MarkerType::unabledx);
            else if (g == MarkerType::ty || g == MarkerType::ry) mm->set_marker_type(MarkerType::unabledy);
            else if (g == MarkerType::tz || g == MarkerType::rz) mm->set_marker_type(MarkerType::unabledz);
            else if (g == MarkerType::s) mm->set_marker_type(MarkerType::unableds);
        } else if (mode == "Translation mode") {
            if (g == MarkerType::unabledx || g == MarkerType::rx) mm->set_marker_type(MarkerType::tx);
            else if (g == MarkerType::unabledy || g == MarkerType::ry) mm->set_marker_type(MarkerType::ty);
            else if (g == MarkerType::unabledz || g == MarkerType::rz) mm->set_marker_type(MarkerType::tz);
            else if (g == MarkerType::s) mm->set_marker_type(MarkerType::unableds);
        } else if (mode == "Rotation mode") {
            if (g == MarkerType::tx || g == MarkerType::unabledx) mm->set_marker_type(MarkerType::rx);
            else if (g == MarkerType::ty || g == MarkerType::unabledy) mm->set_marker_type(MarkerType::ry);
            else if (g == MarkerType::tz || g == MarkerType::unabledz) mm->set_marker_type(MarkerType::rz);
            else if (g == MarkerType::s) mm->set_marker_type(MarkerType::unableds);
        } else if (mode == "Scale mode") {
            if (g == MarkerType::tx || g == MarkerType::rx || g == MarkerType::unabledx) mm->set_marker_type(MarkerType::unabledx);
            else if (g == MarkerType::ty || g == MarkerType::ry || g == MarkerType::unabledy) mm->set_marker_type(MarkerType::unabledy);
            else if (g == MarkerType::tz || g == MarkerType::rz || g == MarkerType::unabledz) mm->set_marker_type(MarkerType::unabledz);
            else if (g == MarkerType::unableds) mm->set_marker_type(MarkerType::s);
        }
    }
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
    update_markers();
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
    auto sub_menu = Gtk::make_managed<Gtk::Menu>();

    for (const auto& label : sub_menu_items) {
        auto _sub_widget = Gtk::make_managed<Gtk::MenuItem>(label);
        _sub = _sub_widget;   

        if (label == "Ouvrir modèle 3D") { open_3d_model(); }
        else if (label == "Charger scan (PLY/LAS)") { load_scan(); }
        else if (label == "Capturer image") { capture_image(); }
        else if (label == "Lancer scan") { launch_scan(); }
        else if (label == "Afficher/masquer nuage") { display_cloud(); }
        else if (label == "Générer ensemble de données") { generate_dataset(); }
        else if (label == "Générer ensemble de données (à partir d'un JSON)") { generate_dataset_from_json(); }
        else if (label == "Lancer analyse reconnaissance") { launch_recognition(); }
        else if (label == "Plein écran") { to_fullscreen(); }
        else if (label == "Quitter") { exit_app(); }

        sub_menu->append(*_sub_widget);
        _sub_widget->show();
    }

    _menu_item->set_submenu(*sub_menu);
    menubar.append(*_menu_item);
    _menu_item->show();
    sub_menu->show();
}