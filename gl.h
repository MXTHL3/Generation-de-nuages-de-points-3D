#pragma once

#include <gtkmm.h>
#include <epoxy/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>
#include <memory>
#include <filesystem>
#include <algorithm>
#include <fstream>
#include <pdal/io/LasReader.hpp>
#include <png.h>
#include <bits/stdc++.h>
#include "cgal.h"
#include "model_marker.h"
#include "scene.h"
#include "entity.h"
#include "lidar_factory.h"
#include "point_cloud_exporter.h"
#include "logger.h"
#include "lidar_scanner.h"

/// @brief Transformation affine d'un modèle 3D dans la scène OpenGL.
/// Regroupe translation, rotation (Euler XYZ, radians) et échelle uniforme.
/// Le flag use_offset active un décalage automatique pour éviter la superposition
/// des modèles chargés successivement.
struct ModelTransform {
    float pos_x = 0.0f, pos_y = 0.0f, pos_z = 0.0f; ///< Translation
    float angle_x = 0.0f, angle_y = 0.0f, angle_z = 0.0f; ///< Rotation
    float scale = 1.0f; ///< Facteur d'échelle
    bool use_offset = true; ///< Active le décalage automatique entre modèles (OFFSET_STEP).
};

#include "cgal_glm_utils.h"

/// @brief Moteur de rendu OpenGL/GTK : scène 3D, caméra, scan lidar et nuage de points.
/// Gère les VAO/VBO, les shaders, les marqueurs de transformation interactifs
/// et l'interface avec le scanner lidar (run_scan).
class Gl {
public:
    static constexpr float OFFSET_STEP = 1.5f; ///< Décalage entre modèles superposés.
    static constexpr float DRAG_SENSITIVITY = 0.02f; ///< Sensibilité du drag des marqueurs.

    /// @brief Construit le contexte OpenGL avec une scène CGAL par défaut (cube).
    /// @param default_scene Scène CGAL initiale (prend la propriété).
    explicit Gl(std::unique_ptr<Cgal> default_scene);

    /// @brief Retourne le widget GTK racine à intégrer dans la fenêtre.
    /// @return Référence vers l'Overlay contenant la GLArea.
    Gtk::Widget& widget() { return m_overlay; }

    /// @brief Construit le maillage de la grille de référence au sol.
    /// @param size Taille totale de la grille (en mètres).
    /// @param step Espacement entre les lignes (en mètres).
    void build_grid(float size, float step);

    /// @brief Charge un fichier 3D et l'ajoute à la scène.
    /// @param path Chemin vers le fichier (OBJ, PLY, OFF, STL).
    void load_file(const std::string& path);

    /// @brief Charge un nuage de points et l'affiche.
    /// @param path Chemin vers le fichier (PLY, LAS ou LAZ).
    void load_scan(const std::string& path);

    /// @brief Applique une transformation à un modèle de la scène.
    /// @param idx Index du modèle (0 = cube par défaut).
    /// @param tr  Transformation à appliquer.
    void set_transform(int idx, const ModelTransform& tr);

    /// @brief Positionne la caméra dans la scène.
    /// @param pos_x Coordonnée X de la caméra.
    /// @param pos_y Coordonnée Y de la caméra.
    /// @param pos_z Coordonnée Z de la caméra.
    /// @param rx Angle de tangage (pitch) en radians.
    /// @param ry Angle de lacet (yaw) en radians.
    /// @param zoom Distance de la caméra à l'origine.
    void set_camera(float pos_x, float pos_y, float pos_z, float rx, float ry, float zoom);
    
    /// @brief Capture le rendu OpenGL courant et l'enregistre en PNG.
    /// @param path Chemin du fichier image de sortie.
    void capture_image(const std::string& path);

    /// @brief Ajoute un widget GTK en overlay sur la GLArea.
    /// @param w Widget à superposer.
    void add_overlay_widget(Gtk::Widget& w);

    /// @brief Lance un scan lidar de la scène complète et exporte le nuage de points.
    /// @param lidar_config_path Chemin vers le fichier JSON de configuration lidar.
    /// @param output_path Chemin du fichier de sortie (PLY ou LAS).
    void run_scan(const std::string& lidar_config_path, const std::string& output_path);

    /// @brief Définit le chemin du fichier de configuration lidar actif.
    /// @param path Chemin vers le fichier JSON de configuration.
    void set_lidar_config(const std::string& path) { m_lidar_config = path; }

    /// @brief Remplace la configuration lidar par une instance déjà créée.
    /// @param lidar Configuration à utiliser à la place du fichier JSON.
    void set_lidar_override(std::shared_ptr<LidarConfig> lidar) { m_lidar_override = std::move(lidar); }
    
    /// @brief Supprime la configuration lidar de remplacement (retour au fichier JSON).
    void clear_lidar_override() { m_lidar_override.reset(); }
    
    /// @brief Retourne la configuration lidar de remplacement courante.
    /// @return Pointeur partagé vers la configuration, nullptr si aucune.
    std::shared_ptr<LidarConfig> get_lidar_override() const { return m_lidar_override; }
    
    /// @brief Modifie la distance minimale de détection du lidar de remplacement.
    /// @param v Nouvelle distance minimale (mètres).
    void lidar_override_set_min(double v);
    
