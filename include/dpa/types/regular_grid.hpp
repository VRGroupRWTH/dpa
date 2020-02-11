#ifndef DPA_TYPES_REGULAR_GRID_HPP
#define DPA_TYPES_REGULAR_GRID_HPP

#include <array>
#include <cmath>
#include <cstddef>
#include <type_traits>
#include <vector>

#include <boost/multi_array.hpp>

#include <dpa/math/permute_for.hpp>
#include <dpa/types/basic_types.hpp>

namespace dpa
{
template <typename _element_type, std::size_t _dimensions>
struct regular_grid
{
  static constexpr auto dimensions = _dimensions;

  using element_type               = _element_type;
                                   
  using domain_type                = typename vector_traits<scalar  , dimensions>::type;
  using index_type                 = std::array        <std::size_t , dimensions>;
  using container_type             = boost::multi_array<element_type, dimensions>;
  using array_view_type            = typename container_type::template array_view<dimensions>::type;
                                   
  using gradient_type              = regular_grid<typename gradient_traits <element_type, dimensions>::type, dimensions>;
  using potential_type             = regular_grid<typename potential_traits<element_type, dimensions>::type, dimensions>;
  using structure_tensor_type      = regular_grid<typename gradient_traits <typename gradient_traits<element_type, dimensions>::type, dimensions>::type, dimensions>;
  using hessian_type               = regular_grid<typename gradient_traits <typename gradient_traits<element_type, dimensions>::type, dimensions>::type, dimensions>;
  using laplacian_type             = regular_grid<domain_type, dimensions>;

  // Ducks [] on the domain_type.
  index_type            cell_index         (const domain_type& position) const
  {
    index_type index;
    for (std::size_t i = 0; i < dimensions; ++i)
      index[i] = std::floor((position[i] - offset[i]) / spacing[i]);
    return index;
  }
  // Ducks [] on the domain_type.
  element_type&         cell               (const domain_type& position)
  {
    return data(cell_index(position));
  }
  // Ducks [] on the domain_type.
  bool                  contains           (const domain_type& position) const
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
  element_type          interpolate        (const domain_type& position) const
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

  void                  apply              (std::function<void(const index_type&, element_type&)> function)
  {
    index_type start_index; start_index.fill(0);
    index_type end_index  ;
    index_type increment  ; increment  .fill(1);
    for (std::size_t i = 0; i < dimensions; ++i)
      end_index[i] = data.shape()[i];
    parallel_permute_for<index_type>([&] (const index_type& index) { function(index, data(index)); }, start_index, end_index, increment);
  }
  void                  apply_window       (std::function<void(const index_type&, element_type&, array_view_type&)> function, const index_type& window_size)
  {
    apply([&] (const index_type& index, element_type& element)
    {
      boost::detail::multi_array::index_gen<dimensions, dimensions> indices;
      for (auto dimension = 0; dimension < dimensions; ++dimension)
      {
        auto& range = indices.ranges_[dimension];

        if (index[dimension] > std::floor(window_size[dimension] / scalar(2)))
          range.start (index[dimension] - std::floor(window_size[dimension] / scalar(2)));
        else
          range.start (0);
        
        if (index[dimension] + std::ceil (window_size[dimension] / scalar(2)) < data.shape()[dimension])
          range.finish(index[dimension] + std::ceil (window_size[dimension] / scalar(2)));
        else
          range.finish(data.shape()[dimension]);
      }
      function(index, element, data[indices]);
    });
  }

