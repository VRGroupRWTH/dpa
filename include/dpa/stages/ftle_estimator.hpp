#ifndef DPA_STAGES_FTLE_ESTIMATOR_HPP
#define DPA_STAGES_FTLE_ESTIMATOR_HPP

#include <tbb/tbb.h>

#include <dpa/types/particle.hpp>
#include <dpa/types/regular_fields.hpp>

namespace dpa
{
class ftle_estimator
{
public:
  static regular_scalar_field_3d estimate(
    const regular_vector_field_3d&             original_vector_field  , 
    const std::size_t                          seed_maximum_iterations, 
    const vector3&                             seed_stride            , 
    const scalar                               step_size              , 
    const tbb::concurrent_vector<particle_3d>& local_particles        );
};
}

#endif