#pragma once

#include <gtkmm.h>
#include <memory>
#include <filesystem>
#include <cstdlib>
#include <thread>
#include <iostream>
#include "menu_items_actions.h"
#include "gl.h"
#include "handle_file.h"
#include "pipeline.h"
#include "asset_manager.h"
#include "scene_utils.h"
#include "noise_model.h"
#include "point_cloud_exporter.h"
#include "lidar_scanner.h"      
#include "logger.h"

/// @brief Classe de base regroupant les actions des entrées de la barre de menus.
/// Chaque méthode connecte le signal activate d'un sous-menu (_sub)
/// à l'action correspondante. MainWindow hérite de cette classe.
class MenuItemsActions {
protected:
    Gtk::MenuItem* _sub = nullptr; ///< Pointeur vers le sous-menu en cours de configuration.  
    std::unique_ptr<Gl> m_gl; ///< Contexte OpenGL partagé avec la fenêtre.
    HandleFile m_handle_file; ///< Gestionnaire d'ouverture de fichiers 3D.

    /// @brief Retourne la fenêtre GTK parente (implémenté par MainWindow).
    virtual Gtk::Window& as_window() = 0;

    /// @brief Met à jour les marqueurs de transformation (implémenté par MainWindow).
    virtual void update_markers() = 0;

    /// @brief Ouvre un fichier 3D et le charge dans la scène.
    void open_3d_model();

    /// @brief Ouvre un fichier de nuage de points (PLY, LAS, LAZ) et l'affiche.
    void load_scan();

    /// @brief Charge une scène complète depuis un fichier JSON.
    void load_json_scene();

    /// @brief Exporte la scène courante vers un fichier JSON.
    void save_json_scene();

    /// @brief Capture le rendu OpenGL et l'enregistre en PNG.
    void capture_image();

    /// @brief Lance un scan lidar de la scène complète et exporte le nuage.
    void launch_full_scene_scan();

    /// @brief Lance un scan lidar individuel pour chaque modèle 3D de la scène.
    void launch_one_scan_per_3d_model();

    /// @brief Bascule l'affichage du nuage de points.
    void display_cloud();

    /// @brief Ajoute des scans à un dataset et propose l'entraînement de l'IA.
    void generate_dataset();

    /// @brief Lance l'analyse de reconnaissance de silhouettes sur un fichier PLY.
    void launch_recognition();

    /// @brief Bascule le mode plein écran de la fenêtre.
    void to_fullscreen();

    /// @brief Ouvre la documentation HTML dans le navigateur.
    void open_docs();

    /// @brief Supprime tous les modèles chargés (conserve le cube par défaut).
    void reset_scene();

    /// @brief Supprime tous les nuages de points chargés.
    void reset_scans();

    /// @brief Ouvre la boîte de dialogue des paramètres du scanner lidar.
    void scanner_settings();

    /// @brief Génère des variations de position et rotation pour chaque modèle 3D chargé (hors cube par défaut) via le pipeline
    void vary_poses();

    /// @brief Ferme la fenêtre principale et quitte l'application.
    void exit_app();
};