    /// @brief Modifie la distance maximale de détection du lidar de remplacement.
    /// @param v Nouvelle distance maximale (mètres).
    void lidar_override_set_max(double v);
    
    /// @brief Modifie la résolution horizontale du lidar mécanique de remplacement.
    /// @param v Pas azimutal en degrés.
    void lidar_override_set_hstep(size_t /*idx*/, double v);
    
    /// @brief Modifie la précision du lidar de remplacement.
    /// @param v Nouvelle précision (mètres).
    void lidar_override_set_accuracy(double v);
    
    /// @brief Retourne le chemin du fichier de configuration lidar actif.
    /// @return Chemin vers le fichier JSON de configuration.
    const std::string& get_lidar_config() const { return m_lidar_config; }
    
    /// @brief Bascule l'affichage du nuage de points.
    void toggle_point_cloud() { m_show_point_cloud = !m_show_point_cloud; gl_area.queue_render(); }
    
    /// @brief Retourne la liste des marqueurs de transformation interactifs.
    /// @return Référence vers le vecteur de marqueurs.
    std::vector<std::unique_ptr<ModelMarker>>& get_markers() { return m_markers; };
    
    /// @brief Supprime tous les modèles chargés (conserve le cube par défaut).
    void reset_scene();
    
    /// @brief Supprime tous les nuages de points chargés.
    void reset_scans();
    
    /// @brief Signal émis quand l'utilisateur clique sur un marqueur.
    sigc::signal<void(MarkerType)> signal_marker_clicked;
    
    /// @brief Retourne le nombre de modèles présents dans la scène.
    /// @return Nombre de scènes CGAL (cube inclus).
    size_t model_count() const { return m_scenes.size(); }
    
    /// @brief Masque ou affiche un modèle de la scène.
    /// @param idx Index du modèle.
    /// @param hidden true pour masquer, false pour afficher.
    void set_model_hidden(size_t idx, bool hidden);
    
    /// @brief Indique si un modèle est actuellement masqué.
    /// @param idx Index du modèle.
    /// @return true si le modèle est masqué.
    bool is_model_hidden(size_t idx) const {
        return idx < m_model_hidden.size() ? m_model_hidden[idx] : false;
    }
    
    /// @brief Retourne l'angle de tangage (pitch) courant de la caméra.
    float get_angle_x() const { return angle_x; }
    
    /// @brief Retourne l'angle de lacet (yaw) courant de la caméra.
    float get_angle_y() const { return angle_y; }
    
    /// @brief Retourne les chemins des fichiers 3D chargés.
    const std::vector<std::string>& model_paths() const { return m_model_paths; }
    
    /// @brief Retourne les transformations appliquées à chaque modèle.
    const std::vector<ModelTransform>& transforms() const { return m_transforms; }
    
    /// @brief Retourne la position de la caméra dans le repère monde.
    /// @return Vecteur 3D de la position de la caméra.
    glm::vec3 get_camera_world_position() const;

private:
    Gtk::Overlay m_overlay;
    Gtk::GLArea gl_area;
    Gtk::Fixed m_fixed;
    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint vao_cloud = 0;
    GLuint vbo_cloud = 0;
    int m_cloud_point_count = 0;
    bool m_show_point_cloud  = false;
    std::vector<float> m_cloud_data;
    GLuint vao_grid = 0;
    GLuint vbo_grid = 0;
    GLsizei grid_vertex_count = 0;
    std::string m_lidar_config = "lidars_config/ouster_os1_64.json";
    GLuint shader_program = 0;
    std::vector<float> vertex_data;
    std::vector<std::unique_ptr<Cgal>> m_scenes;
    std::vector<std::string> m_model_paths;
    std::vector<std::unique_ptr<ModelMarker>> m_markers;
    std::vector<ModelTransform> m_transforms;
    std::shared_ptr<LidarConfig> m_lidar_override;  
    std::vector<bool> m_model_hidden; 

    int m_load_count = 0;
    float angle_x = 0.0f;
    float angle_y = 0.0f;
    bool m_dragging = false;
    double m_last_x = 0.0;
    double m_last_y = 0.0;
    float m_zoom = 5.0f;

    void rebuild_vertex_data();
    void upload_vertex_data();
    void on_realize();
    bool on_render(const Glib::RefPtr<Gdk::GLContext>& context);
    void on_unrealize();
    bool on_button_press(GdkEventButton* e);
    bool on_button_release(GdkEventButton* e);
    bool on_motion(GdkEventMotion* e);
    bool on_key_press(GdkEventKey* e);
    void update_markers_positions();
    void focus_gl_area();
    std::pair<double, double> project_to_2d(const glm::vec3& point_3d);
    std::pair<double, double> project_to_2d(const glm::vec3& local_pos, const ModelTransform& tr);
    glm::mat4 make_model_matrix(const ModelTransform& tr) const;
    void add_center_marker(int model_index);
    bool on_fixed_draw(const Cairo::RefPtr<Cairo::Context>& cr);
    void on_marker_dragged(MarkerType type, int model_index, double dx, double dy);
    void connect_marker_signals(ModelMarker* marker);
};