#include <dpa/stages/particle_advector.hpp>

#include <cmath>
#include <optional>

#include <boost/serialization/vector.hpp>
#include <boost/mpi.hpp>
#include <tbb/tbb.h>

#include <dpa/utility/serialization/concurrent_vector.hpp>

#undef min
#undef max

namespace dpa
{
particle_advector::particle_advector(domain_partitioner* partitioner, const size particles_per_round, const std::string& load_balancer, const std::string& integrator, const scalar step_size, const bool gather_particles, const bool record)
: partitioner_        (partitioner)
, particles_per_round_(particles_per_round)
, step_size_          (step_size)
, gather_particles_   (gather_particles)
, record_             (record)
{
  if      (load_balancer == "diffuse_constant"                      ) load_balancer_ = load_balancer::diffuse_constant;
  else if (load_balancer == "diffuse_lesser_average"                ) load_balancer_ = load_balancer::diffuse_lesser_average;
  else if (load_balancer == "diffuse_greater_limited_lesser_average") load_balancer_ = load_balancer::diffuse_greater_limited_lesser_average;
  else                                                                load_balancer_ = load_balancer::none;

  if      (integrator    == "euler"                                 ) integrator_    = euler_integrator                       <vector3>();
  else if (integrator    == "modified_midpoint"                     ) integrator_    = modified_midpoint_integrator           <vector3>();
  else if (integrator    == "runge_kutta_4"                         ) integrator_    = runge_kutta_4_integrator               <vector3>();
  else if (integrator    == "runge_kutta_cash_karp_54"              ) integrator_    = runge_kutta_cash_karp_54_integrator    <vector3>();
  else if (integrator    == "runge_kutta_dormand_prince_5"          ) integrator_    = runge_kutta_dormand_prince_5_integrator<vector3>();
  else if (integrator    == "runge_kutta_fehlberg_78"               ) integrator_    = runge_kutta_fehlberg_78_integrator     <vector3>();
  else if (integrator    == "adams_bashforth_2"                     ) integrator_    = adams_bashforth_2_integrator           <vector3>();
  else if (integrator    == "adams_bashforth_moulton_2"             ) integrator_    = adams_bashforth_moulton_2_integrator   <vector3>();
}

particle_advector::output      particle_advector::advect                  (const vector_field_map& vector_fields, particle_vector& particles)
{
  state  state(vector_fields, particles, partitioner_->partitions());
  output output;
  while (!check_completion(state))
  {
                       load_balance_distribute (state);
    auto round_state = compute_round_state     (state);
                       allocate_integral_curves(       round_state, output);
                       advect                  (state, round_state, output);
                       load_balance_collect    (state, round_state, output);
                       out_of_bounds_distribute(state, round_state);
  }
  gather_particles     (output);
  prune_integral_curves(output);
  return output;
}

bool                           particle_advector::check_completion        (const state& state) const
{ 
  std::vector<std::size_t> particle_counts;
  boost::mpi::gather   (*partitioner_->cartesian_communicator(), state.total_active_particle_count(), particle_counts, 0);
  auto   complete = std::all_of(particle_counts.begin(), particle_counts.end(), std::bind(std::equal_to<std::size_t>(), std::placeholders::_1, 0));
  boost::mpi::broadcast(*partitioner_->cartesian_communicator(), complete, 0);
  return complete;
}
void                           particle_advector::load_balance_distribute (      state& state)
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
    const load_balancing_info                                   local_load_balancing_info { communicator->rank(), state.total_active_particle_count() };
    std::unordered_map<relative_direction, load_balancing_info> neighbor_load_balancing_info;
    {
      std::vector<boost::mpi::request> requests;
      for (auto& partition : partitions)
        if (partition.first != center)
          requests.push_back(communicator->isend(partition.second.rank, 0, local_load_balancing_info));
      for (auto& partition : partitions)
        if (partition.first != center)
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
      auto remaining_particles = state.active_particles.size();
      for (auto& neighbor : neighbor_load_balancing_info)
      {
        if (neighbor.second.particle_count < local_load_balancing_info.particle_count)
        {
          outgoing_counts[neighbor.first] = std::min(remaining_particles, std::size_t((local_load_balancing_info.particle_count - neighbor.second.particle_count) * double(0.5)));
          remaining_particles -= outgoing_counts[neighbor.first];
        }
      }
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
          outgoing_counts[neighbor.first] = std::min(state.active_particles.size(), mean - neighbor_load_balancing_info[neighbor.first].particle_count);
    }
    else if (load_balancer_ == load_balancer::diffuse_greater_limited_lesser_average)
    {
      std::unordered_map<relative_direction, bool> greater_contributors;
      std::size_t greater_sum(0), greater_count(0), greater_mean(local_load_balancing_info.particle_count);
      auto is_greater_mean_complete = [&] ()
      {
        // False while there are contributors below the mean.
        for (auto& contributor : greater_contributors)
          if (contributor.second)
            if (neighbor_load_balancing_info.at(contributor.first).particle_count < greater_mean)
              return false;
        return true;
      };
      
      do
      {
        greater_sum   = local_load_balancing_info.particle_count;
        greater_count = 1;
        for (auto& neighbor : neighbor_load_balancing_info)
        {
          if (neighbor.second.particle_count > greater_mean)
          {
            greater_contributors[neighbor.first]  = true;
            greater_sum                          += neighbor.second.particle_count;
            greater_count                        ++;
          }
          else
            greater_contributors[neighbor.first] = false;
        }
        greater_mean = greater_sum / greater_count;
      } while (!is_greater_mean_complete());

      auto total_quota = greater_mean - local_load_balancing_info.particle_count;
      std::unordered_map<relative_direction, quota_info> outgoing_quotas, incoming_quotas;
      for (auto& neighbor : neighbor_load_balancing_info)
        outgoing_quotas[neighbor.first] = quota_info { greater_contributors[neighbor.first] ? total_quota * neighbor.second.particle_count / (greater_sum - local_load_balancing_info.particle_count) : 0ull};

      // Send/receive quotas to/from neighbors.
      {
        std::vector<boost::mpi::request> requests;
        for (auto& neighbor : neighbor_load_balancing_info)
          requests.push_back(communicator->isend(neighbor.second.rank, 0, outgoing_quotas[neighbor.first]));
        for (auto& neighbor : neighbor_load_balancing_info)
          communicator->recv(neighbor.second.rank, 0, incoming_quotas[neighbor.first]);
        for (auto& request : requests)
          request.wait();
      }

      std::unordered_map<relative_direction, bool> lesser_contributors;
      std::size_t lesser_sum(0), lesser_count(0), lesser_mean(local_load_balancing_info.particle_count);
      auto is_lesser_mean_complete = [&] ()
      {
        // False while there are contributors above the mean.
        for (auto& contributor : lesser_contributors)
          if (contributor.second)
            if (neighbor_load_balancing_info.at(contributor.first).particle_count > lesser_mean)
              return false;
        return true;
      };

      do
      {
        lesser_sum   = local_load_balancing_info.particle_count;
        lesser_count = 1;
        for (auto& neighbor : neighbor_load_balancing_info)
        {
          if (neighbor.second.particle_count < lesser_mean)
          {
            lesser_contributors[neighbor.first]  = true;
            lesser_sum                          += neighbor.second.particle_count;
            lesser_count                        ++;
          }
          else
            lesser_contributors[neighbor.first] = false;
        }
        lesser_mean = lesser_sum / lesser_count;
      } while (!is_lesser_mean_complete());

      for (auto& neighbor : neighbor_load_balancing_info)
        if (lesser_contributors[neighbor.first])
          outgoing_counts[neighbor.first] = std::min(state.active_particles.size(), std::min(incoming_quotas[neighbor.first].quota, lesser_mean - neighbor_load_balancing_info[neighbor.first].particle_count));
    }

