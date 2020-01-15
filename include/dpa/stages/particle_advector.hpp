#ifndef DPA_STAGES_PARTICLE_ADVECTOR_HPP
#define DPA_STAGES_PARTICLE_ADVECTOR_HPP

#include <cstddef>
#include <string>
#include <unordered_map>
#include <vector>

#include <tbb/concurrent_hash_map.h>

#include <dpa/stages/domain_partitioner.hpp>
#include <dpa/types/basic_types.hpp>
#include <dpa/types/integral_curves.hpp>
#include <dpa/types/integrators.hpp>
#include <dpa/types/particle.hpp>
#include <dpa/types/regular_fields.hpp>

namespace dpa
{
class pipeline;

class particle_advector
{
public:
  enum class load_balancer
  {
    none,
    diffuse_constant,
    diffuse_lesser_average,
    diffuse_greater_limited_lesser_average
  };

  struct round_info
  {
    using particle_map = tbb::concurrent_hash_map<relative_direction, std::vector<particle<vector3, integer>>>;

    std::size_t  particle_count                   = 0;
    std::size_t  curve_stride                     = 0;
    std::size_t  vertex_count                     = 0;
    particle_map out_of_bounds_particles          {};
    particle_map neighbor_out_of_bounds_particles {};
  };
  struct load_balancing_info
  {
    // Function for boost::serialization which is used by boost::mpi.
    template<class archive_type>
    void serialize(archive_type& archive, const std::uint32_t version)
    {
      archive & particle_count;
    }

    std::size_t  particle_count;
  };
  struct output
  {
    std::vector<particle<vector3, integer>> particles       {};
    integral_curves_3d                      integral_curves {};
  };

  explicit particle_advector  (domain_partitioner* partitioner, const integer particles_per_round, const std::string& load_balancer, const std::string& integrator, const scalar step_size, const bool gather_particles, const bool record);
  particle_advector           (const particle_advector&  that) = delete ;
  particle_advector           (      particle_advector&& temp) = default;
 ~particle_advector           ()                               = default;
  particle_advector& operator=(const particle_advector&  that) = delete ;
  particle_advector& operator=(      particle_advector&& temp) = default;

  output             advect                  (const std::unordered_map<relative_direction, regular_vector_field_3d>& vector_fields,       std::vector<particle<vector3, integer>>& particles);
  
protected:
  friend pipeline; // For benchmarking of individual steps.

  bool               check_completion        (                                                                                      const std::vector<particle<vector3, integer>>& active_particles);
  void               load_balance_distribute (                                                                                            std::vector<particle<vector3, integer>>& active_particles);
  round_info         compute_round_info      (                                                                                      const std::vector<particle<vector3, integer>>& active_particles,                                                              const integral_curves_3d& integral_curves);
  void               allocate_integral_curves(                                                                                      const std::vector<particle<vector3, integer>>& active_particles,                                                                    integral_curves_3d& integral_curves, const round_info& round_info);
  void               advect                  (const std::unordered_map<relative_direction, regular_vector_field_3d>& vector_fields,       std::vector<particle<vector3, integer>>& active_particles, std::vector<particle<vector3, integer>>& inactive_particles,       integral_curves_3d& integral_curves,       round_info& round_info);
  void               load_balance_collect    (const std::unordered_map<relative_direction, regular_vector_field_3d>& vector_fields,                                                                  std::vector<particle<vector3, integer>>& inactive_particles,                                                  round_info& round_info);
  void               out_of_bounds_distribute(                                                                                            std::vector<particle<vector3, integer>>& active_particles,                                                                                                         const round_info& round_info);
  void               gather_particles        (                                                                                                                                                       std::vector<particle<vector3, integer>>& inactive_particles);
  void               prune_integral_curves   (                                                                                                                                                                                                                          integral_curves_3d& integral_curves);

  domain_partitioner*        partitioner_         {};
  integer                    particles_per_round_ {};
  load_balancer              load_balancer_       {};
  variant_vector3_integrator integrator_          {};
  scalar                     step_size_           {};
  bool                       gather_particles_    {};
  bool                       record_              {};
};
}

#endif