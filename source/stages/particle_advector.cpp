#include <dpa/stages/particle_advector.hpp>

#include <tbb/tbb.h>

#undef min
#undef max

namespace dpa
{
particle_advector::particle_advector(domain_partitioner* partitioner, const integer particles_per_round, const std::string& load_balancer, const std::string& integrator, const scalar step_size, const bool record)
: partitioner_        (partitioner)
, particles_per_round_(particles_per_round)
, step_size_          (step_size)
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

integral_curves_3d            particle_advector::advect                  (const std::unordered_map<relative_direction, regular_vector_field_3d>& vector_fields,       std::vector<particle<vector3, integer>>& particles)
{
  integral_curves_3d integral_curves;
  while (!check_completion(particles))
  {
                      load_balance_distribute (               particles                             );
    auto round_info = compute_round_info      (               particles, integral_curves            );
                      allocate_integral_curves(               particles, integral_curves, round_info);
                      advect                  (vector_fields, particles, integral_curves, round_info);
                      load_balance_collect    (                                           round_info);
                      out_of_bounds_distribute(               particles,                  round_info);
  }
  prune (integral_curves);
  return integral_curves;
}

bool                          particle_advector::check_completion        (                                                                                      const std::vector<particle<vector3, integer>>& particles                                                                         ) 
{ 

}
void                          particle_advector::load_balance_distribute (                                                                                            std::vector<particle<vector3, integer>>& particles                                                                         ) 
{ 

}
particle_advector::round_info particle_advector::compute_round_info      (                                                                                      const std::vector<particle<vector3, integer>>& particles, const integral_curves_3d& integral_curves                              ) 
{ 

}
void                          particle_advector::allocate_integral_curves(                                                                                      const std::vector<particle<vector3, integer>>& particles,       integral_curves_3d& integral_curves, const round_info& round_info) 
{ 

}
void                          particle_advector::advect                  (const std::unordered_map<relative_direction, regular_vector_field_3d>& vector_fields, const std::vector<particle<vector3, integer>>& particles,       integral_curves_3d& integral_curves,       round_info& round_info) 
{ 
  integral_curves_3d integral_curves;

  auto total_iterations = seeds.at(0).remaining_iterations;
  integral_curves.resize(seeds.size() * total_iterations, invalid_value<vector3>());

  tbb::parallel_for(std::size_t(0), seeds.size(), std::size_t(1), [&] (const std::size_t particle_index)
  {
    auto& particle   = seeds[particle_index];

    auto  center     = vector_fields.at(relative_direction::center);
    auto  minimum    = center.offset;
    auto  maximum    = center.offset + center.size;
    auto  integrator = integrator_;

    integral_curves[particle_index * total_iterations] = particle.position;
    for (std::size_t iteration_index = 1; iteration_index < particle.remaining_iterations; ++iteration_index)
    {
      integral_curves[particle_index * total_iterations + iteration_index] = terminal_value<vector3>();

      if (!center.contains(particle.position))
        break;

      const auto vector = center.interpolate(particle.position);
      if (vector.isZero())
        break;
      
      const auto system = [&] (const vector3& x, vector3& dxdt, const float t) 
      { 
        dxdt = vector;
      };
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

      integral_curves[particle_index * total_iterations + iteration_index] = particle.position;

      if (iteration_index + 1 == particle.remaining_iterations)
      {
        integral_curves[particle_index * total_iterations + iteration_index] = terminal_value<vector3>();
        break;
      }
    }
  });
  integral_curves.erase(std::remove(integral_curves.begin(), integral_curves.end(), invalid_value<vector3>()), integral_curves.end());

  return integral_curves;
}
void                          particle_advector::load_balance_collect    (                                                                                                                                                                                                 round_info& round_info) 
{ 

}
void                          particle_advector::out_of_bounds_distribute(                                                                                            std::vector<particle<vector3, integer>>& particles,                                            const round_info& round_info) 
{ 

}
void                          particle_advector::prune                   (                                                                                                                                                      integral_curves_3d& integral_curves                              ) 
{ 

}
}
