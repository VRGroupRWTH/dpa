#include <dpa/stages/argument_parser.hpp>

#include <fstream>

#include <nlohmann/json.hpp>

namespace dpa
{
arguments argument_parser::parse(const std::string& filepath)
{ 
  std::ifstream  file(filepath);
  nlohmann::json json;
  file >> json;

  arguments arguments;
  arguments.input_dataset_filepath                = json["input_dataset_filepath"               ]   .get<std::string>();
  arguments.input_dataset_name                    = json["input_dataset_name"                   ]   .get<std::string>();
  arguments.input_dataset_spacing_name            = json["input_dataset_spacing_name"           ]   .get<std::string>();
  arguments.seed_generation_stride[0]             = json["seed_generation_stride"               ][0].get<scalar>     ();
  arguments.seed_generation_stride[1]             = json["seed_generation_stride"               ][1].get<scalar>     ();
  arguments.seed_generation_stride[2]             = json["seed_generation_stride"               ][2].get<scalar>     ();
  arguments.seed_generation_iterations            = json["seed_generation_iterations"           ]   .get<integer>    ();
  arguments.particle_advector_particles_per_round = json["particle_advector_particles_per_round"]   .get<integer>    ();
  arguments.particle_advector_load_balancer       = json["particle_advector_load_balancer"      ]   .get<std::string>();
  arguments.particle_advector_integrator          = json["particle_advector_integrator"         ]   .get<std::string>();
  arguments.particle_advector_step_size           = json["particle_advector_step_size"          ]   .get<scalar>     ();
  arguments.particle_advector_gather_particles    = json["particle_advector_gather_particles"   ]   .get<bool>       ();
  arguments.particle_advector_record              = json["particle_advector_record"             ]   .get<bool>       ();
  arguments.output_dataset_filepath               = json["output_dataset_filepath"              ]   .get<std::string>();

  if (json.contains("seed_generation_boundaries"))
  {
    auto boundaries = json["seed_generation_boundaries"];
    arguments.seed_generation_boundaries = aabb3(
      vector3(boundaries["minimum"][0].get<scalar>(), boundaries["minimum"][1].get<scalar>(), boundaries["minimum"][2].get<scalar>()),
      vector3(boundaries["maximum"][0].get<scalar>(), boundaries["maximum"][1].get<scalar>(), boundaries["maximum"][2].get<scalar>()));
  } 

  return arguments;
}
}