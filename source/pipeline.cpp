#include <dpa/pipeline.hpp>

#include <boost/mpi/environment.hpp>

#include <dpa/benchmark/benchmark.hpp>
#include <dpa/stages/argument_parser.hpp>
#include <dpa/stages/color_generator.hpp>
#include <dpa/stages/ftle_estimator.hpp>
#include <dpa/stages/index_generator.hpp>
#include <dpa/stages/regular_grid_loader.hpp>
#include <dpa/stages/regular_grid_saver.hpp>
#include <dpa/stages/domain_partitioner.hpp>
#include <dpa/stages/integral_curve_saver.hpp>
#include <dpa/stages/particle_advector.hpp>
#include <dpa/stages/uniform_seed_generator.hpp>

#undef min
#undef max

namespace dpa
{
std::int32_t pipeline::run(std::int32_t argc, char** argv)
{
  boost::mpi::environment environment(argc, argv, boost::mpi::threading::level::serialized);
  std::cout << "Started pipeline on " << environment.processor_name() << "\n";

  auto arguments         = argument_parser::parse(argv[1]);
  auto benchmark_session = run_mpi<float, std::milli>([&] (session_recorder<float, std::milli>& recorder)
  {
    auto partitioner     = domain_partitioner ();
    auto loader          = regular_grid_loader(
      &partitioner                        , 
      arguments.input_dataset_filepath    , 
      arguments.input_dataset_name        , 
      arguments.input_dataset_spacing_name);
    auto advector        = particle_advector(
      &partitioner                                   , 
      arguments.particle_advector_particles_per_round,
      arguments.particle_advector_load_balancer      , 
      arguments.particle_advector_integrator         ,
      arguments.particle_advector_step_size          ,
      arguments.particle_advector_gather_particles   ,
      arguments.particle_advector_record             );

    auto vector_fields = std::unordered_map<relative_direction, regular_vector_field_3d>();
    auto particles     = std::vector<particle_3d>();
    auto ftle_field    = std::optional<regular_scalar_field_3d>();

    std::cout << "domain_partitioning\n";
    partitioner.set_domain_size(loader.load_dimensions(), svector3::Ones());

    std::cout << "data_loading\n";
    vector_fields = loader.load_vector_fields(
      arguments.particle_advector_load_balancer == "diffuse_constant"                       || 
      arguments.particle_advector_load_balancer == "diffuse_lesser_average"                 || 
      arguments.particle_advector_load_balancer == "diffuse_greater_limited_lesser_average" );

    std::cout << "seed_generation\n";
    const auto offset        = vector_fields[center].spacing.array() * partitioner.partitions().at(center).offset.cast<scalar>().array();
    const auto size          = vector_fields[center].spacing.array() * partitioner.block_size()                  .cast<scalar>().array();
    const auto iterations    = arguments.seed_generation_iterations;
    const auto process_index = partitioner.cartesian_communicator()->rank();
    const auto boundaries    = arguments.seed_generation_boundaries ? arguments.seed_generation_boundaries : std::nullopt;

    if      (arguments.seed_generation_stride)
      particles = uniform_seed_generator::generate(
        offset       ,
        size         , 
        vector_fields[center].spacing.array() * arguments.seed_generation_stride->array(),
        iterations   , 
        process_index,
        boundaries   );
    else if (arguments.seed_generation_count)
      particles = uniform_seed_generator::generate_random(
        offset       ,
        size         , 
        *arguments.seed_generation_count,
        iterations   , 
        process_index,
        boundaries   );
    else if (arguments.seed_generation_range)
      particles = uniform_seed_generator::generate_random(
        offset       ,
        size         , 
        *arguments.seed_generation_range,
        iterations   , 
        process_index,
        boundaries   );

    std::cout << "particle_advection\n";
    particle_advector::state       state       = {vector_fields, particles, partitioner.partitions()};
    particle_advector::round_state round_state = particle_advector::round_state(partitioner.partitions());
    particle_advector::output      output      = {};
    integer                        rounds      = 0;
    bool                           complete    = false;

    partitioner.cartesian_communicator()->barrier();
    recorder.record("total_time", [&] ()
    {
      while (!complete)
      {
        recorder.set   ("round." + std::to_string(rounds) + ".load", state.total_active_particle_count());
        recorder.record("round." + std::to_string(rounds) + ".time", [&] ()
        {
          // partitioner.cartesian_communicator()->barrier();
          // std::cout << "load_balance_distribute\n";
                        advector.load_balance_distribute (state);
          
          // partitioner.cartesian_communicator()->barrier();
          // std::cout << "compute_round_state\n";
          round_state = advector.compute_round_state     (state);
          
          // partitioner.cartesian_communicator()->barrier();
          // std::cout << "allocate_integral_curves\n";
                        advector.allocate_integral_curves(       round_state, output);
          
          // partitioner.cartesian_communicator()->barrier();
          // std::cout << "advect\n";
                        advector.advect                  (state, round_state, output);
                        
          // partitioner.cartesian_communicator()->barrier();
          // std::cout << "load_balance_collect\n";
                        advector.load_balance_collect    (state, round_state, output);
          
          // partitioner.cartesian_communicator()->barrier();
          // std::cout << "out_of_bounds_distribute\n";
                        advector.out_of_bounds_distribute(state, round_state);

          // partitioner.cartesian_communicator()->barrier();
          // std::cout << "check_completion\n";
          complete    = advector.check_completion        (state);
          
          // partitioner.cartesian_communicator()->barrier();
          // std::cout <<"prune_integral_curves\n";
                        advector.prune_integral_curves   (                    output);
          rounds++;
        });
      }
    });
    partitioner.cartesian_communicator()->barrier();

    std::cout << "gather_particles\n";
    advector.gather_particles(output);

    std::cout << "prune_integral_curves\n";
    advector.prune_integral_curves(output);
    
    std::cout << "index_generation\n";
    if (arguments.particle_advector_record)
      index_generator::generate(output.integral_curves, arguments.particle_advector_particles_per_round * arguments.seed_generation_iterations > std::numeric_limits<std::uint32_t>::max());
    
    std::cout << "color_generation\n";
    if (arguments.particle_advector_record)
      color_generator::generate_from_angular_velocities(output.integral_curves);

    std::cout << "save_integral_curves\n";
    if (arguments.particle_advector_record)
      integral_curve_saver(&partitioner, arguments.output_dataset_filepath).save(output.integral_curves);

    std::cout << "estimate_ftle\n";
    if (arguments.estimate_ftle)
      ftle_field = ftle_estimator::estimate(vector_fields.at(center), arguments.seed_generation_iterations, arguments.seed_generation_stride.value(), arguments.particle_advector_step_size, output.inactive_particles);
    
    std::cout << "save_ftle_field\n";
    if (arguments.estimate_ftle)
      regular_grid_saver(&partitioner, arguments.output_dataset_filepath).save(ftle_field.value());
  }, 1);
  benchmark_session.gather();
  benchmark_session.to_csv(arguments.output_dataset_filepath + ".benchmark.csv");
  return 0;
}
}
