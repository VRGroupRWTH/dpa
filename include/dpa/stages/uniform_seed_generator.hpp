#ifndef DPA_STAGES_UNIFORM_SEED_GENERATOR_HPP
#define DPA_STAGES_UNIFORM_SEED_GENERATOR_HPP

#include <optional>
#include <vector>

#include <dpa/types/basic_types.hpp>
#include <dpa/types/particle.hpp>

namespace dpa
{
class uniform_seed_generator
{
public:
  static std::vector<particle_3d> generate       (vector3 offset, vector3 size, vector3   stride, dpa::size iterations, integer process_index, std::optional<aabb3> aabb = std::nullopt);
  static std::vector<particle_3d> generate_random(vector3 offset, vector3 size, dpa::size count , dpa::size iterations, integer process_index, std::optional<aabb3> aabb = std::nullopt);
  static std::vector<particle_3d> generate_random(vector3 offset, vector3 size, svector2  range , dpa::size iterations, integer process_index, std::optional<aabb3> aabb = std::nullopt);

  // TODO: Seeds from radius, generated within the sphere enclosed by it.
  // TODO: Seeds from vector of particles     (read in parallel, then distributed all to all, also from file of 1D vector3 array).
  // TODO: Seeds from scalar mask of booleans (read in parallel, one particle per true voxel, also from file of 3D boolean array).
};
}

#endif