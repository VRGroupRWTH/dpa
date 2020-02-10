#ifndef DPA_STAGES_REGULAR_GRID_SAVER_HPP
#define DPA_STAGES_REGULAR_GRID_SAVER_HPP

#include <string>

#include <dpa/stages/domain_partitioner.hpp>
#include <dpa/types/regular_fields.hpp>

namespace dpa
{
class regular_grid_saver
{
public:
  explicit regular_grid_saver  (domain_partitioner* partitioner, const std::string& filepath);
  regular_grid_saver           (const regular_grid_saver&  that) = delete ;
  regular_grid_saver           (      regular_grid_saver&& temp) = default;
 ~regular_grid_saver           ()                                = default;
  regular_grid_saver& operator=(const regular_grid_saver&  that) = delete ;
  regular_grid_saver& operator=(      regular_grid_saver&& temp) = default;

  void save(const regular_scalar_field_3d& scalar_field);
  
protected:
  domain_partitioner* partitioner_ = {};
  std::string         filepath_    = {};
};
}

#endif