#pragma once

#include "pose.hpp"

#include <pdal/PointTable.hpp>
#include <pdal/PointView.hpp>
#include <pdal/io/LasWriter.hpp>
#include <pdal/Options.hpp>
#include <pdal/io/BufferReader.hpp>

#include <vector>
#include <string>

class IPointCloudExporter {
public:
    virtual ~IPointCloudExporter() {};
    virtual void save(const std::string& filename, const std::vector<Point3>& points) = 0;
};

class PlyExporter : public IPointCloudExporter {
public:
    void save(const std::string& filename, const std::vector<Point3>& points) override;
};

/// @brief 
class LazBatchExporter{
public:
    /// @param filename Chemin fichier qui sera sauvegarder
    LazBatchExporter(const std::string& filename);

    /// @brief Ajoute un nuage dans le fichier ouvert
    /// @param points Nuage de point à sauvegarder
    void add_point_cloud(const std::vector<Point3>& points);

    /// @brief Sauvegarde le batch LAZ.
    bool save_file();

    /// @brief Retourne vrai si le batch doit être écrit. 
    bool is_full();

    /// @brief Enregistre le batch courant et créé le nouveau
    bool next_batch();

    /// @brief Retourne le nom du batch courant
    std::string current_filename() {
        std::stringstream s;
        s << m_filename << m_num_batch;
        return s.str();
    }

private:
    std::string m_filename; ///< Nom basique du fichier
    
    size_t m_num_batch; ///< Index du batch qu'on est en train d'écrire
    size_t m_point_source_index; ///< Index du prochain PointSourceID à écrire dans le fichier

    std::unique_ptr<pdal::PointTable> m_table;
    std::shared_ptr<pdal::PointView> m_view;
};

