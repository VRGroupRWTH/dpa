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

    std::cout    << "1.domain_partitioning\n";
    recorder.record("1.domain_partitioning", [&] ()
    {
      partitioner.set_domain_size(loader.load_dimensions(), svector3::Ones());
    });
    partitioner.cartesian_communicator()->barrier();
    std::cout    << "2.data_loading\n";
    recorder.record("2.data_loading"       , [&] ()
    {
      vector_fields = loader.load_vector_fields(
        arguments.particle_advector_load_balancer == "diffuse_constant"                       || 
        arguments.particle_advector_load_balancer == "diffuse_lesser_average"                 || 
        arguments.particle_advector_load_balancer == "diffuse_greater_limited_lesser_average" );
    });
    partitioner.cartesian_communicator()->barrier();
    std::cout    << "3.seed_generation\n";
    recorder.record("3.seed_generation"    , [&] ()
    {
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
    });

    partitioner.cartesian_communicator()->barrier();
    particle_advector::state  state    = {vector_fields, particles, partitioner.partitions()};
    particle_advector::output output   = {};
    integer                   rounds   = 0;
    bool                      complete = false;
    while (!complete)
    {
      particle_advector::round_state round_state(partitioner.partitions());
      std::cout    << "4.1." + std::to_string(rounds) + ".load_balance_distribute\n";
      recorder.record("4.1." + std::to_string(rounds) + ".load_balance_distribute" , [&] ()
      {
                     advector.load_balance_distribute (state);
      });
      std::cout    << "4.2." + std::to_string(rounds) + ".compute_round_state\n";
      recorder.record("4.2." + std::to_string(rounds) + ".compute_round_state"      , [&] ()
      {
        round_state = advector.compute_round_state    (state);
      });
      std::cout    << "4.3." + std::to_string(rounds) + ".allocate_integral_curves\n";
      recorder.record("4.3." + std::to_string(rounds) + ".allocate_integral_curves", [&] ()
      {
                     advector.allocate_integral_curves(       round_state, output);
      });
      std::cout    << "4.4." + std::to_string(rounds) + ".advect\n";
      recorder.record("4.4." + std::to_string(rounds) + ".advect"                  , [&] ()
      {
                     advector.advect                  (state, round_state, output);
      });
      std::cout    << "4.5." + std::to_string(rounds) + ".load_balance_collect\n";
      recorder.record("4.5." + std::to_string(rounds) + ".load_balance_collect"    , [&] ()
      {
                     advector.load_balance_collect    (state, round_state, output);
      });
      std::cout    << "4.6." + std::to_string(rounds) + ".out_of_bounds_distribute\n";
      recorder.record("4.6." + std::to_string(rounds) + ".out_of_bounds_distribute", [&] ()
      {
                     advector.out_of_bounds_distribute(state, round_state);
      });
      std::cout    << "4.7." + std::to_string(rounds) + ".check_completion\n";
      recorder.record("4.7." + std::to_string(rounds) + ".check_completion"        , [&] ()
      {
        complete =   advector.check_completion        (state);
      });  
      rounds++;
    }
    partitioner.cartesian_communicator()->barrier();
    std::cout    << "4.8.gather_particles\n";
    recorder.record("4.8.gather_particles"     , [&] ()
    {
      advector.gather_particles     (output);
    });
    partitioner.cartesian_communicator()->barrier();
    std::cout    << "4.9.prune_integral_curves\n";
    recorder.record("4.9.prune_integral_curves", [&] ()
    {
      advector.prune_integral_curves(output);
    });
    
    partitioner.cartesian_communicator()->barrier();
    std::cout    << "5.index_generation\n";
    recorder.record("5.index_generation"       , [&] ()
    {
      if (arguments.particle_advector_record)
        index_generator::generate(output.integral_curves, arguments.particle_advector_particles_per_round * arguments.seed_generation_iterations > std::numeric_limits<std::uint32_t>::max());
    });
    partitioner.cartesian_communicator()->barrier();
    std::cout    << "6.color_generation\n";
    recorder.record("6.color_generation"       , [&] ()
    {
      if (arguments.particle_advector_record)
        color_generator::generate_from_angular_velocities(output.integral_curves);
    });

    partitioner.cartesian_communicator()->barrier();
    std::cout    << "7.save_integral_curves\n";
    recorder.record("7.save_integral_curves"   , [&] ()
    {
      if (arguments.particle_advector_record)
        integral_curve_saver(&partitioner, arguments.output_dataset_filepath).save(output.integral_curves);
    });

    partitioner.cartesian_communicator()->barrier();
    std::cout    << "8.estimate_ftle\n";
    recorder.record("8.estimate_ftle"          , [&] ()
    {
      // Note: FTLE requires stride (i.e. regular seed generation).
      if (arguments.estimate_ftle)
        ftle_field = ftle_estimator::estimate(vector_fields.at(center), arguments.seed_generation_iterations, arguments.seed_generation_stride.value(), arguments.particle_advector_step_size, output.inactive_particles);
    });
    partitioner.cartesian_communicator()->barrier();
    std::cout    << "9.save_ftle_field\n";
    recorder.record("9.save_ftle_field"        , [&] ()
    {
      if (arguments.estimate_ftle)
        regular_grid_saver(&partitioner, arguments.output_dataset_filepath).save(ftle_field.value());
    });
  }, 1);
  benchmark_session.gather();
  benchmark_session.to_csv(arguments.output_dataset_filepath + ".benchmark.csv");
  return 0;
}
}
