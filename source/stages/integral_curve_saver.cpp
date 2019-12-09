#include <dpa/stages/integral_curve_saver.hpp>

#include <fstream>

#include <boost/mpi.hpp>
#include <tbb/tbb.h>

#include <dpa/utility/xdmf.hpp>

#undef min
#undef max

namespace dpa
{
integral_curve_saver::integral_curve_saver (domain_partitioner* partitioner, const std::string& filepath) : partitioner_(partitioner), filepath_(filepath)
{
  const auto property = H5Pcreate(H5P_FILE_ACCESS);
  H5Pset_fapl_mpio(property, *partitioner->communicator(), MPI_INFO_NULL);

  file_ = H5Fcreate(filepath_.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, property);

  H5Pclose(property);
}
integral_curve_saver::~integral_curve_saver()
{
  H5Fclose(file_);
}

void integral_curve_saver::save_integral_curves(const integral_curves_3d& integral_curves)
{
  if (integral_curves.empty()) return;

  std::vector<std::array<std::uint8_t, 4>> colors(integral_curves.size(), std::array<std::uint8_t, 4>{0, 0, 0, 0});
  tbb::parallel_for(std::size_t(0), integral_curves.size(), std::size_t(1), [&] (const std::size_t index)
  {
    if (integral_curves[index] != vector3(-1, -1, -1))
    {
      vector3 tangent;
      if (index     > 1                      && integral_curves[index - 1] != vector3(-1, -1, -1))
        tangent = (integral_curves[index    ] - integral_curves[index - 1]).normalized();
      if (index + 1 < integral_curves.size() && integral_curves[index + 1] != vector3(-1, -1, -1))
        tangent = ((tangent + (integral_curves[index + 1] - integral_curves[index]).normalized()) / scalar(2)).normalized();

      colors[index] = std::array<std::uint8_t, 4>
      {
        std::uint8_t(255 * std::abs(tangent[0])),
        std::uint8_t(255 * std::abs(tangent[1])),
        std::uint8_t(255 * std::abs(tangent[2])),
        std::uint8_t(255)
      };
    }
  });
  
  std::vector<std::uint32_t> indices(2 * integral_curves.size(), std::numeric_limits<std::uint32_t>::max());
  tbb::parallel_for(std::size_t(0), integral_curves.size() - 1, std::size_t(1), [&] (const std::size_t index)
  {
    if (integral_curves[index] != vector3(-1, -1, -1) && integral_curves[index + 1] != vector3(-1, -1, -1))
    {
      indices[2 * index    ] = index;
      indices[2 * index + 1] = index + 1;
    }
  });
  indices.erase(std::remove(indices.begin(), indices.end(), std::numeric_limits<std::uint32_t>::max()), indices.end());

  std::vector<std::size_t> partial_vertex_counts(partitioner_->cartesian_communicator()->size());
  std::vector<std::size_t> partial_index_counts (partitioner_->cartesian_communicator()->size());
  boost::mpi::all_gather(*partitioner_->cartesian_communicator(), integral_curves.size(), partial_vertex_counts);
  boost::mpi::all_gather(*partitioner_->cartesian_communicator(), indices        .size(), partial_index_counts );
  auto global_vertex_count    = 3 * 4 * std::accumulate(partial_vertex_counts.begin(), partial_vertex_counts.end  (), std::size_t(0));
  auto global_color_count     = 4 * 1 * std::accumulate(partial_vertex_counts.begin(), partial_vertex_counts.end  (), std::size_t(0));
  auto global_index_count     = 1 * 4 * std::accumulate(partial_index_counts .begin(), partial_index_counts .end  (), std::size_t(0));
  auto local_vertex_offset    = 3 * 4 * std::accumulate(partial_vertex_counts.begin(), partial_vertex_counts.begin() + partitioner_->cartesian_communicator()->rank(), std::size_t(0));
  auto local_color_offset     = 4 * 1 * std::accumulate(partial_vertex_counts.begin(), partial_vertex_counts.begin() + partitioner_->cartesian_communicator()->rank(), std::size_t(0));
  auto local_index_offset     = 1 * 4 * std::accumulate(partial_index_counts .begin(), partial_index_counts .begin() + partitioner_->cartesian_communicator()->rank(), std::size_t(0));
  auto local_vertex_count     = 3 * 4 * std::size_t(integral_curves.size());
  auto local_color_count      = 4 * 1 * std::size_t(integral_curves.size());
  auto local_index_count      = 1 * 4 * std::size_t(indices.size());
  auto local_stride           = std::size_t(1);

  const auto vertices_space   = H5Screate_simple(1, &global_vertex_count, nullptr);
  const auto colors_space     = H5Screate_simple(1, &global_color_count , nullptr);
  const auto indices_space    = H5Screate_simple(1, &global_index_count , nullptr);
  const auto vertices_dataset = H5Dcreate2(file_, "vertices", H5T_NATIVE_FLOAT , vertices_space, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
  const auto colors_dataset   = H5Dcreate2(file_, "colors"  , H5T_NATIVE_UINT8 , colors_space  , H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
  const auto indices_dataset  = H5Dcreate2(file_, "indices" , H5T_NATIVE_UINT32, indices_space , H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
  const auto property         = H5Pcreate (H5P_DATASET_XFER);

  H5Pset_dxpl_mpio   (property, H5FD_MPIO_COLLECTIVE);
  H5Sselect_hyperslab(vertices_space, H5S_SELECT_SET, &local_vertex_offset, &local_stride, &local_vertex_count, nullptr);
  H5Sselect_hyperslab(colors_space  , H5S_SELECT_SET, &local_color_offset , &local_stride, &local_color_count , nullptr);
  H5Sselect_hyperslab(indices_space , H5S_SELECT_SET, &local_index_offset , &local_stride, &local_index_count , nullptr);
  H5Dwrite           (vertices_dataset, H5T_NATIVE_FLOAT , vertices_space, vertices_space, property, integral_curves.data()->data());
  H5Dwrite           (colors_dataset  , H5T_NATIVE_UINT8 , colors_space  , colors_space  , property, colors         .data()->data());
  H5Dwrite           (indices_dataset , H5T_NATIVE_UINT32, indices_space , indices_space , property, indices        .data());

  H5Pclose           (property);
  H5Sclose           (vertices_space);
  H5Sclose           (indices_space);
  H5Dclose           (vertices_dataset);
  H5Dclose           (colors_dataset  );
  H5Dclose           (indices_dataset );

  std::ofstream file (filepath_ + ".xdmf");
  file << create_xdmf(filepath_, local_vertex_count, local_index_count);
}
}