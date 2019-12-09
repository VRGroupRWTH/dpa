#ifndef DPA_STAGES_PARTICLE_ADVECTOR_HPP
#define DPA_STAGES_PARTICLE_ADVECTOR_HPP

#include <string>

#include <dpa/stages/domain_partitioner.hpp>
#include <dpa/types/basic_types.hpp>
#include <dpa/types/integral_curves.hpp>
#include <dpa/types/integrators.hpp>
#include <dpa/types/particle.hpp>
#include <dpa/types/regular_fields.hpp>

namespace dpa
{
class particle_advector
{
public:
  enum class load_balancer
  {
    none,
    diffuse
  };

  explicit particle_advector  (domain_partitioner* partitioner, const std::string& load_balancer, const std::string& integrator, const scalar step_size, const bool record);
  particle_advector           (const particle_advector&  that) = delete ;
  particle_advector           (      particle_advector&& temp) = default;
 ~particle_advector           ()                               = default;
  particle_advector& operator=(const particle_advector&  that) = delete ;
  particle_advector& operator=(      particle_advector&& temp) = default;

  integral_curves_3d advect(const std::unordered_map<relative_direction, regular_vector_field_3d>& vector_fields, std::vector<particle<vector3, integer>>& seeds);

protected:
  domain_partitioner*        partitioner_   {};
  load_balancer              load_balancer_ {};
  variant_vector3_integrator integrator_    {};
  scalar                     step_size_     {};
  bool                       record_        {};
};
}

#endif