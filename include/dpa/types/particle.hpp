#ifndef DPA_TYPES_PARTICLE_HPP
#define DPA_TYPES_PARTICLE_HPP

#include <cstdint>

#include <dpa/types/relative_direction.hpp>

namespace dpa
{
// Ducks [] on the position_type.
template <typename position_type, typename size_type>
struct particle
{
  // Function for boost::serialization which is used by boost::mpi.
  template<class archive_type>
  void serialize(archive_type& archive, const std::uint32_t version)
  {
    archive & position[0];
    archive & position[1];
    archive & position[2];
    archive & remaining_iterations;
    archive & relative_direction;

#ifdef DPA_FTLE_SUPPORT
    archive & original_rank;
    archive & original_position[0];
    archive & original_position[1];
    archive & original_position[2];
#endif
  }

  position_type           position             = {};
  size_type               remaining_iterations = 0 ;
  dpa::relative_direction relative_direction   = center;

#ifdef DPA_FTLE_SUPPORT
  integer_type            original_rank        = 0 ;
  position_type           original_position    = position;
#endif
};

using particle_2d = particle<vector2, size>;
using particle_3d = particle<vector3, size>;
using particle_4d = particle<vector4, size>;
}

#endif
