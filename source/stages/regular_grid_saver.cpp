#include <dpa/stages/regular_grid_saver.hpp>

#include <filesystem>
#include <fstream>

#include <boost/algorithm/string/replace.hpp>
#include <boost/mpi.hpp>
#include <hdf5.h>

#include <dpa/utility/xdmf.hpp>

namespace dpa
{
regular_grid_saver::regular_grid_saver (domain_partitioner* partitioner, const std::string& filepath) 
: partitioner_(partitioner)
, filepath_   (std::filesystem::path(filepath).replace_extension(".rank_" + std::to_string(partitioner_->cartesian_communicator()->rank()) + "_grid.h5").string())
{

}

void regular_grid_saver::save(const regular_scalar_field_3d& scalar_field)
{
  if (scalar_field.data.empty())
    return;

  const auto shape                 = scalar_field.data.shape();

  const auto file                  = H5Fcreate(filepath_.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
  const auto dataset_element_count = std::array<hsize_t, 3>{hsize_t(shape[0]), hsize_t(shape[1]), hsize_t(shape[2])};
  const auto dataset_space         = H5Screate_simple(3, dataset_element_count.data(), nullptr);
  const auto dataset               = H5Dcreate2(file, "data", H5T_NATIVE_FLOAT, dataset_space, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
  H5Dwrite(dataset, H5T_NATIVE_FLOAT, dataset_space, dataset_space, H5P_DEFAULT, scalar_field.data.data());
  H5Sclose(dataset_space);
  H5Dclose(dataset);
  H5Fclose(file);

  auto xdmf = xdmf_body_volume;
  boost::replace_all(xdmf, "$FILEPATH"    , std::filesystem::path(filepath_).filename().string());
  boost::replace_all(xdmf, "$DATASET_NAME", "data");
  boost::replace_all(xdmf, "$SIZE"        , std::to_string(scalar_field.data.shape()[0]) + " " + std::to_string(scalar_field.data.shape()[1]) + " " + std::to_string(scalar_field.data.shape()[2]));
  boost::replace_all(xdmf, "$ORIGIN"      , std::to_string(scalar_field.offset      [0]) + " " + std::to_string(scalar_field.offset      [1]) + " " + std::to_string(scalar_field.offset      [2]));
  boost::replace_all(xdmf, "$SPACING"     , std::to_string(scalar_field.spacing     [0]) + " " + std::to_string(scalar_field.spacing     [1]) + " " + std::to_string(scalar_field.spacing     [2]));

  std::ofstream stream(filepath_ + ".xdmf");
  stream << xdmf_header << xdmf << xdmf_footer;
}
}