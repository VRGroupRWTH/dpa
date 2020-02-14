#ifndef DPA_TYPES_REGULAR_GRID_HPP
#define DPA_TYPES_REGULAR_GRID_HPP

#include <array>
#include <cmath>
#include <cstddef>
#include <type_traits>
#include <vector>

#include <boost/multi_array.hpp>
#include <Eigen/Dense>

#include <dpa/math/permute_for.hpp>
#include <dpa/types/basic_types.hpp>
#include <dpa/utility/multi_array.hpp>

namespace dpa
{
template <typename _element_type, std::size_t _dimensions>
struct regular_grid
{
  static constexpr auto dimensions         = _dimensions;

  using element_type                       = _element_type;
  using complex_type                       = ???             
                                   
  using domain_type                        = typename vector_traits<scalar, dimensions>::type;
  using index_type                         = std::array<std::size_t, dimensions>;
  using container_type                     = boost::multi_array<element_type, dimensions>;
  using array_view_type                    = typename container_type::template array_view<dimensions>::type;
                                           
  using type                               = regular_grid<element_type, dimensions>;
  using gradient_type                      = regular_grid<typename gradient_traits <element_type, dimensions>::type, dimensions>;
  using potential_type                     = regular_grid<typename potential_traits<element_type, dimensions>::type, dimensions>;
  using second_gradient_type               = regular_grid<typename gradient_traits <typename gradient_traits <element_type, dimensions>::type, dimensions>::type, dimensions>;
  using second_potential_type              = regular_grid<typename potential_traits<typename potential_traits<element_type, dimensions>::type, dimensions>::type, dimensions>;
  ???using eigen_decomposition_type           = std::pair<regular_grid<complex_type, dimensions>, regular_grid<typename potential_traits<element_type, dimensions>::complex_type, dimensions>>;
  ???using symmetric_eigen_decomposition_type = std::pair<regular_grid<element_type, dimensions>, regular_grid<typename potential_traits<element_type, dimensions>::type, dimensions>>;

  /// Access.
  
  index_type                         cell_index                    (const domain_type& position) const
  {
    index_type index;
    for (std::size_t i = 0; i < dimensions; ++i)
      index[i] = std::floor((position[i] - offset[i]) / spacing[i]);
    return index;
  }
  element_type&                      cell                          (const domain_type& position)
  {
    return data(cell_index(position));
  }
  bool                               contains                      (const domain_type& position) const
  {
    for (std::size_t i = 0; i < dimensions; ++i)
    {
      const auto subscript = std::floor((position[i] - offset[i]) / spacing[i]);
      if (std::int64_t(0) > std::int64_t(subscript) || std::size_t(subscript) >= data.shape()[i] - 1)
        return false;
    }
    return true;
  }
  element_type                       interpolate                   (const domain_type& position) const
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

  /// Iteration.

  void                               apply                         (std::function<void(const index_type&, element_type&)> function)
  {
    index_type start_index; start_index.fill(0);
    index_type end_index  ;
    index_type increment  ; increment  .fill(1);
    for (std::size_t i = 0; i < dimensions; ++i)
      end_index[i] = data.shape()[i];
    permute_for<index_type>([&] (const index_type& index) { function(index, data(index)); }, start_index, end_index, increment);
  }
  void                               apply_parallel                (std::function<void(const index_type&, element_type&)> function)
  {
    index_type start_index; start_index.fill(0);
    index_type end_index  ;
    index_type increment  ; increment  .fill(1);
    for (std::size_t i = 0; i < dimensions; ++i)
      end_index[i] = data.shape()[i];
    parallel_permute_for<index_type>([&] (const index_type& index) { function(index, data(index)); }, start_index, end_index, increment);
  }
  void                               apply_window                  (std::function<void(const index_type&, element_type&, array_view_type&)> function, const index_type& window_size)
  {
    apply([&] (const index_type& index, element_type& element)
    {
      apply_window_internal(function, window_size, index, element);
    });
  }
  void                               apply_window_parallel         (std::function<void(const index_type&, element_type&, array_view_type&)> function, const index_type& window_size)
  {
    apply_parallel([&] (const index_type& index, element_type& element)
    {
      apply_window_internal(function, window_size, index, element);
    });
  }
  void                               apply_window_internal         (std::function<void(const index_type&, element_type&, array_view_type&)> function, const index_type& window_size, const index_type& index, element_type& element)
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
  }

  /// Derivatives/integrals through central differences.
  
