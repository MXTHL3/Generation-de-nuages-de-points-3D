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

enum class ScanStrategyType { ThreeSixty, Timed, Multiple };

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
    void reset_scene(int start_index = 1);
    
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

    /// @brief Active ou désactive l'application du modèle de bruit sur les mesures lidar.
    /// @param v true pour ajouter du bruit simulé, false pour utiliser des mesures idéales.
    void set_apply_noise(bool v) { m_apply_noise = v; }

    /// @brief Indique si le modèle de bruit est appliqué aux mesures lidar.
    /// @return true si le bruit est activé, false sinon.
    bool get_apply_noise() const { return m_apply_noise; }

    /// @brief Définit la stratégie de scan utilisée pour la simulation lidar.
    /// @param s Type de stratégie à utiliser (360°, temporisée ou multiple).
    void set_scan_strategy(ScanStrategyType s) { m_scan_strategy = s; }

    /// @brief Retourne la stratégie de scan actuellement sélectionnée.
    /// @return Type de stratégie de scan active.
    ScanStrategyType get_scan_strategy() const { return m_scan_strategy; }

    /// @brief Définit la durée d'un scan temporisé.
    /// @param v Durée du scan en secondes.
    void set_scan_duration(double v) { m_scan_duration = v; }

    /// @brief Retourne la durée configurée pour un scan temporisé.
    /// @return Durée du scan en secondes.
    double get_scan_duration() const { return m_scan_duration; }

    /// @brief Définit le nombre de scans à effectuer avec la stratégie multiple.
    /// @param v Nombre de scans successifs à réaliser.
    void set_scan_n_scans(int v) { m_scan_n_scans = v; }

    /// @brief Retourne le nombre de scans configurés pour la stratégie multiple.
    /// @return Nombre de scans successifs.
    int get_scan_n_scans() const { return m_scan_n_scans; }

    /// @brief Définit le nombre de threads utilisés pour le lancer de rayons parallèle.
    /// @param v Nombre de threads à utiliser pour la simulation.
    void set_scan_threads(int v) { m_scan_threads = v; }

    /// @brief Retourne le nombre de threads utilisés pour la simulation lidar.
    /// @return Nombre de threads de calcul.
    int get_scan_threads() const { return m_scan_threads; }

