#include <dpa/stages/uniform_seed_generator.hpp>

#include <tbb/tbb.h>

#include <dpa/math/indexing.hpp>

namespace dpa
{
std::vector<particle<vector3, integer>> uniform_seed_generator::generate(const vector3& offset, const vector3& size, const vector3& stride, integer iterations, integer process_index)
{
  ivector3 particles_per_dimension = (size.array() / stride.array()).cast<integer>();

  std::vector<particle<vector3, integer>> particles(particles_per_dimension.prod());
  tbb::parallel_for(std::size_t(0), particles.size(), std::size_t(1), [&] (const std::size_t index)
  {
    const ivector3 multi_index = unravel_index(index, particles_per_dimension);
    const vector3  position    = offset.array() + stride.array() * multi_index.cast<scalar>().array();
    particles[index]           = {position, iterations, process_index};
  });
  return particles;
}
}
