#include <dpa/stages/uniform_seed_generator.hpp>

#include <random>

#include <tbb/tbb.h>

#include <dpa/math/indexing.hpp>
#include <dpa/math/distributions/multivariate_uniform_distribution.hpp>

#undef min
#undef max

namespace dpa
{
std::vector<particle<vector3, integer>> uniform_seed_generator::generate       (vector3 offset, vector3 size, vector3                    stride, integer iterations, integer process_index, std::optional<aabb3> aabb)
{
  if (aabb)
  {
    auto intersection = aabb->intersection(aabb3(offset, offset + size));
    if (!intersection.isEmpty())
    {
      offset = intersection.min  ();
      size   = intersection.sizes();
    }
  }

  ivector3 particles_per_dimension = (size.array() / stride.array()).cast<integer>();

  std::vector<particle<vector3, integer>> particles(particles_per_dimension.prod());
  tbb::parallel_for(std::size_t(0), particles.size(), std::size_t(1), [&] (const std::size_t index)
  {
    const ivector3 multi_index = unravel_index(index, particles_per_dimension);
    const vector3  position    = offset.array() + stride.array() * multi_index.cast<scalar>().array();

#ifdef DPA_FTLE_SUPPORT
    particles[index]           = {position, iterations, relative_direction::center, process_index};
#else
    particles[index]           = {position, iterations, relative_direction::center};
#endif
  });
  return particles;
}
std::vector<particle<vector3, integer>> uniform_seed_generator::generate_random(vector3 offset, vector3 size, std::size_t                count , integer iterations, integer process_index, std::optional<aabb3> aabb) 
{
  if (aabb)
  {
    auto intersection = aabb->intersection(aabb3(offset, offset + size));
    if (!intersection.isEmpty())
    {
      offset = intersection.min  ();
      size   = intersection.sizes();
    }
  }

  auto distribution_range = std::initializer_list
  {
    std::initializer_list {offset[0], offset[0] + size[0]},
    std::initializer_list {offset[1], offset[1] + size[1]},
    std::initializer_list {offset[2], offset[2] + size[2]}
  };

  std::vector<particle<vector3, integer>> particles(count);
  tbb::parallel_for(std::size_t(0), particles.size(), std::size_t(1), [&] (const std::size_t index)
  {
    static thread_local std::random_device     random_device   ;
    static thread_local std::mt19937           mersenne_twister;
    multivariate_uniform_distribution<vector3> distribution(distribution_range);

#ifdef DPA_FTLE_SUPPORT
    particles[index] = {distribution(mersenne_twister), iterations, relative_direction::center, process_index};
#else
    particles[index] = {distribution(mersenne_twister), iterations, relative_direction::center};
#endif
  });
  return particles;
}
std::vector<particle<vector3, integer>> uniform_seed_generator::generate_random(vector3 offset, vector3 size, std::array<std::size_t, 2> range , integer iterations, integer process_index, std::optional<aabb3> aabb)
{
  std::random_device                         random_device;
  std::mt19937                               mersenne_twister(random_device());
  std::uniform_int_distribution<std::size_t> distribution(range[0], range[1]);
  return generate_random(offset, size, distribution(mersenne_twister), iterations, process_index, aabb);
}
}
