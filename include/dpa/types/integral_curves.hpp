#ifndef DPA_TYPES_INTEGRAL_CURVES_HPP
#define DPA_TYPES_INTEGRAL_CURVES_HPP

#include <vector>

#include <dpa/types/basic_types.hpp>

namespace dpa
{
using integral_curves_2d = std::vector<std::vector<vector2>>;
using integral_curves_3d = std::vector<std::vector<vector3>>;
using integral_curves_4d = std::vector<std::vector<vector4>>;
}

#endif