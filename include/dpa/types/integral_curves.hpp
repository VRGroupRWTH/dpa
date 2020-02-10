#ifndef DPA_TYPES_INTEGRAL_CURVES_HPP
#define DPA_TYPES_INTEGRAL_CURVES_HPP

#include <variant>
#include <vector>

#include <dpa/types/basic_types.hpp>

namespace dpa
{
struct integral_curve
{
  std::vector<vector3>                                                 vertices;
  std::variant<std::vector<scalar>       , std::vector<bvector3>>      colors  ;
  std::variant<std::vector<std::uint32_t>, std::vector<std::uint64_t>> indices ;
};

using integral_curves = std::vector<integral_curve>;
}

#endif