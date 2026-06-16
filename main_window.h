#pragma once

#include <gtkmm.h>
#include <vector>
#include <string>
#include <memory>
#include <cstdio>     
#include <cstdlib>    
#include <sstream>    
#include "gl.h"
#include "model_marker.h"
#include "menu_items_actions.h"

/// @brief Fenêtre principale de l'application.
/// Intègre la vue 3D OpenGL (Gl), la barre de menus et le label d'état
/// indiquant le mode de transformation actif (translation / rotation / échelle).
class MainWindow : public Gtk::Window, public MenuItemsActions {
public:
    /// @brief Construit la fenêtre et initialise tous les widgets.
    /// @param gl Contexte OpenGL à intégrer (prend la propriété).
    explicit MainWindow(std::unique_ptr<Gl> gl);

    /// @brief Ajoute un élément de menu avec ses sous-éléments et connecte les actions.
    /// @param menu_item       Libellé du menu parent.
    /// @param sub_menu_items  Libellés des entrées du sous-menu.
    void add_menu_item(const std::string& menu_item,
                       const std::vector<std::string>& sub_menu_items);

private:
    Gtk::Box main_box; ///< Conteneur vertical principal.
    Gtk::MenuBar menubar; ///< Barre de menus.
    Gtk::Label m_status_label; ///< Label affichant le mode de transformation courant.
    Gtk::ComboBoxText m_lidar_combo; ///< Combo de sélection du modèle lidar (réservé).
    std::vector<std::vector<std::string>> menu_bar_data; ///< Données de construction de la barre de menus.

    /// @brief Modes de transformation cyclables par clic sur le marqueur central.
    const std::vector<std::string> transformation_mode = {
        "", "Mode translation", "Mode rotation", "Mode échelle"
    };
    int m_tm_id = 0; ///< Index du mode de transformation actif.  

    /// @brief Retourne la fenêtre GTK parente (requis par MenuItemsActions).
    Gtk::Window& as_window() override { return *this; }

    /// @brief Met à jour le type des marqueurs selon le mode de transformation actif.
    void update_markers();

    /// @brief Rafraîchit le label d'état et les marqueurs.
    void update_status_label();

    /// @brief Construit la barre de menus à partir de menu_bar_data.
    void build_menubar();
};