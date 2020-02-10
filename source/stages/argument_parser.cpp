#include <dpa/stages/argument_parser.hpp>

#include <fstream>

#include <boost/lexical_cast.hpp>
#include <nlohmann/json.hpp>

namespace dpa
{
arguments argument_parser::parse(const std::string& filepath)
{ 
  std::ifstream  file(filepath);
  nlohmann::json json;
  file >> json;

  arguments arguments;
  arguments.input_dataset_filepath             = json["input_dataset_filepath"            ].get<std::string>();
  arguments.input_dataset_name                 = json["input_dataset_name"                ].get<std::string>();
  arguments.input_dataset_spacing_name         = json["input_dataset_spacing_name"        ].get<std::string>();
  arguments.particle_advector_load_balancer    = json["particle_advector_load_balancer"   ].get<std::string>();
  arguments.particle_advector_integrator       = json["particle_advector_integrator"      ].get<std::string>();
  arguments.particle_advector_step_size        = json["particle_advector_step_size"       ].get<scalar>     ();
  arguments.particle_advector_gather_particles = json["particle_advector_gather_particles"].get<bool>       ();
  arguments.particle_advector_record           = json["particle_advector_record"          ].get<bool>       ();
  arguments.estimate_ftle                      = json["estimate_ftle"                     ].get<bool>       ();
  arguments.output_dataset_filepath            = json["output_dataset_filepath"           ].get<std::string>();

  // Due to limitations of JSON, 64 bit integers are stored as strings.
  arguments.seed_generation_iterations            = boost::lexical_cast<std::size_t>(json["seed_generation_iterations"           ].get<std::string>());
  arguments.particle_advector_particles_per_round = boost::lexical_cast<std::size_t>(json["particle_advector_particles_per_round"].get<std::string>());

  if (json.contains("seed_generation_stride"))
  {
    auto stride = json["seed_generation_stride"];
    arguments.seed_generation_stride = vector3(stride[0].get<scalar>(), stride[1].get<scalar>(), stride[2].get<scalar>());
  }
  if (json.contains("seed_generation_count"))
  {
    auto count  = json["seed_generation_count" ];
    arguments.seed_generation_count  = boost::lexical_cast<std::size_t>(count.get<std::string>());
  }
  if (json.contains("seed_generation_range"))
  {
    auto range  = json["seed_generation_range" ];
    arguments.seed_generation_range  = svector2(
      boost::lexical_cast<std::size_t>(range[0].get<std::string>()), 
      boost::lexical_cast<std::size_t>(range[1].get<std::string>()));
  }
  if (json.contains("seed_generation_boundaries"))
  {
    auto boundaries = json["seed_generation_boundaries"];
    auto minimum    = boundaries["minimum"];
    auto maximum    = boundaries["maximum"];
    arguments.seed_generation_boundaries = aabb3(
      vector3(minimum[0].get<scalar>(), minimum[1].get<scalar>(), minimum[2].get<scalar>()),
      vector3(maximum[0].get<scalar>(), maximum[1].get<scalar>(), maximum[2].get<scalar>()));
  } 

  return arguments;
}
}