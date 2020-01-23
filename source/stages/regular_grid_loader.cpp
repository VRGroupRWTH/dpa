#include <dpa/stages/regular_grid_loader.hpp>

#include <array>

namespace dpa
{
regular_grid_loader::regular_grid_loader (domain_partitioner* partitioner, const std::string& filepath, const std::string& dataset_path, const std::string& spacing_path) : partitioner_(partitioner)
{
  const auto property = H5Pcreate(H5P_FILE_ACCESS);
  H5Pset_fapl_mpio(property, *partitioner->communicator(), MPI_INFO_NULL);

  file_    = H5Fopen (filepath.c_str(), H5F_ACC_RDONLY, property);
  dataset_ = H5Dopen2(file_, dataset_path.c_str(), H5P_DEFAULT);
  spacing_ = H5Aopen (file_, spacing_path.c_str(), H5P_DEFAULT);

  H5Pclose(property);
}
regular_grid_loader::~regular_grid_loader()
{
  H5Aclose(spacing_);
  H5Dclose(dataset_);
  H5Fclose(file_   );
}

svector3                                                        regular_grid_loader::load_dimensions   ()
{
  std::array<hsize_t, 4> dimensions {0, 0, 0, 0};

  const auto space = H5Dget_space(dataset_);
  H5Sget_simple_extent_dims(space, dimensions.data(), nullptr);
  H5Sclose(space);

  return svector3(dimensions[0], dimensions[1], dimensions[2]);
}
std::unordered_map<relative_direction, regular_vector_field_3d> regular_grid_loader::load_vector_fields(const bool load_neighbors)
{
  std::unordered_map<relative_direction, regular_vector_field_3d> vector_fields;

  auto partitions = partitioner_->partitions();

  vector_fields.emplace(center, load_vector_field(partitions.at(center).ghosted_offset, partitions.at(center).ghosted_block_size));
  if (load_neighbors)
  {
    if (partitions.find(negative_x) != partitions.end()) vector_fields.emplace(negative_x, load_vector_field(partitions.at(negative_x).ghosted_offset, partitions.at(negative_x).ghosted_block_size));
    if (partitions.find(positive_x) != partitions.end()) vector_fields.emplace(positive_x, load_vector_field(partitions.at(positive_x).ghosted_offset, partitions.at(positive_x).ghosted_block_size));
    if (partitions.find(negative_y) != partitions.end()) vector_fields.emplace(negative_y, load_vector_field(partitions.at(negative_y).ghosted_offset, partitions.at(negative_y).ghosted_block_size));
    if (partitions.find(positive_y) != partitions.end()) vector_fields.emplace(positive_y, load_vector_field(partitions.at(positive_y).ghosted_offset, partitions.at(positive_y).ghosted_block_size));
    if (partitions.find(negative_z) != partitions.end()) vector_fields.emplace(negative_z, load_vector_field(partitions.at(negative_z).ghosted_offset, partitions.at(negative_z).ghosted_block_size));
    if (partitions.find(positive_z) != partitions.end()) vector_fields.emplace(positive_z, load_vector_field(partitions.at(positive_z).ghosted_offset, partitions.at(positive_z).ghosted_block_size));
  }

  return vector_fields;
}

regular_vector_field_3d                                         regular_grid_loader::load_vector_field (const svector3& offset, const svector3& size)
{
  regular_vector_field_3d vector_field {boost::multi_array<vector3, 3>(boost::extents[size[0]][size[1]][size[2]])};

  const std::array<hsize_t, 4> native_offset {hsize_t(offset[0]), hsize_t(offset[1]), hsize_t(offset[2]), 0};
  const std::array<hsize_t, 4> native_size   {hsize_t(size  [0]), hsize_t(size  [1]), hsize_t(size  [2]), 3};
  const std::array<hsize_t, 4> native_stride {1, 1, 1, 1};

  const auto space    = H5Dget_space    (dataset_);
  const auto memspace = H5Screate_simple(4, native_size.data(), NULL);
  const auto property = H5Pcreate       (H5P_DATASET_XFER);
  H5Pset_dxpl_mpio   (property, H5FD_MPIO_COLLECTIVE);
  H5Sselect_hyperslab(space, H5S_SELECT_SET, native_offset.data(), native_stride.data(), native_size.data(), nullptr);
  H5Dread            (dataset_, H5T_NATIVE_FLOAT, memspace, space, property, vector_field.data.origin()->data());
  H5Pclose           (property);
  H5Sclose           (memspace);
  H5Sclose           (space);

  H5Aread            (spacing_, H5T_NATIVE_FLOAT, vector_field.spacing.data());

  vector_field.offset  = offset.cast<scalar>().array() * vector_field.spacing.array();
  vector_field.size    = size  .cast<scalar>().array() * vector_field.spacing.array();
  return vector_field;
}
}
