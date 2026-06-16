#pragma once
#include <gtkmm/drawingarea.h>
#include <gtkmm/fixed.h>
#include <glm/glm.hpp>
#include <sigc++/sigc++.h>

/// @brief Types de marqueurs de transformation interactifs.
/// Les valeurs prefixées par "unabled" indiquent un marqueur désactivé/invisible.
enum class MarkerType {
    unabledx, unabledy, unabledz, unableds, ///< Marqueurs désactivés (cachés).
    center, ///< Marqueur central (clic = cycle de mode).
    tx, ty, tz, ///< Marqueurs de translation sur X, Y, Z.
    rx, ry, rz, ///< Marqueurs de rotation autour de X, Y, Z.
    s ///< Marqueur d'échelle uniforme.
};

/// @brief Marqueur de transformation interactif affiché en overlay sur la GLArea.
/// Dessiné en Cairo, il émet des signaux lors d'un clic ou d'un drag
/// pour piloter la transformation du modèle 3D associé.
class ModelMarker : public Gtk::DrawingArea {
public:
    /// @brief Construit un marqueur et l'attache au conteneur fixe parent.
    /// @param parent Conteneur GTK fixe dans lequel le marqueur est positionné.
    /// @param local_pos Position locale 3D du marqueur (en coordonnées objet).
    /// @param zoom Zoom courant de la caméra (utilisé à la création).
    /// @param type Type du marqueur (translation, rotation, échelle, centre…).
    /// @param color Couleur RGB du marqueur.
    /// @param model_index Index du modèle 3D auquel ce marqueur est rattaché.
    ModelMarker(Gtk::Fixed* parent, const glm::vec3& local_pos, float zoom, MarkerType type,
                const glm::vec3& color, int model_index = 0);
        
    /// @brief Déplace le marqueur à une position écran donnée.
    /// @param x Coordonnée X en pixels dans le widget.
    /// @param y Coordonnée Y en pixels dans le widget.            
    void set_position(double x, double y);

    /// @brief Retourne le type courant du marqueur.
    MarkerType get_marker_type() const { return m_type; };

    /// @brief Remplace le type du marqueur et met à jour sa visibilité.
    /// @param mt Nouveau type à appliquer.
    void set_marker_type(MarkerType mt);

    /// @brief Retourne la position locale 3D du marqueur (coordonnées objet).
    const glm::vec3& local_position() const { return m_local_pos; }

    /// @brief Retourne l'index du modèle 3D auquel ce marqueur est rattaché.
    int model_index() const { return m_model_index; }

    /// @brief Associe un marqueur central à ce marqueur (pour le dessin du trait).
    /// @param center Pointeur vers le marqueur central du même modèle.
    void set_center_marker(ModelMarker* center) { m_center_marker = center; }

    /// @brief Retourne le marqueur central associé.
    /// @return Pointeur vers le marqueur central, nullptr si non défini.
    ModelMarker* get_center_marker() const { return m_center_marker; }
 
    /// @brief Retourne la coordonnée X écran courante du marqueur (pixels).
    double screen_x() const { return m_screen_x; }

    /// @brief Retourne la coordonnée Y écran courante du marqueur (pixels).
    double screen_y() const { return m_screen_y; }

    /// @brief Signal émis lors d'un clic sur le marqueur.
    sigc::signal<void(MarkerType)> signal_clicked;

    /// @brief Signal émis pour redonner le focus clavier à la GLArea.
    sigc::signal<void()> signal_request_focus;

    /// @brief Signal émis lors d'un drag, avec le déplacement en pixels.
    /// @details Paramètres : type du marqueur, index du modèle, dx, dy.
    sigc::signal<void(MarkerType, int, double, double)> signal_dragged;

private:
    Gtk::Fixed* m_parent_fixed; ///< Conteneur fixe parent.
    ModelMarker* m_center_marker = nullptr; ///< Marqueur central associé.
    MarkerType m_type; ///< Type courant du marqueur.
    glm::vec3 m_color; ///< Couleur RGB d'affichage.
    glm::vec3 m_local_pos; ///< Position locale 3D (coordonnées objet).
    int m_model_index  = 0; ///< Index du modèle rattaché.
    bool m_hovered = false; ///< true si la souris survole le marqueur.
    bool m_hidden = false; ///< true si le marqueur est désactivé/invisible.
    bool m_dragging = false; ///< true pendant un drag actif.
    double m_drag_last_x = 0.0; ///< Dernière position X de la souris lors du drag.
    double m_drag_last_y = 0.0; ///< Dernière position Y de la souris lors du drag.
    double m_screen_x = 0.0; ///< Position X écran courante (pixels).
    double m_screen_y = 0.0; ///< Position Y écran courante (pixels).

    bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr) override;
    bool on_enter_notify_event(GdkEventCrossing* event) override;
    bool on_leave_notify_event(GdkEventCrossing* event) override;
    bool on_button_press_event(GdkEventButton* event) override;
    bool on_button_release_event(GdkEventButton* event) override;
    bool on_motion_notify_event(GdkEventMotion* event) override;

    /// @brief Recalcule et applique la visibilité selon le type courant.
    void refresh_visibility();
};