#include <dpa/stages/particle_advector.hpp>

#include <cmath>
#include <optional>

#include <boost/serialization/vector.hpp>
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
  if      (load_balancer == "diffuse_constant")                       load_balancer_ = load_balancer::diffuse_constant;
  else if (load_balancer == "diffuse_lesser_average")                 load_balancer_ = load_balancer::diffuse_lesser_average;
  else if (load_balancer == "diffuse_greater_limited_lesser_average") load_balancer_ = load_balancer::diffuse_greater_limited_lesser_average;
  else                                                                load_balancer_ = load_balancer::none;

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
    auto round_info = compute_round_info      (               particles                                                      );
                      allocate_integral_curves(                                            output.integral_curves, round_info);
                      advect                  (vector_fields, particles, output.particles, output.integral_curves, round_info);
                      load_balance_collect    (vector_fields,            output.particles,                         round_info);
                      out_of_bounds_distribute(               particles,                                           round_info);
  }
  gather_particles     (output.particles      );
  prune_integral_curves(output.integral_curves);
  return output;
}

bool                          particle_advector::check_completion        (                                                                                      const std::vector<particle<vector3, integer>>& particles) 
{ 
  std::vector<std::size_t> particle_sizes;
  boost::mpi::gather   (*partitioner_->cartesian_communicator(), particles.size(), particle_sizes, 0);
  auto   complete = std::all_of(particle_sizes.begin(), particle_sizes.end(), std::bind(std::equal_to<std::size_t>(), std::placeholders::_1, 0));
  boost::mpi::broadcast(*partitioner_->cartesian_communicator(), complete, 0);
  return complete;
}
void                          particle_advector::load_balance_distribute (                                                                                            std::vector<particle<vector3, integer>>& particles)
{
  if (load_balancer_ == load_balancer::none) return;

  if (load_balancer_ == load_balancer::diffuse_constant || load_balancer_ == load_balancer::diffuse_lesser_average || load_balancer_ == load_balancer::diffuse_greater_limited_lesser_average)
  {
#ifdef DPA_USE_NEIGHBORHOOD_COLLECTIVES
    // TODO: Neighborhood collectives.
#else
    auto  communicator = partitioner_->cartesian_communicator();
    auto& partitions   = partitioner_->partitions            ();

    // Send/receive particle counts to/from neighbors.
    const load_balancing_info                                   local_load_balancing_info { std::size_t(communicator->rank()), std::min(std::size_t(particles_per_round_), particles.size()) };
    std::unordered_map<relative_direction, load_balancing_info> neighbor_load_balancing_info;
    {
      std::vector<boost::mpi::request> requests;
      for (auto& partition : partitions)
        requests.push_back(communicator->isend(partition.second.rank, 0, local_load_balancing_info));
      for (auto& partition : partitions)
        communicator->recv(partition.second.rank, 0, neighbor_load_balancing_info[partition.first]);
      for (auto& request : requests)
        request.wait();
    }

    // Load balancers fill the outgoing_counts to each of their neighbors.
    std::unordered_map<relative_direction, std::size_t> outgoing_counts;
    for (auto& neighbor : neighbor_load_balancing_info)
      outgoing_counts[neighbor.first] = 0;
    if      (load_balancer_ == load_balancer::diffuse_constant)
    {
      // Alpha is  1 - 2 / (dimensions + 1) -> 0.5 for 3D.
      for (auto& neighbor : neighbor_load_balancing_info)
        if (neighbor.second.particle_count < local_load_balancing_info.particle_count)
          outgoing_counts[neighbor.first] = std::min(particles.size(), std::size_t((local_load_balancing_info.particle_count - neighbor.second.particle_count) * double(0.5)));
    }
    else if (load_balancer_ == load_balancer::diffuse_lesser_average)
    {
      std::unordered_map<relative_direction, bool> contributors;
      std::size_t sum(0), count(0), mean(local_load_balancing_info.particle_count);
      auto is_complete = [&] ()
      {
        // False while there are contributors above the mean.
        for (auto& contributor : contributors)
          if (contributor.second)
            if (neighbor_load_balancing_info.at(contributor.first).particle_count > mean)
              return false;
        return true;
      };

      do
      {
        sum   = local_load_balancing_info.particle_count;
        count = 1;
        for (auto& neighbor : neighbor_load_balancing_info)
        {
          if (neighbor.second.particle_count < mean)
          {
            contributors[neighbor.first]  = true;
            sum                          += neighbor.second.particle_count;
            count                        ++;
          }
          else
            contributors[neighbor.first] = false;
        }
        mean = sum / count;
      } while (!is_complete());

      for (auto& neighbor : neighbor_load_balancing_info)
        if (contributors[neighbor.first])
          outgoing_counts[neighbor.first] = std::min(particles.size(), mean - neighbor_load_balancing_info[neighbor.first].particle_count);
    }
    else if (load_balancer_ == load_balancer::diffuse_greater_limited_lesser_average)
    {
      // TODO
    }

    // Send/receive outgoing_counts particles to/from neighbors.
    {
      std::vector<boost::mpi::request> requests;
      for (auto& neighbor : neighbor_load_balancing_info)
      {
        std::vector<particle<vector3, integer>> outgoing_particles(particles.end() - outgoing_counts[neighbor.first], particles.end());
        particles.erase(particles.end() - outgoing_counts[neighbor.first], particles.end());

        tbb::parallel_for(std::size_t(0), outgoing_particles.size(), std::size_t(1), [&] (const std::size_t index)
        {
          outgoing_particles[index].relative_direction = relative_direction(-neighbor.first);
        });

        requests.push_back(communicator->isend(neighbor.second.rank, 0, outgoing_particles));
        std::cout << "Send " << outgoing_particles.size() << " particles to neighbor " << neighbor.first << "\n";
      } 
      for (auto& neighbor : neighbor_load_balancing_info)
      {
        std::vector<particle<vector3, integer>> incoming_particles;
        communicator->recv(neighbor.second.rank, 0, incoming_particles);
        particles.insert(particles.end(), incoming_particles.begin(), incoming_particles.end());
        std::cout << "Recv " << incoming_particles.size() << " particles from neighbor " << neighbor.first << "\n";
      }   
      for (auto& request : requests)
        request.wait();
    }
#endif
  }
}
particle_advector::round_info particle_advector::compute_round_info      (                                                                                      const std::vector<particle<vector3, integer>>& particles) 
{
  round_info round_info;
  round_info.particle_count = std::min(std::size_t(particles_per_round_), particles.size());

  if (record_)
  {
    // Two more vertices per curve; one for initial position, one for termination vertex.
    round_info.curve_stride = std::max_element(particles.end() - round_info.particle_count, particles.end(), [ ] (const particle<vector3, integer>& lhs, const particle<vector3, integer>& rhs) { return lhs.remaining_iterations < rhs.remaining_iterations; })->remaining_iterations + 2;
    round_info.vertex_count = round_info.particle_count * round_info.curve_stride;
  }

  for (auto& partition : partitioner_->partitions())
  {
    round_info.out_of_bounds_particles              .emplace(partition.first, std::vector<particle<vector3, integer>>());
    round_info.load_balanced_out_of_bounds_particles.emplace(partition.first, std::vector<particle<vector3, integer>>());
  }

  return round_info;
}
void                          particle_advector::allocate_integral_curves(                                                                                                                                                                                                             integral_curves_3d& integral_curves, const round_info& round_info) 
{
  if (!record_) return;

  integral_curves.emplace_back().resize(round_info.vertex_count, invalid_value<vector3>());
}
void                          particle_advector::advect                  (const std::unordered_map<relative_direction, regular_vector_field_3d>& vector_fields,       std::vector<particle<vector3, integer>>& particles, std::vector<particle<vector3, integer>>& inactive_particles, integral_curves_3d& integral_curves,       round_info& round_info)
{
  tbb::mutex mutex;
  tbb::parallel_for(std::size_t(0), round_info.particle_count, std::size_t(1), [&] (const std::size_t particle_index)
  {
    auto& particle        = particles[particles.size() - round_info.particle_count + particle_index];
    auto& vector_field    = vector_fields.at(particle.relative_direction);
    auto  lower_bounds    = vector_field.offset;
    auto  upper_bounds    = vector_field.offset + vector_field.size;
    auto  integrator      = integrator_;
    auto  iteration_index = 0;

    if (record_)
      integral_curves.back()[particle_index * round_info.curve_stride] = particle.position;

    for ( ; particle.remaining_iterations > 0; ++iteration_index, --particle.remaining_iterations)
    {
      if (!vector_field.contains(particle.position))
      {
        if (particle.relative_direction == relative_direction::center) // if non-load balanced particle:
        {
          std::optional<relative_direction> direction;
          if      (particle.position[0] < lower_bounds[0]) direction = relative_direction::negative_x;
          else if (particle.position[0] > upper_bounds[0]) direction = relative_direction::positive_x;
          else if (particle.position[1] < lower_bounds[1]) direction = relative_direction::negative_y;
          else if (particle.position[1] > upper_bounds[1]) direction = relative_direction::positive_y;
          else if (particle.position[2] < lower_bounds[2]) direction = relative_direction::negative_z;
          else if (particle.position[2] > upper_bounds[2]) direction = relative_direction::positive_z;

          round_info::particle_map::accessor accessor;
          if (direction && round_info.out_of_bounds_particles.find(accessor, direction.value()))
            accessor->second.push_back(particle);
          else
          {
            tbb::mutex::scoped_lock lock(mutex);
            inactive_particles.push_back(particle);
          }
        }
        else // if load balanced particle:
        {
          round_info::particle_map::accessor accessor;
          if (round_info.load_balanced_out_of_bounds_particles.find(accessor, particle.relative_direction))
            accessor->second.push_back(particle);
        }
        break;
      }

      const auto vector = vector_field.interpolate(particle.position);
      if (vector.isZero())
      {
        tbb::mutex::scoped_lock lock(mutex);
        inactive_particles.push_back(particle);

        break;
      }

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
      
      if (record_)
        integral_curves.back()[particle_index * round_info.curve_stride + iteration_index + 1] = particle.position;
    }

    if (record_)
      integral_curves.back()[particle_index * round_info.curve_stride + iteration_index + 1] = terminal_value<vector3>();

    if (particle.remaining_iterations == 0)
    {
      tbb::mutex::scoped_lock lock(mutex);
      inactive_particles.push_back(particle);
    }
  });
  particles.resize(particles.size() - round_info.particle_count);
}
void                          particle_advector::load_balance_collect    (const std::unordered_map<relative_direction, regular_vector_field_3d>& vector_fields,                                                           std::vector<particle<vector3, integer>>& inactive_particles,                                            round_info& round_info) 
{
  if (load_balancer_ == load_balancer::none) return;

  if (load_balancer_ == load_balancer::diffuse_constant || load_balancer_ == load_balancer::diffuse_lesser_average || load_balancer_ == load_balancer::diffuse_greater_limited_lesser_average)
  {
#ifdef DPA_USE_NEIGHBORHOOD_COLLECTIVES
    // TODO: Neighborhood collectives.
#else
    std::vector<boost::mpi::request> requests;

    auto  communicator = partitioner_->cartesian_communicator();
    auto& partitions   = partitioner_->partitions            ();

    for (auto& neighbor : round_info.load_balanced_out_of_bounds_particles)
      requests.push_back(communicator->isend(partitions.at(neighbor.first).rank, 0, neighbor.second));
    for (auto& neighbor : round_info.load_balanced_out_of_bounds_particles)
    {
      std::vector<particle<vector3, integer>> temporary_particles;
      communicator->recv(partitions.at(neighbor.first).rank, 0, temporary_particles);
      
      auto& vector_field = vector_fields.at(relative_direction::center);
      auto  lower_bounds = vector_field.offset;
      auto  upper_bounds = vector_field.offset + vector_field.size;

      tbb::mutex mutex;
      tbb::parallel_for(std::size_t(0), temporary_particles.size(), std::size_t(1), [&] (const std::size_t particle_index)
      {
        auto& particle = temporary_particles[particle_index];

        particle.relative_direction = relative_direction::center;

        std::optional<relative_direction> direction;
        if      (particle.position[0] < lower_bounds[0]) direction = relative_direction::negative_x;
        else if (particle.position[0] > upper_bounds[0]) direction = relative_direction::positive_x;
        else if (particle.position[1] < lower_bounds[1]) direction = relative_direction::negative_y;
        else if (particle.position[1] > upper_bounds[1]) direction = relative_direction::positive_y;
        else if (particle.position[2] < lower_bounds[2]) direction = relative_direction::negative_z;
        else if (particle.position[2] > upper_bounds[2]) direction = relative_direction::positive_z;

        round_info::particle_map::accessor accessor;
        if (direction && round_info.out_of_bounds_particles.find(accessor, direction.value()))
          accessor->second.push_back(particle);
        else
        {
          tbb::mutex::scoped_lock lock(mutex);
          inactive_particles.push_back(particle);
        }
      });
    }

    for (auto& request : requests)
      request.wait();
#endif
  }
}                                                                                                                                                                                                                         
void                          particle_advector::out_of_bounds_distribute(                                                                                            std::vector<particle<vector3, integer>>& particles,                                                                                                         const round_info& round_info) 
{
#ifdef DPA_USE_NEIGHBORHOOD_COLLECTIVES
  // TODO: Neighborhood collectives.
#else
  std::vector<boost::mpi::request> requests;

  auto  communicator = partitioner_->cartesian_communicator();
  auto& partitions   = partitioner_->partitions            ();

  for (auto& neighbor : round_info.out_of_bounds_particles)
    requests.push_back(communicator->isend(partitions.at(neighbor.first).rank, 0, neighbor.second));
  for (auto& neighbor : round_info.out_of_bounds_particles)
  {
    std::vector<particle<vector3, integer>> temporary_particles;
    communicator->recv(partitions.at(neighbor.first).rank, 0, temporary_particles);
    particles.insert(particles.end(), temporary_particles.begin(), temporary_particles.end());
  }

  for (auto& request : requests)
    request.wait();
#endif
}
void                          particle_advector::gather_particles        (                                                                                            std::vector<particle<vector3, integer>>& particles)
{
  if (!gather_particles_) return;

#ifdef DPA_FTLE_SUPPORT
  std::vector<std::vector<particle<vector3, integer>>> sent    (partitioner_->cartesian_communicator()->size());
  std::vector<std::vector<particle<vector3, integer>>> received(partitioner_->cartesian_communicator()->size());

  tbb::mutex mutex;
  tbb::parallel_for(std::size_t(0), particles.size(), std::size_t(1), [&](const std::size_t index)
  {
    tbb::mutex::scoped_lock lock(mutex);
    sent[particles[index].original_rank].push_back(particles[index]);
  });

  boost::mpi::all_to_all(*partitioner_->cartesian_communicator(), sent, received);

  particles.clear();
  for (auto& particles_vector : received)
    particles.insert(particles.end(), particles_vector.begin(), particles_vector.end());
#else
  std::cout << "Particles are not gathered since original ranks are unavailable. Declare DPA_FTLE_SUPPORT and rebuild." << std::endl;
#endif
}
void                          particle_advector::prune_integral_curves   (                                                                                                                                                                                                                   integral_curves_3d& integral_curves) 
{
  if (!record_) return;
  
  tbb::parallel_for(std::size_t(0), integral_curves.size(), std::size_t(1), [&](const std::size_t index)
  {
    auto& curves = integral_curves[index];
    curves.erase(std::remove(curves.begin(), curves.end(), invalid_value<vector3>()), curves.end());
  });
}
}
