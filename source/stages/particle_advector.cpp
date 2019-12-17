#include <dpa/stages/particle_advector.hpp>

#include <optional>

#include <boost/mpi.hpp>
#include <tbb/tbb.h>

#undef min
#undef max

namespace dpa
{
particle_advector::particle_advector(domain_partitioner* partitioner, const integer particles_per_round, const std::string& load_balancer, const std::string& integrator, const scalar step_size, const bool gather_particles, const bool record)
: partitioner_        (partitioner)
, particles_per_round_(particles_per_round)
, step_size_          (step_size)
, gather_particles_   (gather_particles)
, record_             (record)
{
  if (load_balancer == "diffuse") load_balancer_ = load_balancer::diffuse;
  else                            load_balancer_ = load_balancer::none   ;

  if      (integrator == "euler"                       ) integrator_ = euler_integrator                       <vector3>();
  else if (integrator == "modified_midpoint"           ) integrator_ = modified_midpoint_integrator           <vector3>();
  else if (integrator == "runge_kutta_4"               ) integrator_ = runge_kutta_4_integrator               <vector3>();
  else if (integrator == "runge_kutta_cash_karp_54"    ) integrator_ = runge_kutta_cash_karp_54_integrator    <vector3>();
  else if (integrator == "runge_kutta_dormand_prince_5") integrator_ = runge_kutta_dormand_prince_5_integrator<vector3>();
  else if (integrator == "runge_kutta_fehlberg_78"     ) integrator_ = runge_kutta_fehlberg_78_integrator     <vector3>();
  else if (integrator == "adams_bashforth_2"           ) integrator_ = adams_bashforth_2_integrator           <vector3>();
  else if (integrator == "adams_bashforth_moulton_2"   ) integrator_ = adams_bashforth_moulton_2_integrator   <vector3>();
}

particle_advector::output     particle_advector::advect                  (const std::unordered_map<relative_direction, regular_vector_field_3d>& vector_fields,       std::vector<particle<vector3, integer>>& particles)
{
  output output;
  while (!check_completion(particles))
  {
                      load_balance_distribute (               particles                                                      );
    auto round_info = compute_round_info      (               particles,                   output.integral_curves            );
                      allocate_integral_curves(               particles,                   output.integral_curves, round_info);
                      advect                  (vector_fields, particles, output.particles, output.integral_curves, round_info);
                      load_balance_collect    (                                                                    round_info);
                      out_of_bounds_distribute(               particles,                                           round_info);
  }
  gather_particles     (output.particles      );
  prune_integral_curves(output.integral_curves);
  return output;
}

bool                          particle_advector::check_completion        (                                                                                      const std::vector<particle<vector3, integer>>& particles                                                                         ) 
{ 
  std::vector<std::size_t> particle_sizes;
  boost::mpi::gather   (*partitioner_->cartesian_communicator(), particles.size(), particle_sizes, 0);
  auto   complete = std::all_of(particle_sizes.begin(), particle_sizes.end(), std::bind(std::equal_to<std::size_t>(), std::placeholders::_1, 0));
  boost::mpi::broadcast(*partitioner_->cartesian_communicator(), complete, 0);
  return complete;
}
void                          particle_advector::load_balance_distribute (                                                                                            std::vector<particle<vector3, integer>>& particles                                                                         ) 
{
  if (load_balancer_ == load_balancer::none) return;

#ifdef DPA_USE_NEIGHBORHOOD_COLLECTIVES
  // TODO: Neighborhood collectives.
#else

#endif
}                                                                                                                                                                                                                         
particle_advector::round_info particle_advector::compute_round_info      (                                                                                      const std::vector<particle<vector3, integer>>& particles, const integral_curves_3d& integral_curves                              ) 
{
  round_info round_info;
  round_info.particle_count               = std::min(std::size_t(particles_per_round_), particles.size());
  round_info.maximum_remaining_iterations = std::max_element(particles.begin(), particles.begin() + round_info.particle_count, [ ] (const particle<vector3, integer>& lhs, const particle<vector3, integer>& rhs) { return lhs.remaining_iterations < rhs.remaining_iterations; })->remaining_iterations;
  round_info.vertex_offset                = record_ ? integral_curves.size()                                              : 0;
  round_info.vertex_count                 = record_ ? round_info.particle_count * round_info.maximum_remaining_iterations : 0;
  
  for (auto& partition : partitioner_->partitions())
  {
    round_info.out_of_bounds_particles         .emplace(partition.first, std::vector<particle<vector3, integer>>());
    round_info.neighbor_out_of_bounds_particles.emplace(partition.first, std::vector<particle<vector3, integer>>());
  }

  return round_info;
}
void                          particle_advector::allocate_integral_curves(                                                                                      const std::vector<particle<vector3, integer>>& particles,       integral_curves_3d& integral_curves, const round_info& round_info) 
{
  if (!record_) return;

  integral_curves.resize(integral_curves.size() + round_info.vertex_count);
  tbb::parallel_for(std::size_t(0), round_info.particle_count, std::size_t(1), [&] (const std::size_t index)
  {
    integral_curves[integral_curves.size() + round_info.maximum_remaining_iterations * index] = particles[index].position;
  });
}
void                          particle_advector::advect                  (const std::unordered_map<relative_direction, regular_vector_field_3d>& vector_fields,       std::vector<particle<vector3, integer>>& particles,       integral_curves_3d& integral_curves,       round_info& round_info) 
{
  tbb::parallel_for(std::size_t(0), round_info.particle_count, std::size_t(1), [&] (const std::size_t particle_index)
  {
    auto& particle     = particles[particle_index];
    auto& vector_field = vector_fields.at(particle.relative_direction);
    auto  minimum      = vector_field.offset;
    auto  maximum      = vector_field.offset + vector_field.size;
    auto  integrator   = integrator_;

    for (auto iteration_index = 1; iteration_index < particle.remaining_iterations; ++iteration_index)
    {
      auto terminated = false;

      if (!vector_field.contains(particle.position))
      {
        terminated = true;

        std::optional<relative_direction> direction;
        if      (particle.position[0] < minimum[0]) direction = relative_direction::negative_x;
        else if (particle.position[0] > maximum[0]) direction = relative_direction::positive_x;
        else if (particle.position[1] < minimum[1]) direction = relative_direction::negative_y;
        else if (particle.position[1] > maximum[1]) direction = relative_direction::positive_y;
        else if (particle.position[2] < minimum[2]) direction = relative_direction::negative_z;
        else if (particle.position[2] > maximum[2]) direction = relative_direction::positive_z;

        // Terminate local.

        if (direction)
        {
          round_info::particle_map::accessor accessor;
          if (round_info.out_of_bounds_particles.find(accessor, direction.value()))
            accessor->second.push_back(particle);
        }
      }
      const auto vector = vector_field.interpolate(particle.position);
      if (vector.isZero())
      {
        terminated = true;
        // TODO: Terminate particle. I.e. MOVE from particles to finished
      }

      if (!terminated)
      {
        const auto system = [&] (const vector3& x, vector3& dxdt, const float t) { dxdt = vector; };
        if      (std::holds_alternative<euler_integrator<vector3>>                       (integrator))
          std::get<euler_integrator<vector3>>                       (integrator).do_step(system, particle.position, iteration_index * step_size_, step_size_);
        else if (std::holds_alternative<modified_midpoint_integrator<vector3>>           (integrator))
          std::get<modified_midpoint_integrator<vector3>>           (integrator).do_step(system, particle.position, iteration_index * step_size_, step_size_);
        else if (std::holds_alternative<runge_kutta_4_integrator<vector3>>               (integrator))
          std::get<runge_kutta_4_integrator<vector3>>               (integrator).do_step(system, particle.position, iteration_index * step_size_, step_size_);
        else if (std::holds_alternative<runge_kutta_cash_karp_54_integrator<vector3>>    (integrator))
          std::get<runge_kutta_cash_karp_54_integrator<vector3>>    (integrator).do_step(system, particle.position, iteration_index * step_size_, step_size_);
        else if (std::holds_alternative<runge_kutta_dormand_prince_5_integrator<vector3>>(integrator))
          std::get<runge_kutta_dormand_prince_5_integrator<vector3>>(integrator).do_step(system, particle.position, iteration_index * step_size_, step_size_);
        else if (std::holds_alternative<runge_kutta_fehlberg_78_integrator<vector3>>     (integrator))
          std::get<runge_kutta_fehlberg_78_integrator<vector3>>     (integrator).do_step(system, particle.position, iteration_index * step_size_, step_size_);
        else if (std::holds_alternative<adams_bashforth_2_integrator<vector3>>           (integrator))
          std::get<adams_bashforth_2_integrator<vector3>>           (integrator).do_step(system, particle.position, iteration_index * step_size_, step_size_);
        else if (std::holds_alternative<adams_bashforth_moulton_2_integrator<vector3>>   (integrator))
          std::get<adams_bashforth_moulton_2_integrator<vector3>>   (integrator).do_step(system, particle.position, iteration_index * step_size_, step_size_);
      }

      if (record_)
        integral_curves[round_info.vertex_offset + particle_index * round_info.maximum_remaining_iterations + iteration_index] = terminated ? terminal_value<vector3>() : particle.position;

      if (terminated)
        break;
    }
  });                                                                                                                                                                                                             
}                                                                                                                                                                                                                         
void                          particle_advector::load_balance_collect    (                                                                                                                                                                                                 round_info& round_info) 
{
  if (load_balancer_ == load_balancer::none) return;

#ifdef DPA_USE_NEIGHBORHOOD_COLLECTIVES
  // TODO: Neighborhood collectives.
#else

#endif
}                                                                                                                                                                                                                         
void                          particle_advector::out_of_bounds_distribute(                                                                                            std::vector<particle<vector3, integer>>& particles,                                            const round_info& round_info) 
{
#ifdef DPA_USE_NEIGHBORHOOD_COLLECTIVES
  // TODO: Neighborhood collectives.
#else
  std::vector<boost::mpi::request> requests;

  auto  communicator = partitioner_->cartesian_communicator();
  auto& partitions   = partitioner_->partitions            ();

  for (auto& neighbor : round_info.out_of_bounds_particles)
  {
    if (partitions.find(neighbor.first) != partitions.end())
    {
      requests.push_back(communicator->isend(partitions.at(neighbor.first).rank, 0, neighbor.second));
    }
  } 
  for (auto& neighbor : round_info.out_of_bounds_particles)
  {
    if (partitions.find(neighbor.first) != partitions.end())
    {
      std::vector<particle<vector3, integer>> temporary_particles;
      communicator->recv(partitions.at(neighbor.first).rank, 0, temporary_particles);
      particles.insert(particles.end(), temporary_particles.begin(), temporary_particles.end());
    }
  }

  for (auto& request : requests)
    request.wait();
#endif
}
void                          particle_advector::gather_particles        (                                                                                            std::vector<particle<vector3, integer>>& particles                                                                         )
{

}
void                          particle_advector::prune_integral_curves   (                                                                                                                                                      integral_curves_3d& integral_curves                              ) 
{
  if (!record_) return;

  integral_curves.erase(std::remove(integral_curves.begin(), integral_curves.end(), invalid_value<vector3>()), integral_curves.end());
}
}
