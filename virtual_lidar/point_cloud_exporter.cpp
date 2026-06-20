#include "point_cloud_exporter.hpp"
#include "logger.hpp"

#include <fstream>
#include <iomanip>

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
        out_file << p.x() << " " << p.y() << " " << p.z() << "\n";
    }

    SIM_INFO("Fichier PLY : {} écrit !", filename);

    out_file.close();
}

LazBatchExporter::LazBatchExporter(const std::string &filename)
    : m_filename(filename), m_num_batch(0), m_point_source_index(1){
    
    m_table = std::make_unique<pdal::PointTable>();
    m_table->layout()->registerDim(pdal::Dimension::Id::X);
    m_table->layout()->registerDim(pdal::Dimension::Id::Y);
    m_table->layout()->registerDim(pdal::Dimension::Id::Z);
    m_table->layout()->registerDim(pdal::Dimension::Id::PointSourceId);
    m_table->layout()->finalize();

    m_view = std::make_shared<pdal::PointView>(*m_table);

}

void LazBatchExporter::add_point_cloud(const std::vector<Point3> &points)
{
    if(is_full()){
        if(next_batch()){
            SIM_DEBUG("Le batch est plein. Nouveau batch créé merci !");
        }
    }
    for (const auto& p : points) {
        pdal::PointId id = m_view->size();
        m_view->setField(pdal::Dimension::Id::X, id, static_cast<double>(p.x()));
        m_view->setField(pdal::Dimension::Id::Y, id, static_cast<double>(p.y()));
        m_view->setField(pdal::Dimension::Id::Z, id, static_cast<double>(p.z()));
        m_view->setField(pdal::Dimension::Id::PointSourceId, id, m_point_source_index);
    }
    ++m_point_source_index;
}

bool LazBatchExporter::save_file(){
    try{
        pdal::BufferReader reader;
        reader.addView(m_view);

        pdal::Options options;
        std::stringstream final_name;

        std::string basename = m_filename.substr(0, m_filename.find('.'));
        std::string ext = m_filename.substr(m_filename.find('.'));
        
        final_name << basename << "_" << std::setprecision(2) << std::to_string(m_num_batch) << ext;

        options.add("filename", final_name.str());
        options.add("minor_version", 4);
        options.add("dataformat_id", 0);
        options.add("scale_x", 0.001);
        options.add("scale_y", 0.001);
        options.add("scale_z", 0.001);

        pdal::LasWriter writer;
        writer.setInput(reader);
        writer.setOptions(options);
        writer.prepare(*m_table);
        writer.execute(*m_table);

        SIM_INFO("Batch LAZ sauvegardé dans : {}", final_name.str());

        return true;
    }catch(pdal::pdal_error& e){
        SIM_ERROR("Erreur lors de l'écriture du fichier 3D Laz {} : {}", e.what(), m_filename);
        return false;
    }
}


bool LazBatchExporter::is_full(){
    // la valeur du champ PointSourceID est sur 16 bits soit 65535! 
    // Pour éviter la mise en cache de trop de données de point dans la RAM et 
    // comme pdal ne peut "flush" on choisie un batch tout les 1000 maillages.
    // La vrai solution serait de compter une limite de nombre de points à écrire.
    return m_point_source_index > 1000;
}

bool LazBatchExporter::next_batch()
{
    bool sucess = save_file();
    if(!sucess) return false;

    m_num_batch++;
    m_point_source_index = 1;
    
    m_table = std::make_unique<pdal::PointTable>();
    m_table->layout()->registerDim(pdal::Dimension::Id::X);
    m_table->layout()->registerDim(pdal::Dimension::Id::Y);
    m_table->layout()->registerDim(pdal::Dimension::Id::Z);
    m_table->layout()->registerDim(pdal::Dimension::Id::PointSourceId);
    m_table->layout()->finalize();
    m_view = std::make_shared<pdal::PointView>(*m_table);

    return true;
}
