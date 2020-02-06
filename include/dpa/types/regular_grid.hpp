#ifndef DPA_TYPES_REGULAR_GRID_HPP
#define DPA_TYPES_REGULAR_GRID_HPP

#include <array>
#include <cmath>
#include <cstddef>
#include <type_traits>
#include <vector>

#include <boost/multi_array.hpp>

#include <dpa/math/indexing.hpp>
#include <dpa/math/permute_for.hpp>
#include <dpa/types/basic_types.hpp>

namespace dpa
{
template <typename _element_type, std::size_t _dimensions>
struct regular_grid
{
  static constexpr std::size_t dimensions = _dimensions;

  using element_type = _element_type;
  using domain_type  = typename vector_traits<scalar, dimensions>::type;
  using index_type   = std::array<std::size_t, dimensions>;

  // Ducks [] on the domain_type.
  index_type    cell_index (const domain_type& position) const
  {
    index_type index;
    for (std::size_t i = 0; i < dimensions; ++i)
      index[i] = std::floor((position[i] - offset[i]) / spacing[i]);
    return index;
  }
  // Ducks [] on the domain_type.
  element_type& cell       (const domain_type& position)
  {
    return data(cell_index(position));
  }
  // Ducks [] on the domain_type.
  bool          contains   (const domain_type& position) const
  {
    for (std::size_t i = 0; i < dimensions; ++i)
    {
      const auto subscript = std::floor((position[i] - offset[i]) / spacing[i]);
      if (std::int64_t(0) > std::int64_t(subscript) || std::size_t(subscript) >= data.shape()[i] - 1)
        return false;
    }
    return true;
  }
  // Ducks [] on the domain_type.
  element_type  interpolate(const domain_type& position) const
  {
    domain_type weights    ;
    index_type  start_index;
    index_type  end_index  ;
    index_type  increment  ;

    for (std::size_t i = 0; i < dimensions; ++i)
    {
      weights    [i] = std::fmod ((position[i] - offset[i]) , spacing[i]) / spacing[i];
      start_index[i] = std::floor((position[i] - offset[i]) / spacing[i]);
      end_index  [i] = start_index[i] + 2;
      increment  [i] = 1;
    }

    std::vector<element_type> intermediates;
    intermediates.reserve(std::pow(2, dimensions));
    permute_for<index_type>([&] (const index_type& index) { intermediates.push_back(data(index)); }, start_index, end_index, increment);

    for (std::int64_t i = dimensions - 1; i >= 0; --i)
      for (std::size_t j = 0; j < std::pow(2, i); ++j)
        intermediates[j] = (scalar(1) - weights[i]) * intermediates[2 * j] + weights[i] * intermediates[2 * j + 1];
    return intermediates[0];
  }

  void          apply      (std::function<void(const index_type&, element_type&)> function)
  {
    index_type start_index; start_index.fill(0);
    index_type end_index  ;
    index_type increment  ; increment  .fill(1);
    for (std::size_t i = 0; i < dimensions; ++i)
      end_index[i] = data.shape()[i];
    parallel_permute_for<index_type>([&] (const index_type& index) { function(index, data(index)); }, start_index, end_index, increment);
  }

  regular_grid<typename gradient_traits <element_type, dimensions>::type, dimensions> gradient ()
  {
    using gradient_type = regular_grid<typename gradient_traits<element_type, dimensions>::type, dimensions>;

    auto& shape       = reinterpret_cast<std::array<std::size_t, dimensions> const&>(*data.shape());
    auto  two_spacing = 2 * spacing;

    gradient_type gradient;
    gradient.data.resize(shape);
    gradient.apply([&] (const index_type& index, typename gradient_type::element_type& element)
    {
      for (std::size_t dimension = 0; dimension < dimensions; ++dimension)
      {
        auto prev_index = index, next_index = index;
        if (index[dimension] > 0)                     prev_index[dimension] -= 1;
        if (index[dimension] < shape[dimension] - 1)  next_index[dimension] += 1;

        element.col(dimension) = (data(next_index) - data(prev_index)) / two_spacing[dimension]; // TODO: Extend to 3rd+ order tensors via <unsupported/Eigen/CXX11/Tensor>
      }
    });
    return gradient;
  }
  regular_grid<typename potential_traits<element_type, dimensions>::type, dimensions> potential()
  {
    using potential_type = regular_grid<typename potential_traits<element_type, dimensions>::type, dimensions>;

    auto& shape = reinterpret_cast<std::array<std::size_t, dimensions> const&>(*data.shape());

    potential_type potential;
    potential.data.resize(shape);

    for (std::size_t dimension = 0; dimension < dimensions; ++dimension)
    {
      // Initial condition: potentials[0,0,0] = 0.
      // For i=1...n, j=0    , k=0    : potentials[i, 0, 0] = potentials[i - 1, 0    , 0    ] + spacing.x * 0.5 * (vectors[i - 1, 0    , 0    ].x + vectors[i, 0, 0].x).
      // For i=1...n, j=1...n, k=0    : potentials[i, j, 0] = potentials[i    , j - 1, 0    ] + spacing.y * 0.5 * (vectors[i    , j - 1, 0    ].y + vectors[i, j, 0].y).
      // For i=1...n, j=1...n, k=1...n: potentials[i, j, k] = potentials[i    , j    , k - 1] + spacing.z * 0.5 * (vectors[i    , j    , k - 1].z + vectors[i, j, k].z).
    }

    return potential;
  }

  boost::multi_array<element_type, dimensions> data    {};
  domain_type                                  offset  {};
  domain_type                                  size    {};
  domain_type                                  spacing {};
};
}

#endif