#pragma once

#include <gtkmm.h>
#include <iostream>
#include <vector>
#include <string>

/// @brief Gestion de l'ouverture de fichiers 3D via une boîte de dialogue GTK.
/// Conserve la liste des chemins des modèles chargés dans la session.
class HandleFile {
public:
    /// @brief Ouvre une boîte de dialogue de sélection de fichier 3D.
    /// Ajoute le chemin choisi au cache interne et appelle le callback.
    /// @param parent Fenêtre parente pour la boîte de dialogue modale.
    /// @param on_loaded Callback appelé avec le chemin du fichier sélectionné.
    void open_model(Gtk::Window& parent, std::function<void(const std::string&)> on_loaded);
    
    /// @brief Retourne la liste des chemins des modèles ouverts depuis le début de la session.
    /// @return Référence constante vers le vecteur de chemins.
    const std::vector<std::string>& loaded_models() const { return m_loaded_models; }
private:
    std::vector<std::string> m_loaded_models; ///< Chemins des fichiers 3D chargés.
};