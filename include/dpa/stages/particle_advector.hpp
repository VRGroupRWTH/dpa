#ifndef DPA_STAGES_PARTICLE_ADVECTOR_HPP
#define DPA_STAGES_PARTICLE_ADVECTOR_HPP

#include <cstddef>
#include <string>
#include <unordered_map>
#include <vector>

#include <tbb/concurrent_vector.h>

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
  using vector_field_map           = std::unordered_map    <relative_direction, regular_vector_field_3d>;
  using particle_vector            = std::vector           <particle_3d>;
  using particle_map               = std::unordered_map    <relative_direction, particle_vector>;
  using concurrent_particle_vector = tbb::concurrent_vector<particle_3d>;
  using concurrent_particle_map    = std::unordered_map    <relative_direction, concurrent_particle_vector>;

  struct state
  {
    state           (const vector_field_map& vector_fields, particle_vector& active_particles, const std::unordered_map<relative_direction, domain_partitioner::partition>& partitions)
    : vector_fields(vector_fields), active_particles(active_particles)
    {
      for (auto& partition : partitions)
        if (partition.first != center)
          load_balanced_active_particles.emplace(partition.first, particle_vector());
    }
    state           (const state&  that) = default;
    state           (      state&& temp) = default;
   ~state           ()                   = default;
    state& operator=(const state&  that) = default;
    state& operator=(      state&& temp) = default;

    std::size_t total_active_particle_count() const
    {
      auto count = active_particles.size();
      for (auto& entry : load_balanced_active_particles)
        count += entry.second.size();
      return count;
    }
    
    const vector_field_map&    vector_fields;
    particle_vector&           active_particles;
    particle_map               load_balanced_active_particles {};
  };
  struct round_state
  {
    explicit round_state  (const std::unordered_map<relative_direction, domain_partitioner::partition>& partitions)
    {
      for (auto& partition : partitions)
      {
        if (partition.first != center)
        {
          out_of_bounds_particles              .emplace(partition.first, concurrent_particle_vector());
          load_balanced_out_of_bounds_particles.emplace(partition.first, concurrent_particle_vector()); 
        }
      }
    }
    round_state           (const round_state&  that) = default;
    round_state           (      round_state&& temp) = default;
   ~round_state           ()                         = default;
    round_state& operator=(const round_state&  that) = default;
    round_state& operator=(      round_state&& temp) = default;

    std::size_t                particle_count                        = 0;
    std::size_t                curve_stride                          = 0;
    std::size_t                vertex_count                          = 0;
    concurrent_particle_map    out_of_bounds_particles               {};
    concurrent_particle_map    load_balanced_out_of_bounds_particles {};
  };
  struct output
  {
    concurrent_particle_vector inactive_particles                    {};
    integral_curves_3d         integral_curves                       {};
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
    const size          particles_per_round, 
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

  output      advect                  (const vector_field_map& vector_fields, particle_vector& particles);
  
protected:
  friend pipeline; // For benchmarking of individual steps.

  bool        check_completion        (const state& state) const;
  void        load_balance_distribute (      state& state);
  round_state compute_round_state     (const state& state);
  void        allocate_integral_curves(                    const round_state& round_state, output& output);
  void        advect                  (      state& state,       round_state& round_state, output& output);
  void        load_balance_collect    (      state& state,       round_state& round_state, output& output);
  void        out_of_bounds_distribute(      state& state, const round_state& round_state);
  void        gather_particles        (                                                    output& output);
  void        prune_integral_curves   (                                                    output& output);

  domain_partitioner*        partitioner_         {};
  size                       particles_per_round_ {};
  load_balancer              load_balancer_       {};
  variant_vector3_integrator integrator_          {};
  scalar                     step_size_           {};
  bool                       gather_particles_    {};
  bool                       record_              {};
};
}

#endif