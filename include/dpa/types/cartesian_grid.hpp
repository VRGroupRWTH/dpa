#ifndef DPA_TYPES_CARTESIAN_GRID_HPP
#define DPA_TYPES_CARTESIAN_GRID_HPP

#include <cmath>
#include <cstddef>
#include <vector>

#include <dpa/math/indexing.hpp>
#include <dpa/math/permute_for.hpp>

namespace dpa
{
template <typename element_type, typename domain_type, typename index_type>
struct cartesian_grid
{
  // Ducks [] and .size() on the domain_type.
  bool         contains   (const domain_type& position)
  {
    for (std::size_t i = 0; i < position.size(); ++i)
    {
      const auto subscript = std::floor((position[i] - offset[i]) / spacing[i]);
      if (std::size_t(0) > std::size_t(subscript) || std::size_t(subscript) >= std::size_t(dimensions[i] - 1))
        return false;
    }
    return true;
  }
  // Ducks [] and .size() on the domain_type.
  element_type interpolate(const domain_type& position)
  {
    domain_type weights    ;
    index_type  start_index;
    index_type  end_index  ;
    index_type  increment  ;

    for (std::size_t i = 0; i < position.size(); ++i)
    {
      weights    [i] = std::fmod ((position[i] - offset[i]) , spacing[i]) / spacing[i];
      start_index[i] = std::floor((position[i] - offset[i]) / spacing[i]);
      end_index  [i] = start_index[i] + 2;
      increment  [i] = 1;
    }

    std::vector<element_type> intermediates;
    intermediates.reserve(std::pow(2, position.size()));
    permute_for<index_type>([&] (const index_type& iteratee)
    {
      intermediates.push_back(data[ravel_multi_index(iteratee, dimensions)]);
    }, start_index, end_index, increment);

    for (std::int64_t i = position.size() - 1; i >= 0; --i)
      for (std::size_t j = 0; j < std::pow(2, i); ++j)
        intermediates[j] = (1 - weights[i]) * intermediates[2 * j] + weights[i] * intermediates[2 * j + 1];
    return intermediates[0];
  }

  std::vector<element_type> data      {};
  index_type                dimensions{};
  domain_type               offset    {};
  domain_type               size      {};
  domain_type               spacing   {};
};
}

#endif