    // Send/receive outgoing_counts particles to/from neighbors.
    {
      std::vector<boost::mpi::request> requests;
      for (auto& neighbor : neighbor_load_balancing_info)
      {
        particle_vector outgoing_particles(state.active_particles.end() - outgoing_counts[neighbor.first], state.active_particles.end());
        state.active_particles.resize(state.active_particles.size() - outgoing_counts[neighbor.first]);

        tbb::parallel_for(std::size_t(0), outgoing_particles.size(), std::size_t(1), [&] (const std::size_t index)
        {
          outgoing_particles[index].relative_direction = relative_direction(-neighbor.first); // This process is e.g. the north neighbor of its south neighbor.
        });

        requests.push_back(communicator->isend(neighbor.second.rank, 0, outgoing_particles));

        //std::cout << "Send " << outgoing_particles.size() << " particles to neighbor " << neighbor.first << "\n";
      } 
      for (auto& neighbor : neighbor_load_balancing_info)
      {
        particle_vector incoming_particles;
        communicator->recv(neighbor.second.rank, 0, incoming_particles);

        auto& target_particles = state.load_balanced_active_particles[neighbor.first];
        target_particles.insert(target_particles.end(), incoming_particles.begin(), incoming_particles.end());
        
        //std::cout << "Recv " << incoming_particles.size() << " particles from neighbor " << neighbor.first << "\n";
      }   
      for (auto& request : requests)
        request.wait();
    }
#endif
  }
}
particle_advector::round_state particle_advector::compute_round_state     (      state& state) 
{
  round_state round_state(partitioner_->partitions());
  round_state.particle_count = std::min(particles_per_round_, state.total_active_particle_count()); 
  
  auto particle_count     = std::size_t(0);
  auto maximum_iterations = std::size_t(0);
  auto compare            = [ ] (const particle_3d& lhs, const particle_3d& rhs) { return lhs.remaining_iterations < rhs.remaining_iterations; };

  // Prioritize load balanced particles.
  for (auto& neighbor : state.load_balanced_active_particles)
  {
    if (particle_count < round_state.particle_count)
    {
      const auto difference = std::min(round_state.particle_count - particle_count, neighbor.second.size());
      if (difference)
      {
        round_state.round_particles.emplace_back(neighbor.second, difference);
        particle_count += difference;
        
        const auto begin   = neighbor.second.end() - difference;
        const auto end     = neighbor.second.end();
        maximum_iterations = std::max(maximum_iterations, std::max_element(begin, end, compare)->remaining_iterations); 
      }
    }
    else
      break;
  }
  // Fill rest from local particles.
  if (particle_count < round_state.particle_count)
  {
    const auto difference = std::min(round_state.particle_count - particle_count, state.active_particles.size());
    if (difference)
    {
      round_state.round_particles.emplace_back(state.active_particles, difference);
      
      const auto begin = state.active_particles.end() - difference;
      const auto end   = state.active_particles.end();
      maximum_iterations = std::max(maximum_iterations, std::max_element(begin, end, compare)->remaining_iterations); 
    }
  }

  if (record_)
  {
    round_state.curve_stride = maximum_iterations + 2; // Two more vertices per curve; one for initial position, one for termination vertex.
    round_state.vertex_count = round_state.particle_count * round_state.curve_stride;
  }

  return round_state;
}
void                           particle_advector::allocate_integral_curves(                    const round_state& round_state, output& output) 
{
  if (!record_) return;

  output.integral_curves.emplace_back().vertices.resize(round_state.vertex_count, invalid_value<vector3>());
}
void                           particle_advector::advect                  (      state& state,       round_state& round_state, output& output)
{
  auto particle_index_offset = size(0);
  for (auto& pair : round_state.round_particles)
  {
    auto& particle_vector = pair.first ;
    auto  particle_count  = pair.second;

    tbb::parallel_for(std::size_t(0), particle_count, std::size_t(1), [&] (const std::size_t particle_index)
    {
      auto& particle        = particle_vector.get()[particle_vector.get().size() - particle_count + particle_index];
      auto& vector_field    = state.vector_fields.at(particle.relative_direction);
      auto  bounds          = aabb3(vector_field.offset, vector_field.offset + vector_field.size);
      auto  integrator      = integrator_;
      auto  iteration_index = std::size_t(0);

      if (record_)
        output.integral_curves.back().vertices[(particle_index_offset + particle_index) * round_state.curve_stride] = particle.position;

      for ( ; particle.remaining_iterations > 0; ++iteration_index, --particle.remaining_iterations)
      {
        if (!vector_field.contains(particle.position))
        {
          if (particle.relative_direction == center) // if non-load balanced particle, send to neighbor process.
          {
            std::optional<relative_direction> direction;
            if      (particle.position[0] < bounds.min()[0]) direction = negative_x;
            else if (particle.position[0] > bounds.max()[0]) direction = positive_x;
            else if (particle.position[1] < bounds.min()[1]) direction = negative_y;
            else if (particle.position[1] > bounds.max()[1]) direction = positive_y;
            else if (particle.position[2] < bounds.min()[2]) direction = negative_z;
            else if (particle.position[2] > bounds.max()[2]) direction = positive_z;
            
            if (direction && round_state.out_of_bounds_particles.find(direction.value()) != round_state.out_of_bounds_particles.end())
              round_state.out_of_bounds_particles.at(direction.value()).push_back(particle);
            else
              output.inactive_particles.push_back(particle);
          }
          else // if load balanced particle, send to original process which will then send it to neighbor process.
          {
            round_state.load_balanced_out_of_bounds_particles.at(particle.relative_direction).push_back(particle);
          }
          break;
        }

        const auto vector = vector_field.interpolate(particle.position);
        if (vector.isZero())
        {
          output.inactive_particles.push_back(particle);
          break;
        }

        const auto system = [&] (const vector3& x, vector3& dxdt, const float t) { dxdt = vector3(vector[2], vector[1], vector[0]); }; // Data-spacific.
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
          output.integral_curves.back().vertices[(particle_index_offset + particle_index) * round_state.curve_stride + iteration_index + 1] = particle.position;
      }

      if (record_)
        output.integral_curves.back().vertices[(particle_index_offset + particle_index) * round_state.curve_stride + iteration_index + 1] = terminal_value<vector3>();

      if (particle.remaining_iterations == 0)
        output.inactive_particles.push_back(particle);
    });

    particle_vector.get().resize(particle_vector.get().size() - particle_count);
    particle_index_offset += particle_count;
  }
}
void                           particle_advector::prune_integral_curves   (                                                    output& output) 
{
  if (!record_) return;

  auto& curves = output.integral_curves.back().vertices;
  curves.erase(std::remove(curves.begin(), curves.end(), invalid_value<vector3>()), curves.end());
}
void                           particle_advector::load_balance_collect    (      state& state,       round_state& round_state, output& output) 
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

    for (auto& neighbor : round_state.load_balanced_out_of_bounds_particles)
      requests.push_back(communicator->isend(partitions.at(neighbor.first).rank, 0, neighbor.second));
    for (auto& neighbor : round_state.load_balanced_out_of_bounds_particles)
    {
      concurrent_particle_vector particles;
      communicator->recv(partitions.at(neighbor.first).rank, 0, particles);
      
      auto& vector_field = state.vector_fields.at(center);
      auto  bounds       = aabb3(vector_field.offset, vector_field.offset + vector_field.size);

      tbb::parallel_for(std::size_t(0), particles.size(), std::size_t(1), [&] (const std::size_t particle_index)
      {
        auto& particle = particles[particle_index];
        particle.relative_direction = center;

        std::optional<relative_direction> direction;
        if      (particle.position[0] < bounds.min()[0]) direction = negative_x;
        else if (particle.position[0] > bounds.max()[0]) direction = positive_x;
        else if (particle.position[1] < bounds.min()[1]) direction = negative_y;
        else if (particle.position[1] > bounds.max()[1]) direction = positive_y;
        else if (particle.position[2] < bounds.min()[2]) direction = negative_z;
        else if (particle.position[2] > bounds.max()[2]) direction = positive_z;

        if (direction && round_state.out_of_bounds_particles.find(direction.value()) != round_state.out_of_bounds_particles.end())
          round_state.out_of_bounds_particles.at(direction.value()).push_back(particle);
        else
          output.inactive_particles.push_back(particle);
      });
    }

    for (auto& request : requests)
      request.wait();
