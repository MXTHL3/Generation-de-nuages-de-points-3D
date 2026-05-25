#include "point_cloud_exporter.h"

#include <fstream>

void PlyExporter::save(const std::string& filename, const std::vector<Point3>& points) {
    std::ofstream out_file(filename);
    
    if(!out_file.is_open()){
        throw std::runtime_error("Erreur: le fichier de nuage de points .PLY ne peut être créé !");
    }

    // Header

    out_file << "ply\n"
            << "format ascii 1.0\n"
            << "element vertex " << points.size() << "\n"
            << "property float x\n"
            << "property float y\n"
            << "property float z\n"
            << "end_header\n";
    
    // Contenu : X Y Z

    for(const auto& p : points) {
        out_file << p.x() << " " << p.y() << " " << p.z() << std::endl;
    }

}

void LasExporter::save(const std::string& filename,
                       const std::vector<Point3>& points)
{
    pdal::PointTable table;

    auto view = std::make_shared<pdal::PointView>(table);

    for (const auto& p : points)
    {
        pdal::PointId id = view->size();

        view->setField(pdal::Dimension::Id::X, id, static_cast<double>(p.x()));
        view->setField(pdal::Dimension::Id::Y,id,static_cast<double>(p.y()));
        view->setField(pdal::Dimension::Id::Z,id, static_cast<double>(p.z()));
    }

    pdal::BufferReader reader;
    reader.addView(view);

    pdal::Options options;
    options.add("filename", filename);
    options.add("minor_version", 4);
    options.add("dataformat_id", 0);
    options.add("scale_x", 0.001);
    options.add("scale_y", 0.001);
    options.add("scale_z", 0.001);

    pdal::LasWriter writer;
    writer.setInput(reader);
    writer.setOptions(options);
    writer.prepare(table);
    writer.execute(table);
}