#pragma once

#include "pose.h"
#include "logger.h"

#include <pdal/PointTable.hpp>
#include <pdal/PointView.hpp>
#include <pdal/io/LasWriter.hpp>
#include <pdal/Options.hpp>
#include <pdal/io/BufferReader.hpp>
#include <vector>
#include <string>
#include <fstream>
#include <iomanip>

/// @brief Interface d'export de nuage de points vers un fichier.
class IPointCloudExporter {
public:
    virtual ~IPointCloudExporter() {};

    /// @brief Sauvegarde un nuage de points dans un fichier.
    /// @param filename Chemin du fichier de sortie.
    /// @param points   Nuage de points à exporter.
    virtual void save(const std::string& filename, const std::vector<Point3>& points) = 0;
};

/// @brief Export au format PLY ASCII.
class PlyExporter : public IPointCloudExporter {
public:
    /// @brief Sauvegarde le nuage au format PLY ASCII (header + lignes X Y Z).
    /// @param filename Chemin du fichier .ply de sortie.
    /// @param points Nuage de points à exporter.
    void save(const std::string& filename, const std::vector<Point3>& points) override;
};

/// @brief Export au format LAS via PDAL.
class LasExporter : public IPointCloudExporter {
public:
    /// @brief Sauvegarde le nuage au format LAS (LAS 1.4, format 0, échelle 1 mm) via PDAL.
    /// @param filename Chemin du fichier .las de sortie.
    /// @param points Nuage de points à exporter.
    void save(const std::string& filename, const std::vector<Point3>& points) override;
};

/// @brief Export au format LAZ via PDAL.
class LazBatchExporter{
public:
    /// @param filename Chemin fichier qui sera sauvegardé
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
