#ifndef DPA_TYPES_ARGUMENTS_HPP
#define DPA_TYPES_ARGUMENTS_HPP

#include <string>

#include <dpa/types/basic_types.hpp>

namespace dpa
{
struct arguments
{
  std::string dataset_filepath               ;
  std::string dataset_name                   ;
  std::string dataset_spacing_name           ;
  vector3     seed_generation_stride         ;
  integer     seed_generation_iterations     ;
  std::string particle_advector_load_balancer;
  std::string particle_advector_integrator   ;
  scalar      particle_advector_step_size    ;
  bool        particle_advector_record       ;
};
}

#endif