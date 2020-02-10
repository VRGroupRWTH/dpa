#include <dpa/stages/color_generator.hpp>

#include <variant>
#include <vector>

#include <tbb/tbb.h>

#include <dpa/types/basic_types.hpp>

#undef min
#undef max

namespace dpa
{
void color_generator::generate_from_tangents          (integral_curves& integral_curves)
{
  for (auto& integral_curve : integral_curves)
  {
    auto& vertices = integral_curve.vertices;
    auto& colors   = integral_curve.colors  ;

    colors = std::vector<bvector3>(vertices.size());
    tbb::parallel_for(std::size_t(0), vertices.size(), std::size_t(1), [&] (const std::size_t vertex_index)
    {
      if (vertices[vertex_index] != terminal_value<vector3>())
      {
        vector3 tangent;
        if (vertex_index     > 1               && vertices[vertex_index - 1] != terminal_value<vector3>())
          tangent = (vertices[vertex_index    ] - vertices[vertex_index - 1]).normalized();
        if (vertex_index + 1 < vertices.size() && vertices[vertex_index + 1] != terminal_value<vector3>())
          tangent = ((tangent + (vertices[vertex_index + 1] - vertices[vertex_index]).normalized()) / scalar(2)).normalized();
        std::get<std::vector<bvector3>>(colors)[vertex_index] = (255 * tangent.cwiseAbs()).cast<byte>();
      }
    });
  }
}
void color_generator::generate_from_angular_velocities(integral_curves& integral_curves)
{
  for (auto& integral_curve : integral_curves)
  {
    auto& vertices = integral_curve.vertices;
    auto& colors   = integral_curve.colors  ;

    colors = std::vector<scalar>(vertices.size());
    tbb::parallel_for(std::size_t(0), vertices.size(), std::size_t(1), [&] (const std::size_t vertex_index)
    {
      if (vertices[vertex_index] != terminal_value<vector3>())
      {
        vector3 prev, next;
        if (vertex_index     > 1               && vertices[vertex_index - 1] != terminal_value<vector3>())
          prev = (vertices[vertex_index    ] - vertices[vertex_index - 1]).normalized();
        if (vertex_index + 1 < vertices.size() && vertices[vertex_index + 1] != terminal_value<vector3>())
          next = (vertices[vertex_index + 1] - vertices[vertex_index    ]).normalized();
        std::get<std::vector<scalar>>(colors)[vertex_index] = std::atan2(prev.cross(next).norm(), prev.dot(next));
      }
    });
  }
}
void color_generator::generate_from_velocities        (integral_curves& integral_curves, const std::unordered_map<relative_direction, regular_vector_field_3d>& vector_fields)
{
  for (auto& integral_curve : integral_curves)
  {
    auto& vertices = integral_curve.vertices;
    auto& colors   = integral_curve.colors  ;

    colors = std::vector<scalar>(vertices.size(), 0);
    tbb::parallel_for(std::size_t(0), vertices.size(), std::size_t(1), [&] (const std::size_t vertex_index)
    {
      if (vertices[vertex_index] != terminal_value<vector3>())
        for (auto& vector_field : vector_fields)
          if (vector_field.second.contains(vertices[vertex_index]))
            std::get<std::vector<scalar>>(colors)[vertex_index] = vector_field.second.interpolate(vertices[vertex_index]).norm();
    });
  }
}
void color_generator::generate_from_potentials        (integral_curves& integral_curves, const std::unordered_map<relative_direction, regular_vector_field_3d>& vector_fields)
{
  std::unordered_map<relative_direction, regular_scalar_field_3d> scalar_fields;
  for (auto& entry : vector_fields)
    scalar_fields[entry.first] = entry.second.potential();

  for (auto& integral_curve : integral_curves)
  {
    auto& vertices = integral_curve.vertices;
    auto& colors   = integral_curve.colors  ;

    colors = std::vector<scalar>(vertices.size(), 0);
    tbb::parallel_for(std::size_t(0), vertices.size(), std::size_t(1), [&] (const std::size_t vertex_index)
    {
      if (vertices[vertex_index] != terminal_value<vector3>())
        for (auto& scalar_field : scalar_fields)
          if (scalar_field.second.contains(vertices[vertex_index]))
            std::get<std::vector<scalar>>(colors)[vertex_index] = scalar_field.second.interpolate(vertices[vertex_index]);
    });
  }
}
}