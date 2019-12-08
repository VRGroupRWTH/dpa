#include <dpa/pipeline.hpp>

#include <boost/mpi/environment.hpp>

#include <dpa/stages/argument_parser.hpp>
#include <dpa/stages/cartesian_grid_loader.hpp>
#include <dpa/stages/domain_partitioner.hpp>
#include <dpa/stages/integral_curve_saver.hpp>
#include <dpa/stages/particle_advector.hpp>
#include <dpa/stages/uniform_seed_generator.hpp>

namespace dpa
{
std::int32_t pipeline::run(std::int32_t argc, char** argv)
{
  boost::mpi::environment environment(argc, argv, boost::mpi::threading::level::multiple);
  
  auto arguments   = argument_parser::parse(argv[1]);
  auto partitioner = domain_partitioner    ();
  auto loader      = cartesian_grid_loader (&partitioner, arguments.dataset_filepath, arguments.dataset_name, arguments.dataset_spacing_name);
  partitioner.set_domain_size(loader.load_dimensions());

  auto                           local_vector_field = loader.load_local_vector_field();
  std::optional<vector_field_3d> positive_x_vector_field;
  std::optional<vector_field_3d> negative_x_vector_field;
  std::optional<vector_field_3d> positive_y_vector_field;
  std::optional<vector_field_3d> negative_y_vector_field;
  std::optional<vector_field_3d> positive_z_vector_field;
  std::optional<vector_field_3d> negative_z_vector_field;
  if (arguments.particle_advector_load_balancer == "local")
  {
    positive_x_vector_field = loader.load_positive_x_vector_field();
    negative_x_vector_field = loader.load_negative_x_vector_field();
    positive_y_vector_field = loader.load_positive_y_vector_field();
    negative_y_vector_field = loader.load_negative_y_vector_field();
    positive_z_vector_field = loader.load_positive_z_vector_field();
    negative_z_vector_field = loader.load_negative_z_vector_field();
  }

  auto seeds = uniform_seed_generator::generate(
    local_vector_field.offset, 
    local_vector_field.size, 
    arguments.seed_generation_stride.array() * local_vector_field.spacing.array(), 
    arguments.seed_generation_iterations, 
    partitioner.communicator()->rank());

  auto advector = particle_advector(arguments.particle_advector_step_size, arguments.particle_advector_integrator);
  advector.set_seeds        (seeds);
  advector.set_vector_fields(local_vector_field, positive_x_vector_field, negative_x_vector_field, positive_y_vector_field, negative_y_vector_field, positive_z_vector_field, negative_z_vector_field); // Bad. Group to map.
  advector.advect           (); // Register a callback and record the positions as integral_curve.

  if (arguments.particle_advector_record)
  {
    integral_curve_saver saver;
    saver.save(curves);
  }

  return 0;
}
}