  gradient_type         gradient           ()
  {
    auto& shape       = reinterpret_cast<index_type const&>(*data.shape());
    auto  two_spacing = domain_type(2 * spacing);

    gradient_type gradient {gradient_type::container_type(shape), offset, size, spacing};
    gradient.apply([&] (const index_type& index, typename gradient_type::element_type& element)
    {
      for (std::size_t dimension = 0; dimension < dimensions; ++dimension)
      {
        auto prev_index = index, next_index = index;
        if (index[dimension] > 0)                    prev_index[dimension] -= 1;
        if (index[dimension] < shape[dimension] - 1) next_index[dimension] += 1;

        // TODO: Extend to 3rd+ order tensors via <unsupported/Eigen/CXX11/Tensor>.
        element.col(dimension).array() = (data(next_index) - data(prev_index)) / two_spacing[dimension];
      }
    });
    return gradient;
  }
  potential_type        potential          () const
  {
    auto& shape       = reinterpret_cast<index_type const&>(*data.shape());
    auto  two_spacing = domain_type(2 * spacing);
    
    index_type start_index; start_index.fill(0);
    index_type end_index  ; end_index  .fill(1);
    index_type increment  ; increment  .fill(1);

    potential_type potential {potential_type::container_type(shape), offset, size, spacing};
    for (std::size_t dimension = 0; dimension < dimensions; ++dimension)
    {
      for (std::size_t serial_index = 1; serial_index < shape[dimension]; ++serial_index)
      {
        auto partial_start_index = start_index; partial_start_index[dimension] = serial_index    ;
        auto partial_end_index   = end_index  ; partial_end_index  [dimension] = serial_index + 1;

        parallel_permute_for<index_type>([&] (const index_type& index)
        {
          auto prev_index = index, next_index = index;
          if (index[dimension] > 0)                    prev_index[dimension] -= 1;
          if (index[dimension] < shape[dimension] - 1) next_index[dimension] += 1;

          // TODO: Extend to 3rd+ order tensors via <unsupported/Eigen/CXX11/Tensor>.
          if constexpr (std::is_arithmetic<typename potential_type::element_type>::value)
            potential.data(index) = potential.data(prev_index) + two_spacing[dimension] * potential_type::element_type((data(prev_index).col(dimension).array() + data(index).col(dimension).array()).value());
          else                                           
            potential.data(index) = potential.data(prev_index) + two_spacing[dimension] * potential_type::element_type( data(prev_index).col(dimension).array() + data(index).col(dimension).array());
        }, partial_start_index, partial_end_index, increment);
      }
      end_index[dimension] = shape[dimension];
    }
    return potential;
  }

  // Scalar-only operator.
  hessian_type          hessian            ()
  {
    return gradient().gradient(); // In generalized formulation of the gradient, transpose is omitted.
  }
  // Scalar-only operator.
  laplacian_type        laplacian          ()
  {
    laplacian_type laplacian {laplacian_type::container_type(reinterpret_cast<index_type const&>(*data.shape())), offset, size, spacing};

    auto hessian_grid = hessian();
    hessian_grid.apply([ ] (const index_type& index, typename hessian_type::element_type& element)
    {
      laplacian[index] = element.trace();
    });

    return laplacian;
  }
  // Scalar-only operator.
  structure_tensor_type structure_tensor   (const index_type& window_size)
  {
    auto gradient_grid = gradient();

    structure_tensor_type outer_product_grid {structure_tensor_type::container_type(reinterpret_cast<index_type const&>(*data.shape())), offset, size, spacing};
    gradient_grid     .apply       ([&] (const index_type& index, typename gradient_type::element_type& element)
    {
      outer_product_grid.data(index) = element.transpose().eval() * element;
    });

    structure_tensor_type structure_tensor   {structure_tensor_type::container_type(reinterpret_cast<index_type const&>(*data.shape())), offset, size, spacing};
    outer_product_grid.apply_window([&] (const index_type& index, typename structure_tensor_type::element_type& element, typename structure_tensor_type::array_view_type& elements)
    {
      structure_tensor.data(index).setZero();

      index_type start_index; start_index.fill(0);
      index_type end_index  = window_size;
      index_type increment  ; increment  .fill(1);
      permute_for<index_type>([&] (const index_type& relative_index)
      {
        structure_tensor.data(index).array() += (elements(relative_index).array() / elements.num_elements()); // TODO: Adjustable weights instead of 1/elements?
      }, start_index, end_index, increment);  
    }, window_size);

    return structure_tensor;
  }
  // Tensor-only operator.
  void                  orient_eigenvectors(const index_type& window_size)
  {
    apply_window([&](const index_type& index, element_type& element, array_view_type& elements)
    {
      
    });
  }

  container_type data    {};
  domain_type    offset  {};
  domain_type    size    {};
  domain_type    spacing {};
};
}

#endif