  // Scalar/vector operator.
  gradient_type                      gradient                      (const bool normalize = true)
  {
    auto& shape       = reinterpret_cast<index_type const&>(*data.shape());
    auto  two_spacing = domain_type(2 * spacing);

    gradient_type gradient {gradient_type::container_type(shape), offset, size, spacing};
    gradient.apply_parallel([&] (const index_type& index, typename gradient_type::element_type& element)
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

    if (normalize)
      gradient.normalize();

    return gradient;
  }
  // Scalar operator.
  second_gradient_type               second_gradient               (const bool normalize = true)
  {
    return gradient(normalize).gradient(normalize);
  }
  // Vector/tensor operator.
  potential_type                     potential                     () const
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
  // Tensor operator.
  second_potential_type              second_potential              () const
  {
    return potential().potential();
  }

  /// Moments.
  
  // Vector operator.
  ???gradient_type                      structure_tensor              (const index_type& window_size)
  {
    gradient_type outer_product_grid {gradient_type::container_type(reinterpret_cast<index_type const&>(*data.shape())), offset, size, spacing};
    apply_parallel([&] (const index_type& index, typename gradient_type::element_type& element)
    {
      outer_product_grid.data(index) = element.transpose().eval() * element;
    });

    gradient_type structure_tensor   {gradient_type::container_type(reinterpret_cast<index_type const&>(*data.shape())), offset, size, spacing};
    outer_product_grid.apply_window_parallel([&] (const index_type& index, typename gradient_type::element_type& element, typename gradient_type::array_view_type& elements)
    {
      structure_tensor.data(index).setZero();
      dpa::iterate(elements, [&] (const typename gradient_type::element_type& iteratee)
      {
        structure_tensor.data(index).array() += (iteratee.array() / elements.num_elements()); // TODO: Adjustable weights instead of 1/elements?
      });
    }, window_size);

    return structure_tensor;
  }
  
  /// Convenience.
  
  // Vector/tensor operator.
  void                               normalize                     ()
  {
    apply_parallel([ ] (const index_type& index, element_type& element)
    {
      scalar norm = element.norm();
      if (norm > std::numeric_limits<scalar>::epsilon()) 
        element /= norm;
    });
  }
  // Tensor operator.
  second_potential_type              trace                         ()
  {
    second_potential_type trace {second_potential_type::container_type(reinterpret_cast<index_type const&>(*data.shape())), offset, size, spacing};
    apply_parallel([&] (const index_type& index, element_type& element)
    {
      trace.data(index) = element.trace();
    });
    return trace;
  }
  // Tensor operator.
  ??eigen_decomposition_type           eigen_decomposition           ()
  {
    eigen_decomposition_type eigen_decomposition {eigen_decomposition_type::container_type(reinterpret_cast<index_type const&>(*data.shape())), offset, size, spacing};
    apply_parallel([&] (const index_type& index, element_type& element)
    {
      Eigen::EigenSolver<element_type> solver(element);
      eigen_decomposition.data(index) = std::make_pair(solver.eigenvectors(), solver.eigenvalues());
    });
    return eigen_decomposition;
  }
  // Tensor operator.
  ??symmetric_eigen_decomposition_type symmetric_eigen_decomposition ()
  {
    eigen_decomposition_type eigen_decomposition {eigen_decomposition_type::container_type(reinterpret_cast<index_type const&>(*data.shape())), offset, size, spacing};
    apply_parallel([&] (const index_type& index, element_type& element)
    {
      Eigen::SelfAdjointEigenSolver<element_type> solver(element);
      eigen_decomposition.data(index) = std::make_pair(solver.eigenvectors(), solver.eigenvalues());
    });
    return eigen_decomposition;
  }
  // Tensor operator. Requires symmetry.
  ??symmetric_eigen_decomposition_type reoriented_eigen_decomposition(const index_type& window_size)
  {
    auto grid = symmetric_eigen_decomposition();
    grid.apply_window([&] (const index_type& index, typename eigen_decomposition_type::element_type& element, typename eigen_decomposition_type::array_view_type& elements)
    {
      auto counter  = 0;
      auto matrices = std::vector<Eigen::MatrixXf>(dimensions, Eigen::MatrixXf(dimensions, elements.num_elements()));
      dpa::iterate(elements, [&] (const typename eigen_decomposition_type::element_type& iteratee)
      {
        for (auto i = 0; i < dimensions; ++i)
          matrices[i].col(counter) = iteratee.first.col(i);
        counter++;
      });

      for (auto i = 0; i < dimensions; ++i)
      {
        auto svd = matrices[i].jacobiSvd(Eigen::ComputeFullU | Eigen::ComputeFullV);
        dpa::iterate(elements, [&] (typename eigen_decomposition_type::element_type& iteratee)
        {
          if (iteratee.first.col(i).dot(svd.matrixU().col(i)) < 0)
            iteratee.first.col(i) *= -1;
        });
      }
    }, window_size);
    return grid;
  }
  
  /// Special operators.
  
  // Scalar operator.
  second_gradient_type               hessian                       ()
  {
    return second_gradient(); // In generalized formulation of the gradient, transpose is omitted.
  }
  // Scalar operator.
  ??type                               laplacian                     ()
  {
    return second_gradient().trace();
  }
  // Tensor operator. Applied to a vector field gradient.
  ??second_potential_type              divergence                    ()
  {
    return trace();
  }
  // Tensor operator. Applied to a 3x3 vector field gradient.
  ??potential_type                     vorticity                     ()
  {
    potential_type vorticity {potential_type::container_type(reinterpret_cast<index_type const&>(*data.shape())), offset, size, spacing};
    apply_parallel([&] (const index_type& index, element_type& element)
    {
      vorticity.data(index)[0] = element(2, 1) - element(1, 2);
      vorticity.data(index)[1] = element(0, 2) - element(2, 0);
      vorticity.data(index)[2] = element(1, 0) - element(0, 1);
    });
    return vorticity;
  }
  // Tensor operator. Applied to a 3x3 vector field gradient.
  ??potential_type                     q_criterion                   ()
  {
    potential_type q_criterion {potential_type::container_type(reinterpret_cast<index_type const&>(*data.shape())), offset, size, spacing};

    auto gradient_grid = gradient();
    gradient_grid.apply_parallel([&] (const index_type& index, typename gradient_type::element_type& element)
    {
      q_criterion.data(index) = 
        - (element(0, 0) * element(0, 0) + element(1, 1) * element(1, 1) + element(2, 2)) / scalar(2)
        - (element(0, 1) * element(1, 0) + element(0, 2) * element(2, 0) + element(1, 2) * element(2, 1));
    });

    return q_criterion;
  }
  ??void vector_laplacian()
  {
    
  }
  // Tensor operator. Requires symmetry.
  second_potential_type              axial_diffusivity             ()
  {
    second_potential_type output {second_potential_type::container_type(reinterpret_cast<index_type const&>(*data.shape())), offset, size, spacing};
    apply_parallel([&] (const index_type& index, element_type& element)
    {
      auto lambda = element.eigenvalues().real();
      std::sort(lambda.data(), lambda.data() + lambda.size(), std::greater<typename second_potential_type::element_type>());
      output.data(index) = lambda[0];
    });
    return output;
  }
  // Tensor operator. Requires symmetry.
  second_potential_type              radial_diffusivity            ()
  {
    second_potential_type output {second_potential_type::container_type(reinterpret_cast<index_type const&>(*data.shape())), offset, size, spacing};
    apply_parallel([&] (const index_type& index, element_type& element)
    {
      auto lambda = element.eigenvalues().real();
      std::sort(lambda.data(), lambda.data() + lambda.size(), std::greater<typename second_potential_type::element_type>());
      output.data(index) = (lambda.sum() - lambda[0]) / 2;
    });
    return output;
  }
  // Tensor operator. Requires symmetry.
  second_potential_type              mean_diffusivity              ()
  {
    second_potential_type output {second_potential_type::container_type(reinterpret_cast<index_type const&>(*data.shape())), offset, size, spacing};
    apply_parallel([&] (const index_type& index, element_type& element)
    {
      auto lambda = element.eigenvalues().real();
      output.data(index) = lambda.sum() / lambda.size();
    });
    return output;
  }
  // Tensor operator. Requires symmetry.
  second_potential_type              fractional_anisotropy         ()
  {
    second_potential_type output {second_potential_type::container_type(reinterpret_cast<index_type const&>(*data.shape())), offset, size, spacing};
    apply_parallel([&] (const index_type& index, element_type& element)
    {
      auto lambda = element.eigenvalues().real();
      auto mean   = lambda.sum() / lambda.size();
      output.data(index) = std::sqrt(lambda.size() / scalar(2)) * potential_type::element_type(lambda.array() - mean).norm() / lambda.norm();
    });
    return output;
  }
  // Tensor operator. Requires symmetry.
  second_potential_type              relative_anisotropy           ()
  {
    second_potential_type output {second_potential_type::container_type(reinterpret_cast<index_type const&>(*data.shape())), offset, size, spacing};
    apply_parallel([&] (const index_type& index, element_type& element)
    {
      auto lambda = element.eigenvalues().real();
      auto mean   = lambda.sum() / lambda.size();
      output.data(index) = potential_type::element_type(lambda.array() - mean).norm() / (std::sqrt(lambda.size()) * mean);
    });
    return output;
  }
  // Tensor operator. Requires symmetry.
  second_potential_type              volume_ratio                  ()
  {
    second_potential_type output {second_potential_type::container_type(reinterpret_cast<index_type const&>(*data.shape())), offset, size, spacing};
    apply_parallel([&] (const index_type& index, element_type& element)
    {
      auto lambda = element.eigenvalues().real();
      auto mean   = lambda.sum() / lambda.size();
      output.data(index) = lambda.prod() / std::pow(mean, lambda.size());
    });
    return output;
  }

  container_type data    {};
  domain_type    offset  {};
  domain_type    size    {};
  domain_type    spacing {};
};
}

#endif