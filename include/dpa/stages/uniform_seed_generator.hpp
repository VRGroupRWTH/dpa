#ifndef DPA_STAGES_UNIFORM_SEED_GENERATOR_HPP
#define DPA_STAGES_UNIFORM_SEED_GENERATOR_HPP

#include <vector>

#include <dpa/types/basic_types.hpp>
#include <dpa/types/particle.hpp>

namespace dpa
{
class uniform_seed_generator
{
public:
  static std::vector<particle<vector3, integer>> generate(const vector3& offset, const vector3& size, const vector3& stride, integer iterations, integer process_index);
};
}

#endif