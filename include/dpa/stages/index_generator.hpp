#ifndef DPA_STAGES_INDEX_GENERATOR_HPP
#define DPA_STAGES_INDEX_GENERATOR_HPP

#include <dpa/types/integral_curves.hpp>

namespace dpa
{
class index_generator
{
public:
  static void generate(integral_curves& integral_curves, const bool use_64_bit = false);
};
}

#endif