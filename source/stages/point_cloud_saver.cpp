#include <dpa/stages/point_cloud_saver.hpp>

#include <filesystem>
#include <fstream>
#include <variant>

#include <boost/algorithm/string/replace.hpp>
#include <boost/mpi.hpp>
#include <hdf5.h>

#include <dpa/utility/xdmf.hpp>

#undef min
#undef max

namespace dpa
{
point_cloud_saver::point_cloud_saver (domain_partitioner* partitioner, const std::string& filepath) 
: partitioner_(partitioner)
, filepath_   (std::filesystem::path(filepath).replace_extension(".rank_" + std::to_string(partitioner_->cartesian_communicator()->rank()) + "_points.h5").string())
{

}

void point_cloud_saver::save(const point_cloud& point_cloud)
{
  const auto use_scalar_colors    = std::holds_alternative<std::vector<scalar>>(point_cloud.colors);
  const auto vertex_element_count = hsize_t(3 * point_cloud.vertices.size());
  const auto color_element_count  = hsize_t(use_scalar_colors  ? point_cloud.vertices.size() : 3 * point_cloud.vertices.size());

  const auto file                 = H5Fcreate(filepath_.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
  const auto vertices_space       = H5Screate_simple(1, &vertex_element_count, nullptr);
  const auto colors_space         = H5Screate_simple(1, &color_element_count , nullptr);
  const auto vertices_dataset     = H5Dcreate2(file, "vertices", H5T_NATIVE_FLOAT                                        , vertices_space, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
  const auto colors_dataset       = H5Dcreate2(file, "colors"  , use_scalar_colors ? H5T_NATIVE_FLOAT  : H5T_NATIVE_UINT8, colors_space  , H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
  H5Dwrite(vertices_dataset, H5T_NATIVE_FLOAT                                       , vertices_space, vertices_space, H5P_DEFAULT, point_cloud.vertices.data()->data());
  H5Dwrite(colors_dataset  , use_scalar_colors ? H5T_NATIVE_FLOAT : H5T_NATIVE_UINT8, colors_space  , colors_space  , H5P_DEFAULT, use_scalar_colors ? reinterpret_cast<const void*>(std::get<std::vector<scalar>>(point_cloud.colors).data()) : std::get<std::vector<bvector3>>(point_cloud.colors).data()->data());
  H5Sclose(vertices_space  );
  H5Sclose(colors_space    );
  H5Dclose(vertices_dataset);
  H5Dclose(colors_dataset  );
  H5Fclose(file            );
  
  auto xdmf = xdmf_body_points;
  boost::replace_all(xdmf, "$FILEPATH"            , std::filesystem::path(filepath_).filename().string());
  boost::replace_all(xdmf, "$VERTEX_COUNT"        , point_cloud.vertices.size());
  boost::replace_all(xdmf, "$VERTEX_ARRAY_SIZE"   , std::to_string(vertex_element_count));
  boost::replace_all(xdmf, "$COLOR_ARRAY_SIZE"    , std::to_string(color_element_count ));
  boost::replace_all(xdmf, "$COLOR_ATTRIBUTE_TYPE", use_scalar_colors ? "Scalar" : "Vector");
  boost::replace_all(xdmf, "$COLOR_ARRAY_TYPE"    , use_scalar_colors ? "Float"  : "UChar" );
  boost::replace_all(xdmf, "$COLOR_PRECISION"     , use_scalar_colors ? "4"      : "1"     );
  std::ofstream stream(filepath_ + ".xdmf");
  stream << xdmf_header << xdmf << xdmf_footer;
}
}