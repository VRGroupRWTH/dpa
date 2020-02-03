#include <dpa/stages/ftle_estimator.hpp>

namespace dpa
{
regular_scalar_field_3d ftle_estimator::estimate(const regular_vector_field_3d& original_vector_field, const vector3& seed_stride, const tbb::concurrent_vector<particle_3d>& local_particles)
{
  regular_scalar_field_3d scalar_field;
#if DPA_FTLE_SUPPORT
  scalar_field.offset = original_vector_field.offset;


  // Map the inactive particles to a vector field, scaled by the stride.
  // Find original voxel and subtract to obtain the flow map.
  // Compute the gradient of the flow map (generically).
  // Extract the Eigenvalues of the gradient, and take the maximum e1.
  // Compute FTLE for each voxel and assign to the field.

#else
  std::cout << "FTLE is not estimated since original ranks/positions are unavailable. Declare DPA_FTLE_SUPPORT and rebuild." << std::endl;
#endif
  return scalar_field;
}
}
