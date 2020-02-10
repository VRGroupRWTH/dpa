#include <dpa/stages/integral_curve_saver.hpp>

#include <filesystem>
#include <fstream>
#include <variant>

#include <boost/algorithm/string/replace.hpp>
#include <boost/mpi.hpp>
#include <hdf5.h>
#include <tbb/tbb.h>

#include <dpa/utility/xdmf.hpp>

#undef min
#undef max

namespace dpa
{
integral_curve_saver::integral_curve_saver (domain_partitioner* partitioner, const std::string& filepath) 
: partitioner_(partitioner)
, filepath_   (std::filesystem::path(filepath).replace_extension(".rank_" + std::to_string(partitioner_->cartesian_communicator()->rank()) + ".h5").string())
{

}

void integral_curve_saver::save(const integral_curves& integral_curves)
{
  std::vector<std::string> xdmf_bodies;

  const auto file = H5Fcreate(filepath_.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
  for (std::size_t curve_index = 0; curve_index < integral_curves.size(); ++curve_index)
  {
    auto& curve    = integral_curves[curve_index];
    auto& vertices = curve.vertices;
    auto& colors   = curve.colors  ;
    auto& indices  = curve.indices ;

    if (vertices.empty())
      continue;
    
    const auto use_scalar_colors    = std::holds_alternative<std::vector<scalar>>       (colors );
    const auto use_64_bit_indices   = std::holds_alternative<std::vector<std::uint64_t>>(indices);

    const auto vertex_element_count = hsize_t(3 * vertices.size());
    const auto color_element_count  = hsize_t(use_scalar_colors  ? vertices.size() : 3 * vertices.size());
    const auto index_count          = hsize_t(use_64_bit_indices ? std::get<std::vector<std::uint64_t>>(indices).size() : std::get<std::vector<std::uint32_t>>(indices).size());
    const auto vertices_name        = "vertices_" + std::to_string(curve_index);
    const auto colors_name          = "colors_"   + std::to_string(curve_index);
    const auto indices_name         = "indices_"  + std::to_string(curve_index);
    const auto vertices_space       = H5Screate_simple(1, &vertex_element_count, nullptr);
    const auto colors_space         = H5Screate_simple(1, &color_element_count , nullptr);
    const auto indices_space        = H5Screate_simple(1, &index_count         , nullptr);
    const auto vertices_dataset     = H5Dcreate2(file, vertices_name.c_str(), H5T_NATIVE_FLOAT                                          , vertices_space, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    const auto colors_dataset       = H5Dcreate2(file, colors_name  .c_str(), use_scalar_colors  ? H5T_NATIVE_FLOAT  : H5T_NATIVE_UINT8 , colors_space  , H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    const auto indices_dataset      = H5Dcreate2(file, indices_name .c_str(), use_64_bit_indices ? H5T_NATIVE_UINT64 : H5T_NATIVE_UINT32, indices_space , H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    H5Dwrite(vertices_dataset, H5T_NATIVE_FLOAT                                          , vertices_space, vertices_space, H5P_DEFAULT, vertices.data()->data());
    H5Dwrite(colors_dataset  , use_scalar_colors  ? H5T_NATIVE_FLOAT  : H5T_NATIVE_UINT8 , colors_space  , colors_space  , H5P_DEFAULT, use_scalar_colors  ? reinterpret_cast<const void*>(std::get<std::vector<scalar>>       (colors ).data()) : std::get<std::vector<bvector3>>     (colors ).data()->data());
    H5Dwrite(indices_dataset , use_64_bit_indices ? H5T_NATIVE_UINT64 : H5T_NATIVE_UINT32, indices_space , indices_space , H5P_DEFAULT, use_64_bit_indices ? reinterpret_cast<const void*>(std::get<std::vector<std::uint64_t>>(indices).data()) : std::get<std::vector<std::uint32_t>>(indices).data());
    H5Sclose(vertices_space  );
    H5Sclose(colors_space    );
    H5Sclose(indices_space   );
    H5Dclose(vertices_dataset);
    H5Dclose(colors_dataset  );
    H5Dclose(indices_dataset );
    
    auto& xdmf = xdmf_bodies.emplace_back(xdmf_body_geometry);
    boost::replace_all(xdmf, "$FILEPATH"             , std::filesystem::path(filepath_).filename().string());
    boost::replace_all(xdmf, "$GRID_NAME"            , "grid_" + std::to_string(curve_index));   
    boost::replace_all(xdmf, "$VERTICES_DATASET_NAME", vertices_name);
    boost::replace_all(xdmf, "$VERTEX_ARRAY_SIZE"    , std::to_string(vertex_element_count));
    boost::replace_all(xdmf, "$COLORS_DATASET_NAME"  , colors_name  );
    boost::replace_all(xdmf, "$COLOR_ARRAY_SIZE"     , std::to_string(color_element_count));
    boost::replace_all(xdmf, "$COLOR_ATTRIBUTE_TYPE" , use_scalar_colors ? "Scalar" : "Vector");
    boost::replace_all(xdmf, "$COLOR_ARRAY_TYPE"     , use_scalar_colors ? "Float"  : "UChar" );
    boost::replace_all(xdmf, "$COLOR_PRECISION"      , use_scalar_colors ? "4"      : "1"     );
    boost::replace_all(xdmf, "$INDICES_DATASET_NAME" , indices_name );
    boost::replace_all(xdmf, "$INDEX_ARRAY_SIZE"     , std::to_string(index_count));
    boost::replace_all(xdmf, "$INDEX_PRECISION"      , use_64_bit_indices ? "8" : "4");
    boost::replace_all(xdmf, "$POLYLINE_COUNT"       , std::to_string(index_count / 2));
  }
  H5Fclose(file);

  std::ofstream stream(filepath_ + ".xdmf");
  stream << xdmf_header << std::accumulate(xdmf_bodies.begin(), xdmf_bodies.end(), std::string("")) << xdmf_footer;
}
}