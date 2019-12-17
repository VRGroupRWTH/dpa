#ifndef DPA_TYPES_PARTICLE_HPP
#define DPA_TYPES_PARTICLE_HPP

#include <cstdint>

#include <dpa/types/relative_direction.hpp>

namespace dpa
{
// Ducks [] on the position_type.
template <typename position_type, typename integer_type>
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
    archive & original_rank;
    archive & static_cast<std::uint32_t>(relative_direction);
  }

  position_type      position             = {};
  integer_type       remaining_iterations = 0 ;
  integer_type       original_rank        = 0 ;
  relative_direction relative_direction   = relative_direction::center;
};
}

#endif
