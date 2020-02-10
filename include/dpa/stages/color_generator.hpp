#ifndef DPA_STAGES_COLOR_GENERATOR_HPP
#define DPA_STAGES_COLOR_GENERATOR_HPP

#include <dpa/types/integral_curves.hpp>
#include <dpa/types/regular_fields.hpp>
#include <dpa/types/relative_direction.hpp>

namespace dpa
{
class color_generator
{
public:
  static void generate_from_tangents          (integral_curves& integral_curves);
  static void generate_from_angular_velocities(integral_curves& integral_curves);
  static void generate_from_velocities        (integral_curves& integral_curves, const std::unordered_map<relative_direction, regular_vector_field_3d>& vector_fields);
  static void generate_from_potentials        (integral_curves& integral_curves, const std::unordered_map<relative_direction, regular_vector_field_3d>& vector_fields);
};
}

#endif