private:
    Gtk::Overlay m_overlay; ///< Conteneur overlay racine (GLArea + widgets superposés).
    Gtk::GLArea gl_area; ///< Zone de rendu OpenGL.
    Gtk::Fixed m_fixed; ///< Conteneur fixe pour les marqueurs interactifs.

    GLuint vao = 0; ///< Vertex Array Object des maillages 3D.
    GLuint vbo = 0; ///< Vertex Buffer Object des maillages 3D.
    GLuint vao_cloud = 0; ///< VAO du nuage de points.
    GLuint vbo_cloud = 0; ///< VBO du nuage de points.
    int m_cloud_point_count = 0; ///< Nombre de points dans le VBO nuage courant.
    bool m_show_point_cloud = false; ///< true si le nuage de points est affiché.
    std::vector<float> m_cloud_data; ///< Données brutes du nuage (XYZ interleaved).

    GLuint vao_grid = 0; ///< VAO de la grille de référence au sol.
    GLuint vbo_grid = 0; ///< VBO de la grille de référence au sol.
    GLsizei grid_vertex_count = 0; ///< Nombre de sommets de la grille.

    std::string m_lidar_config = "lidars_config/ouster_os1_64.json"; ///< Chemin JSON du capteur actif.
    GLuint shader_program = 0; ///< Handle du programme GLSL compilé.
    std::vector<float> vertex_data; ///< Données de tous les maillages concaténées (XYZ par triangle).

    std::vector<std::unique_ptr<Cgal>> m_scenes; ///< Maillages CGAL de la scène (index 0 = cube par défaut).
    std::vector<std::string> m_model_paths; ///< Chemins des fichiers 3D chargés (même ordre que m_scenes).
    std::vector<std::unique_ptr<ModelMarker>> m_markers; ///< Marqueurs de transformation interactifs.
    std::vector<ModelTransform> m_transforms; ///< Transformations appliquées à chaque modèle.
    std::shared_ptr<LidarConfig> m_lidar_override; ///< Config lidar de remplacement (nullptr = utiliser m_lidar_config).
    std::vector<bool> m_model_hidden; ///< Masquage par modèle (même index que m_scenes).

    int m_load_count = 0; ///< Nombre de modèles chargés depuis le début de la session (sert au calcul d'offset).
    float angle_x = 0.0f; ///< Angle de tangage (pitch) de la caméra orbitale (radians).
    float angle_y = 0.0f; ///< Angle de lacet (yaw) de la caméra orbitale (radians).
    bool m_dragging = false; ///< true pendant un drag caméra actif.
    double m_last_x = 0.0; ///< Dernière position X de la souris lors du drag caméra.
    double m_last_y = 0.0; ///< Dernière position Y de la souris lors du drag caméra.
    float m_zoom = 5.0f; ///< Distance de la caméra à l'origine (zoom orbital).

    bool m_apply_noise = true; ///< Active le modèle de bruit lors du scan.
    ScanStrategyType m_scan_strategy = ScanStrategyType::ThreeSixty; ///< Stratégie de scan active.
    double m_scan_duration = 1.0; ///< Durée du scan en secondes (utilisée par ScanTimed).
    int m_scan_n_scans = 3; ///< Nombre de scans (utilisé par ScanMultiple).
    int m_scan_threads = 4; ///< Nombre de threads pour le lancer de rayons parallèle.

    /// @brief Reconstruit vertex_data à partir de tous les maillages de m_scenes.
    void rebuild_vertex_data();

    /// @brief Uploade vertex_data dans le VBO OpenGL.
    void upload_vertex_data();

    /// @brief Callback GTK appelé à l'initialisation du contexte OpenGL.
    void on_realize();

    /// @brief Callback GTK appelé à chaque frame de rendu.
    /// @param context Contexte OpenGL GTK courant.
    /// @return true pour indiquer que le rendu a été pris en charge.
    bool on_render(const Glib::RefPtr<Gdk::GLContext>& context);

    /// @brief Callback GTK appelé à la destruction du contexte OpenGL.
    void on_unrealize();

    /// @brief Callback GTK appelé lors d'un appui sur un bouton de la souris.
    bool on_button_press(GdkEventButton* e);

    /// @brief Callback GTK appelé lors du relâchement d'un bouton de la souris.
    bool on_button_release(GdkEventButton* e);

    /// @brief Callback GTK appelé lors d'un mouvement de la souris.
    bool on_motion(GdkEventMotion* e);

    /// @brief Callback GTK appelé lors d'un appui sur une touche clavier.
    bool on_key_press(GdkEventKey* e);

    /// @brief Recalcule et met à jour la position écran de tous les marqueurs.
    void update_markers_positions();

    /// @brief Redonne le focus clavier à la GLArea.
    void focus_gl_area();

    /// @brief Projette un point 3D monde vers les coordonnées écran 2D.
    /// @param point_3d Point en coordonnées monde.
    /// @return Paire (x, y) en pixels dans le widget.
    std::pair<double, double> project_to_2d(const glm::vec3& point_3d);

    /// @brief Projette un point local 3D d'un modèle vers les coordonnées écran 2D.
    /// @param local_pos Position locale du point (coordonnées objet).
    /// @param tr Transformation du modèle.
    /// @return Paire (x, y) en pixels dans le widget.
    std::pair<double, double> project_to_2d(const glm::vec3& local_pos, const ModelTransform& tr);

    /// @brief Construit la matrice modèle GLM à partir d'une ModelTransform.
    /// @param tr Transformation source.
    /// @return Matrice 4x4 TRS (Translation × Rotation × Scale).
    glm::mat4 make_model_matrix(const ModelTransform& tr) const;

    /// @brief Crée et ajoute le marqueur central pour un modèle donné.
    /// @param model_index Index du modèle dans m_scenes.
    void add_center_marker(int model_index);

    /// @brief Callback Cairo de dessin du conteneur fixe (traits entre marqueurs).
    bool on_fixed_draw(const Cairo::RefPtr<Cairo::Context>& cr);

    /// @brief Callback appelé lors d'un drag sur un marqueur.
    /// @param type Type du marqueur dragué.
    /// @param model_index Index du modèle concerné.
    /// @param dx Déplacement horizontal en pixels.
    /// @param dy Déplacement vertical en pixels.
    void on_marker_dragged(MarkerType type, int model_index, double dx, double dy);

    /// @brief Connecte les signaux GTK d'un marqueur aux callbacks de Gl.
    /// @param marker Marqueur à connecter.
    void connect_marker_signals(ModelMarker* marker);
};