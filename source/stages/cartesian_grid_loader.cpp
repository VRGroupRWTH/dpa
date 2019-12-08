#include <dpa/stages/cartesian_grid_loader.hpp>

#include <highfive/H5Attribute.hpp>
#include <highfive/H5DataSet.hpp>
#include <tbb/tbb.h>

namespace dpa
{
cartesian_grid_loader::cartesian_grid_loader (domain_partitioner* partitioner, const std::string& filepath, const std::string& dataset_path, const std::string& spacing_path) : partitioner_(partitioner)
{
#ifdef H5_HAVE_PARALLEL
  file_    = std::make_unique<HighFive::File>(filepath, HighFive::File::ReadWrite, HighFive::MPIOFileDriver(*partitioner_->communicator(), MPI_INFO_NULL));
#else
  file_    = std::make_unique<HighFive::File>(filepath, HighFive::File::ReadWrite);
#endif

  dataset_ = std::make_unique<HighFive::DataSet>  (file_->getDataSet  (dataset_path));
  spacing_ = std::make_unique<HighFive::Attribute>(file_->getAttribute(spacing_path));
}

ivector3                       cartesian_grid_loader::load_dimensions             ()
{
  auto dimensions = dataset_->getDimensions();
  return ivector3(dimensions[0], dimensions[1], dimensions[2]);
}

vector_field_3d                cartesian_grid_loader::load_local_vector_field     ()
{
  return load_vector_field(partitioner_->local_partition().offset, partitioner_->block_size());
}
std::optional<vector_field_3d> cartesian_grid_loader::load_positive_x_vector_field()
{
  if (partitioner_->positive_x_partition())
    return load_vector_field(partitioner_->positive_x_partition()->offset, partitioner_->block_size());
  return std::nullopt;
}
std::optional<vector_field_3d> cartesian_grid_loader::load_negative_x_vector_field()
{
  if (partitioner_->negative_x_partition())
    return load_vector_field(partitioner_->negative_x_partition()->offset, partitioner_->block_size());
  return std::nullopt;
}
std::optional<vector_field_3d> cartesian_grid_loader::load_positive_y_vector_field()
{
  if (partitioner_->positive_y_partition())
    return load_vector_field(partitioner_->positive_y_partition()->offset, partitioner_->block_size());
  return std::nullopt;
}
std::optional<vector_field_3d> cartesian_grid_loader::load_negative_y_vector_field()
{
  if (partitioner_->negative_y_partition())
    return load_vector_field(partitioner_->negative_y_partition()->offset, partitioner_->block_size());
  return std::nullopt;
}
std::optional<vector_field_3d> cartesian_grid_loader::load_positive_z_vector_field()
{
  if (partitioner_->positive_z_partition())
    return load_vector_field(partitioner_->positive_z_partition()->offset, partitioner_->block_size());
  return std::nullopt;
}
std::optional<vector_field_3d> cartesian_grid_loader::load_negative_z_vector_field()
{
  if (partitioner_->negative_z_partition())
    return load_vector_field(partitioner_->negative_z_partition()->offset, partitioner_->block_size());
  return std::nullopt;
}

vector_field_3d                cartesian_grid_loader::load_vector_field           (const ivector3 offset, const ivector3 size)
{
  boost::multi_array<scalar, 4> data;
  dataset_->select(
    {std::size_t(offset[0]), std::size_t(offset[1]), std::size_t(offset[2]), 0},
    {std::size_t(size  [0]), std::size_t(size  [1]), std::size_t(size  [2]), 3},
    {1, 1, 1, 1}).read(data);

  std::array<scalar, 3> spacing;
  spacing_->read(spacing);

  vector_field_3d vector_field;

  vector_field.data.resize(std::array<integer, 3>{size[0], size[1], size[2]});
  tbb::parallel_for(tbb::blocked_range3d<std::size_t>(0, size[0], 0, size[1], 0, size[2]), [&] (const tbb::blocked_range3d<std::size_t>& index) {
    for (auto x = index.pages().begin(), x_end = index.pages().end(); x < x_end; ++x) {
    for (auto y = index.rows ().begin(), y_end = index.rows ().end(); y < y_end; ++y) {
    for (auto z = index.cols ().begin(), z_end = index.cols ().end(); z < z_end; ++z) {
      vector_field.data[x][y][z] = {data[x][y][z][0], data[x][y][z][1], data[x][y][z][2]};
    }}}
  });
  
  vector_field.spacing = vector3(spacing[0], spacing[1], spacing[2]);
  vector_field.offset  = offset.cast<scalar>().array() * vector_field.spacing.array();
  vector_field.size    = size  .cast<scalar>().array() * vector_field.spacing.array();
}
}
