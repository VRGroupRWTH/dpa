#include <dpa/stages/ftle_estimator.hpp>

#include <Eigen/Dense>

namespace dpa
{
regular_scalar_field_3d ftle_estimator::estimate(
  const regular_vector_field_3d&             original_vector_field  , 
  const std::size_t                          seed_maximum_iterations, 
  const vector3&                             seed_stride            , 
  const scalar                               step_size              , 
  const tbb::concurrent_vector<particle_3d>& local_particles        )     
{
#if DPA_FTLE_SUPPORT
  auto original_shape = original_vector_field.data.shape();
  auto strided_shape  = std::array<size, 3>
  {
    size(scalar(original_shape[0]) / seed_stride[0]),
    size(scalar(original_shape[1]) / seed_stride[1]),
    size(scalar(original_shape[2]) / seed_stride[2])
  };
  auto strided_spacing = original_vector_field.spacing.array() * seed_stride.array();

  auto flow_map = regular_vector_field_3d
  {
    boost::multi_array<vector3, 3>(strided_shape),
    original_vector_field.offset ,
    original_vector_field.size   ,
    strided_spacing
  };
  auto ftle_map = regular_scalar_field_3d
  {
    boost::multi_array<scalar, 3>(strided_shape),
    original_vector_field.offset ,
    original_vector_field.size   ,
    strided_spacing
  };

  tbb::parallel_for(std::size_t(0), local_particles.size(), std::size_t(1), [&] (const std::size_t index)
  {
    auto& particle = local_particles[index];
    flow_map.cell(particle.original_position) = particle.position;
  });
  
  auto flow_map_gradient = flow_map.gradient();
  flow_map_gradient.apply_parallel([&] (const std::array<std::size_t, 3>& index, const matrix3& value)
  {
    // Compute the spectral norm (right Cauchy-Green tensor).
    const matrix3 right_cauchy_green = value.transpose().eval() * value;
  
    // Compute eigenvalues and eigenvectors.
    const Eigen::SelfAdjointEigenSolver<matrix3> solver(right_cauchy_green);
    const auto eigenvalues  = solver.eigenvalues ();
    const auto eigenvectors = solver.eigenvectors();
  
    // Compute FTLE.
    ftle_map.data(index) = std::log(std::sqrt(eigenvalues.maxCoeff())) / std::abs(step_size * (seed_maximum_iterations - 0)); // TODO: Subtract remaining iterations of associated particle.
  });

  return ftle_map;
#else
  std::cout << "FTLE is not estimated since local_particles are unavailable. Declare DPA_FTLE_SUPPORT and rebuild." << std::endl;
  return regular_scalar_field_3d();
#endif
}
}
