#include <dpa/stages/index_generator.hpp>

#include <tbb/tbb.h>

#undef min
#undef max

namespace dpa
{
void index_generator::generate(integral_curves& integral_curves, const bool use_64_bit)
{
  for (auto& curve : integral_curves)
  {
    if (curve.vertices.empty()) continue;

    use_64_bit
      ? curve.indices = std::vector<std::uint64_t>(2 * curve.vertices.size(), std::numeric_limits<std::uint64_t>::max())
      : curve.indices = std::vector<std::uint32_t>(2 * curve.vertices.size(), std::numeric_limits<std::uint32_t>::max());
    
    std::visit([&] (auto& cast_indices) 
    {
      tbb::parallel_for(std::size_t(0), curve.vertices.size() - 1, std::size_t(1), [&] (const std::size_t vertex_index)
      {
        if (curve.vertices[vertex_index    ] != terminal_value<vector3>() && 
            curve.vertices[vertex_index + 1] != terminal_value<vector3>())
        {
          cast_indices[2 * vertex_index    ] = vertex_index;
          cast_indices[2 * vertex_index + 1] = vertex_index + 1;
        }
      });

      cast_indices.erase(std::remove(
        cast_indices.begin(), 
        cast_indices.end  (), 
        use_64_bit ? std::numeric_limits<std::uint64_t>::max() : std::numeric_limits<std::uint32_t>::max()), 
        cast_indices.end  ());
    }, curve.indices);
  }
}
}
