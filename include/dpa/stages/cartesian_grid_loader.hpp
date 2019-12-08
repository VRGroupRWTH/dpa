#ifndef DPA_STAGES_CARTESIAN_GRID_LOADER_HPP
#define DPA_STAGES_CARTESIAN_GRID_LOADER_HPP

#define HIGHFIVE_PARALLEL_HDF5
#define USE_BOOST
#define USE_EIGEN

#include <memory>
#include <optional>
#include <string>

#include <highfive/H5File.hpp>

#include <dpa/stages/domain_partitioner.hpp>
#include <dpa/types/basic_types.hpp>
#include <dpa/types/fields.hpp>

namespace dpa
{
class cartesian_grid_loader
{
public:
  explicit cartesian_grid_loader  (domain_partitioner* partitioner, const std::string& filepath, const std::string& dataset_path, const std::string& spacing_path);
  cartesian_grid_loader           (const cartesian_grid_loader&  that) = delete ;
  cartesian_grid_loader           (      cartesian_grid_loader&& temp) = default;
 ~cartesian_grid_loader           ()                                   = default;
  cartesian_grid_loader& operator=(const cartesian_grid_loader&  that) = delete ;
  cartesian_grid_loader& operator=(      cartesian_grid_loader&& temp) = default;

  ivector3                       load_dimensions             ();
                                 
  vector_field_3d                load_local_vector_field     ();
  std::optional<vector_field_3d> load_positive_x_vector_field();
  std::optional<vector_field_3d> load_negative_x_vector_field();
  std::optional<vector_field_3d> load_positive_y_vector_field();
  std::optional<vector_field_3d> load_negative_y_vector_field();
  std::optional<vector_field_3d> load_positive_z_vector_field();
  std::optional<vector_field_3d> load_negative_z_vector_field();
  
protected:
  vector_field_3d                load_vector_field           (const ivector3 offset, const ivector3 size);

  domain_partitioner*                  partitioner_ = nullptr;
  std::unique_ptr<HighFive::File>      file_        = nullptr;
  std::unique_ptr<HighFive::DataSet>   dataset_     = nullptr;
  std::unique_ptr<HighFive::Attribute> spacing_     = nullptr;
};
}

#endif