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
 ~regular_grid_loader           ();
  regular_grid_loader& operator=(const regular_grid_loader&  that) = delete ;
  regular_grid_loader& operator=(      regular_grid_loader&& temp) = default;

  ivector3                                                        load_dimensions   ();
  std::unordered_map<relative_direction, regular_vector_field_3d> load_vector_fields(const bool load_neighbors);

protected:
  regular_vector_field_3d                                         load_vector_field (const ivector3& offset, const ivector3& size);

  domain_partitioner* partitioner_ = nullptr;
  hid_t               file_        = 0;
  hid_t               dataset_     = 0;
  hid_t               spacing_     = 0;
};
}

#endif