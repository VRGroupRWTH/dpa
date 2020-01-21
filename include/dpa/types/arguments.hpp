#ifndef DPA_TYPES_ARGUMENTS_HPP
#define DPA_TYPES_ARGUMENTS_HPP

#include <optional>
#include <string>

#include <dpa/types/basic_types.hpp>

namespace dpa
{
struct arguments
{
  std::string          input_dataset_filepath               ;
  std::string          input_dataset_name                   ;
  std::string          input_dataset_spacing_name           ;
  vector3              seed_generation_stride               ;
  integer              seed_generation_iterations           ;
  std::optional<aabb3> seed_generation_boundaries           ;
  integer              particle_advector_particles_per_round;
  std::string          particle_advector_load_balancer      ;
  std::string          particle_advector_integrator         ;
  scalar               particle_advector_step_size          ;
  bool                 particle_advector_gather_particles   ;
  bool                 particle_advector_record             ;
  std::string          output_dataset_filepath              ;
};
}

#endif