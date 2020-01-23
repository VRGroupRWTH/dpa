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
  using vector_field_map  = std::unordered_map<relative_direction, regular_vector_field_3d>;
  using particle_vector   = std::vector<particle_3d>;
  using out_of_bounds_map = tbb::concurrent_hash_map<relative_direction, particle_vector>;

  struct state
  {
    const vector_fields&                              vector_fields;
    particles&                                        active_particles               {};
    std::unordered_map<relative_direction, particles> active_load_balanced_particles {};
  };
  struct round_state
  {
    std::size_t       particle_count                        = 0;
    std::size_t       curve_stride                          = 0;
    std::size_t       vertex_count                          = 0;
    out_of_bounds_map out_of_bounds_particles               {};
    out_of_bounds_map load_balanced_out_of_bounds_particles {};
  };
  struct output
  {
    particles          inactive_particles {}; // Implicitly the inactive particles vector.
    integral_curves_3d integral_curves    {};
  };

  struct load_balancing_info
  {
    // Function for boost::serialization which is used by boost::mpi.
    template<class archive_type>
    void serialize(archive_type& archive, const std::uint32_t version)
    {
      archive & rank;
      archive & particle_count;
    }

    std::size_t rank;
    std::size_t particle_count;
  };
  struct quota_info
  {
    // Function for boost::serialization which is used by boost::mpi.
    template<class archive_type>
    void serialize(archive_type& archive, const std::uint32_t version)
    {
      archive & quota;
    }

    std::size_t quota;
  };
  
  enum class load_balancer
  {
    none,
    diffuse_constant,
    diffuse_lesser_average,
    diffuse_greater_limited_lesser_average
  };

  explicit particle_advector  (
    domain_partitioner* partitioner        , 
    const integer       particles_per_round, 
    const std::string&  load_balancer      , 
    const std::string&  integrator         , 
    const scalar        step_size          , 
    const bool          gather_particles   , 
    const bool          record             );
  particle_advector           (const particle_advector&  that) = delete ;
  particle_advector           (      particle_advector&& temp) = default;
 ~particle_advector           ()                               = default;
  particle_advector& operator=(const particle_advector&  that) = delete ;
  particle_advector& operator=(      particle_advector&& temp) = default;

  output      advect                  (const vector_fields& vector_fields, particles& active_particles);
  
protected:
  friend pipeline; // For benchmarking of individual steps.

  bool        check_completion        (const state& state);
  void        load_balance_distribute (      state& state);
  round_state compute_round_state     (const state& state);
  void        allocate_integral_curves(                    const round_state& round_state, output& output);
  void        advect                  (      state& state,       round_state& round_state, output& output);
  void        load_balance_collect    (      state& state,       round_state& round_state, output& output);
  void        out_of_bounds_distribute(      state& state, const round_state& round_state);
  void        gather_particles        (                                                    output& output);
  void        prune_integral_curves   (                                                    output& output);

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