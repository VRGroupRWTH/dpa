#ifndef DPA_STAGES_REGULAR_GRID_LOADER_HPP
#define DPA_STAGES_REGULAR_GRID_LOADER_HPP

#include <string>
#include <unordered_map>

#include <hdf5.h>

#include <dpa/stages/domain_partitioner.hpp>
#include <dpa/types/basic_types.hpp>
#include <dpa/types/regular_fields.hpp>
#include <dpa/types/relative_direction.hpp>

namespace dpa
{
class regular_grid_loader
{
public:
  explicit regular_grid_loader  (domain_partitioner* partitioner, const std::string& filepath, const std::string& dataset_path, const std::string& spacing_path);
  regular_grid_loader           (const regular_grid_loader&  that) = delete ;
  regular_grid_loader           (      regular_grid_loader&& temp) = default;
 ~regular_grid_loader           ()                                 = default;
  regular_grid_loader& operator=(const regular_grid_loader&  that) = delete ;
  regular_grid_loader& operator=(      regular_grid_loader&& temp) = default;

  svector3                                                        load_dimensions   ();
  std::unordered_map<relative_direction, regular_vector_field_3d> load_vector_fields(const bool load_neighbors);

protected:
  regular_vector_field_3d                                         load_vector_field (const svector3& offset, const svector3& size, hid_t dataset, hid_t spacing);

  domain_partitioner* partitioner_ = nullptr;
  const std::string   filepath_    ;
  const std::string   dataset_path_;
  const std::string   spacing_path_;
};
}

#endif