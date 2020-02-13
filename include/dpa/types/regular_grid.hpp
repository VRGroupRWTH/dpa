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
  static constexpr auto dimensions = _dimensions;

  using element_type               = _element_type;
                                   
  using domain_type                = typename vector_traits<scalar, dimensions>::type;
  using index_type                 = std::array<std::size_t, dimensions>;
  using container_type             = boost::multi_array<element_type, dimensions>;
  using array_view_type            = typename container_type::template array_view<dimensions>::type;

  using type                       = regular_grid<element_type, dimensions>;
  using gradient_type              = regular_grid<typename gradient_traits <element_type, dimensions>::type, dimensions>;
  using potential_type             = regular_grid<typename potential_traits<element_type, dimensions>::type, dimensions>;
  using second_gradient_type       = regular_grid<typename gradient_traits <typename gradient_traits <element_type, dimensions>::type, dimensions>::type, dimensions>;
  using second_potential_type      = regular_grid<typename potential_traits<typename potential_traits<element_type, dimensions>::type, dimensions>::type, dimensions>;
  using eigen_decomposition_type   = regular_grid<std::pair<element_type, typename potential_traits<element_type, dimensions>::type>, dimensions>;

  // Ducks [] on the domain_type.
  index_type               cell_index                    (const domain_type& position) const
  {
    index_type index;
    for (std::size_t i = 0; i < dimensions; ++i)
      index[i] = std::floor((position[i] - offset[i]) / spacing[i]);
    return index;
  }
  // Ducks [] on the domain_type.
  element_type&            cell                          (const domain_type& position)
  {
    return data(cell_index(position));
  }
  // Ducks [] on the domain_type.
  bool                     contains                      (const domain_type& position) const
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
  element_type             interpolate                   (const domain_type& position) const
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
  
  void                     apply                         (std::function<void(const index_type&, element_type&)> function)
  {
    index_type start_index; start_index.fill(0);
    index_type end_index  ;
    index_type increment  ; increment  .fill(1);
    for (std::size_t i = 0; i < dimensions; ++i)
      end_index[i] = data.shape()[i];
    permute_for<index_type>([&] (const index_type& index) { function(index, data(index)); }, start_index, end_index, increment);
  }
  void                     apply_parallel                (std::function<void(const index_type&, element_type&)> function)
  {
    index_type start_index; start_index.fill(0);
    index_type end_index  ;
    index_type increment  ; increment  .fill(1);
    for (std::size_t i = 0; i < dimensions; ++i)
      end_index[i] = data.shape()[i];
    parallel_permute_for<index_type>([&] (const index_type& index) { function(index, data(index)); }, start_index, end_index, increment);
  }
  void                     apply_window                  (std::function<void(const index_type&, element_type&, array_view_type&)> function, const index_type& window_size)
  {
    apply([&] (const index_type& index, element_type& element)
    {
      apply_window_internal(function, window_size, index, element);
    });
  }
  void                     apply_window_parallel         (std::function<void(const index_type&, element_type&, array_view_type&)> function, const index_type& window_size)
  {
    apply_parallel([&] (const index_type& index, element_type& element)
    {
      apply_window_internal(function, window_size, index, element);
    });
  }
  void                     apply_window_internal         (std::function<void(const index_type&, element_type&, array_view_type&)> function, const index_type& window_size, const index_type& index, element_type& element)
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

  gradient_type            gradient                      (const bool normalize = true)
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
  potential_type           potential                     () const
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
  second_gradient_type     hessian                       ()
  {
    return gradient().gradient(); // In generalized formulation of the gradient, transpose is omitted.
  }
  // Scalar-only operator.
  type                     laplacian                     ()
  {
    type laplacian {type::container_type(reinterpret_cast<index_type const&>(*data.shape())), offset, size, spacing};

    auto hessian_grid = hessian();
    hessian_grid.apply_parallel([&] (const index_type& index, typename second_gradient_type::element_type& element)
    {
      laplacian.data(index) = element.trace();
    });

    return laplacian;
  }
  // Scalar-only operator.
  second_gradient_type     structure_tensor              (const index_type& window_size)
  {
    auto gradient_grid = gradient();

    second_gradient_type outer_product_grid {second_gradient_type::container_type(reinterpret_cast<index_type const&>(*data.shape())), offset, size, spacing};
    gradient_grid     .apply_parallel       ([&] (const index_type& index, typename gradient_type::element_type& element)
    {
      outer_product_grid.data(index) = element.transpose().eval() * element;
    });

    second_gradient_type structure_tensor   {second_gradient_type::container_type(reinterpret_cast<index_type const&>(*data.shape())), offset, size, spacing};
    outer_product_grid.apply_window_parallel([&] (const index_type& index, typename second_gradient_type::element_type& element, typename second_gradient_type::array_view_type& elements)
    {
      structure_tensor.data(index).setZero();
      dpa::iterate(elements, [&] (const typename second_gradient_type::element_type& iteratee)
      {
        structure_tensor.data(index).array() += (iteratee.array() / elements.num_elements()); // TODO: Adjustable weights instead of 1/elements?
      });
    }, window_size);

    return structure_tensor;
  }
  // Vector-only operator.
  potential_type           divergence                    ()
  {
    potential_type divergence {potential_type::container_type(reinterpret_cast<index_type const&>(*data.shape())), offset, size, spacing};

    auto gradient_grid = gradient();
    gradient_grid.apply_parallel([&] (const index_type& index, typename gradient_type::element_type& element)
    {
      divergence.data(index) = element.trace();
    });

    return divergence;
  }
  // Vector-only operator.
  type                     vorticity                     ()
  {
    type vorticity {type::container_type(reinterpret_cast<index_type const&>(*data.shape())), offset, size, spacing};

    auto gradient_grid = gradient();
    gradient_grid.apply_parallel([&] (const index_type& index, typename gradient_type::element_type& element)
    {
      vorticity.data(index)[0] = element(2, 1) - element(1, 2);
      vorticity.data(index)[1] = element(0, 2) - element(2, 0);
      vorticity.data(index)[2] = element(1, 0) - element(0, 1);
    });

    return vorticity;
  }
  // Vector-only operator.
  potential_type           q_criterion                   ()
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
  // Vector/tensor-only operator.
  void                     normalize                     ()
  {
    apply_parallel([ ] (const index_type& index, element_type& element)
    {
      scalar norm = element.norm();
      if (norm > std::numeric_limits<scalar>::epsilon()) 
        element /= norm;
    });
  }
  // Tensor-only operator.
  eigen_decomposition_type eigen_decomposition           ()
  {
    eigen_decomposition_type eigen_decomposition {eigen_decomposition_type::container_type(reinterpret_cast<index_type const&>(*data.shape())), offset, size, spacing};
    apply_parallel([&] (const index_type& index, element_type& element)
    {
      Eigen::EigenSolver<element_type> solver(element);
      eigen_decomposition.data(index) = std::make_pair(solver.eigenvectors().real(), solver.eigenvalues().real());
      // TODO: Sort descending.
    });
    return eigen_decomposition;
  }
  // Tensor-only operator.
  eigen_decomposition_type reoriented_eigen_decomposition(const index_type& window_size)
  {
    auto grid = eigen_decomposition();
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
  // Tensor-only operator.
  second_potential_type    fractional_anisotropy         ()
  {
    second_potential_type output {second_potential_type::container_type(reinterpret_cast<index_type const&>(*data.shape())), offset, size, spacing};
    apply_parallel([&] (const index_type& index, element_type& element)
    {
      auto lambda = element.eigenvalues();
      auto mean   = lambda.sum() / lambda.size();
      output.data(index) = std::sqrt(3.0 / 2.0) * (lambda - mean).norm() / lambda.norm();
    });
    return output;
  }
  // Tensor-only operator.
  second_potential_type    relative_anisotropy           ()
  {
    second_potential_type output {second_potential_type::container_type(reinterpret_cast<index_type const&>(*data.shape())), offset, size, spacing};
    apply_parallel([&] (const index_type& index, element_type& element)
    {
      auto lambda = element.eigenvalues();
      auto mean   = lambda.sum() / lambda.size();
      output.data(index) = (lambda - mean).norm() / std::sqrt(3) * mean;
    });
    return output;
  }
  // Tensor-only operator.
  second_potential_type    volume_ratio                  ()
  {
    
  }
  // Tensor-only operator.
  second_potential_type    axial_diffusivity             ()
  {
    second_potential_type axial_diffusivity {second_potential_type::container_type(reinterpret_cast<index_type const&>(*data.shape())), offset, size, spacing};
    apply_parallel([&] (const index_type& index, element_type& element)
    {
      auto lambda = element.eigenvalues();
      axial_diffusivity.data(index) = 
        std::sqrt(0.5)
        * std::sqrt(std::pow(lambda[0] - lambda[1], 2) + std::pow(lambda[1] - lambda[2], 2) + std::pow(lambda[2] - lambda[0], 2))
        / std::sqrt(std::pow(lambda[0]            , 2) + std::pow(lambda[1]            , 2) + std::pow(lambda[2]            , 2));
    });
    return axial_diffusivity;
  }
  // Tensor-only operator.
  second_potential_type    mean_diffusivity              ()
  {
    second_potential_type mean_diffusivity {second_potential_type::container_type(reinterpret_cast<index_type const&>(*data.shape())), offset, size, spacing};
    apply_parallel([&] (const index_type& index, element_type& element)
    {
      auto lambda = element.eigenvalues();
      mean_diffusivity.data(index) = 
        std::sqrt(0.5)
        * std::sqrt(std::pow(lambda[0] - lambda[1], 2) + std::pow(lambda[1] - lambda[2], 2) + std::pow(lambda[2] - lambda[0], 2))
        / std::sqrt(std::pow(lambda[0]            , 2) + std::pow(lambda[1]            , 2) + std::pow(lambda[2]            , 2));
    });
    return mean_diffusivity;
  }
  // Tensor-only operator.
  second_potential_type    relative_diffusivity          ()
  {
    second_potential_type relative_diffusivity {second_potential_type::container_type(reinterpret_cast<index_type const&>(*data.shape())), offset, size, spacing};
    apply_parallel([&] (const index_type& index, element_type& element)
    {
      auto lambda = element.eigenvalues();
      relative_diffusivity.data(index) = 
        std::sqrt(0.5)
        * std::sqrt(std::pow(lambda[0] - lambda[1], 2) + std::pow(lambda[1] - lambda[2], 2) + std::pow(lambda[2] - lambda[0], 2))
        / std::sqrt(std::pow(lambda[0]            , 2) + std::pow(lambda[1]            , 2) + std::pow(lambda[2]            , 2));
    });
    return relative_diffusivity;
  }

  container_type data    {};
  domain_type    offset  {};
  domain_type    size    {};
  domain_type    spacing {};
};
}

#endif