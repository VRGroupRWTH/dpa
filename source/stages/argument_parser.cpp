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
  arguments.dataset_filepath                = json["dataset_filepath"               ]   .get<std::string>();
  arguments.dataset_name                    = json["dataset_name"                   ]   .get<std::string>();
  arguments.dataset_spacing_name            = json["dataset_spacing_name"           ]   .get<std::string>();
  arguments.seed_generation_stride[0]       = json["seed_generation_stride"         ][0].get<scalar>     ();
  arguments.seed_generation_stride[1]       = json["seed_generation_stride"         ][1].get<scalar>     ();
  arguments.seed_generation_stride[2]       = json["seed_generation_stride"         ][2].get<scalar>     ();
  arguments.seed_generation_iterations      = json["seed_generation_iterations"     ]   .get<integer>    ();
  arguments.particle_advector_load_balancer = json["particle_advector_load_balancer"]   .get<std::string>();
  arguments.particle_advector_integrator    = json["particle_advector_integrator"   ]   .get<std::string>();
  arguments.particle_advector_step_size     = json["particle_advector_step_size"    ]   .get<scalar>     ();
  arguments.particle_advector_record        = json["particle_advector_record"       ]   .get<bool>       ();
  return arguments;
}
}