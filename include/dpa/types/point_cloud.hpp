#ifndef DPA_TYPES_POINT_CLOUD_HPP
#define DPA_TYPES_POINT_CLOUD_HPP

#include <variant>
#include <vector>

#include <dpa/types/basic_types.hpp>

namespace dpa
{
struct point_cloud
{
  std::vector<vector3>                                     vertices;
  std::variant<std::vector<scalar>, std::vector<bvector3>> colors  ;
};
}

#endif