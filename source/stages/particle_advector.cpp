#include <dpa/stages/particle_advector.hpp>

#include <tbb/tbb.h>

#undef min
#undef max

namespace dpa
{
particle_advector::particle_advector(domain_partitioner* partitioner, const std::string& load_balancer, const std::string& integrator, const scalar step_size, const bool record)
: partitioner_(partitioner)
, step_size_  (step_size)
, record_     (record)
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

integral_curves_3d particle_advector::advect(const std::unordered_map<relative_direction, regular_vector_field_3d>& vector_fields, std::vector<particle<vector3, integer>>& seeds)
{
  integral_curves_3d integral_curves;

  auto total_iterations = seeds.at(0).remaining_iterations;
  integral_curves.resize(seeds.size() * total_iterations, vector3(-2, -2, -2));

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
      integral_curves[particle_index * total_iterations + iteration_index] = vector3(-1, -1, -1);

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
    }
  });
  integral_curves.erase(std::remove(integral_curves.begin(), integral_curves.end(), vector3(-2, -2, -2)), integral_curves.end());

  return integral_curves;
}
}