#endif
  }
}                                                                                                                                                                                                                         
void                           particle_advector::out_of_bounds_distribute(      state& state, const round_state& round_state)
{
#ifdef DPA_USE_NEIGHBORHOOD_COLLECTIVES
  // TODO: Neighborhood collectives.
#else
  std::vector<boost::mpi::request> requests;

  auto  communicator = partitioner_->cartesian_communicator();
  auto& partitions   = partitioner_->partitions            ();

  for (auto& neighbor : round_state.out_of_bounds_particles)
    requests.push_back(communicator->isend(partitions.at(neighbor.first).rank, 0, neighbor.second));
  for (auto& neighbor : round_state.out_of_bounds_particles)
  {
    concurrent_particle_vector particles;
    communicator->recv(partitions.at(neighbor.first).rank, 0, particles);
    state.active_particles.insert(state.active_particles.end(), particles.begin(), particles.end());
  }

  for (auto& request : requests)
    request.wait();
#endif
}
void                           particle_advector::gather_particles        (                                                    output& output)
{
  if (!gather_particles_) return;

#ifdef DPA_FTLE_SUPPORT
  std::vector<concurrent_particle_vector> sent    (partitioner_->cartesian_communicator()->size());
  std::vector<concurrent_particle_vector> received(partitioner_->cartesian_communicator()->size());

  tbb::parallel_for(std::size_t(0), output.inactive_particles.size(), std::size_t(1), [&] (const std::size_t index)
  {
    sent[output.inactive_particles[index].original_rank].push_back(output.inactive_particles[index]);
  });

  boost::mpi::all_to_all(*partitioner_->cartesian_communicator(), sent, received);

  output.inactive_particles.clear();
  for (auto& particles : received)
    output.inactive_particles.grow_by(particles.begin(), particles.end());
#else
  std::cout << "Particles are not gathered since original ranks are unavailable. Declare DPA_FTLE_SUPPORT and rebuild." << std::endl;
#endif
}
}
