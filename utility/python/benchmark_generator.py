import json

def generate(
  config_filepath           ,
  input_dataset_filepath    ,
  input_dataset_name        ,
  input_dataset_spacing_name,
  stride                    ,
  iterations                ,
  particles_per_round       ,
  load_balancer             ,
  integrator                ,
  step_size                 ,
  gather_particles          ,
  record                    ,
  output_dataset_filepath   ):
    config = {}
    config["input_dataset_filepath"               ] = input_dataset_filepath
    config["input_dataset_name"                   ] = input_dataset_name
    config["input_dataset_spacing_name"           ] = input_dataset_spacing_name
    config["seed_generation_stride"               ] = [stride, stride, stride]
    config["seed_generation_iterations"           ] = iterations
    config["particle_advector_particles_per_round"] = particles_per_round
    config["particle_advector_load_balancer"      ] = load_balancer
    config["particle_advector_integrator"         ] = integrator
    config["particle_advector_step_size"          ] = step_size
    config["particle_advector_gather_particles"   ] = gather_particles
    config["particle_advector_record"             ] = record
    config["output_dataset_filepath"              ] = output_dataset_filepath
    config_json = json.dumps(config)
    with open(config_filepath, 'w') as file:
        json.dump(config, file, indent=2)

    # TODO: Generate accompanying sbatch shell file.

generate(
  "test.json"                                 ,
  "C:/development/data/oregon/astro.h5"       ,
  "Data3"                                     ,
  "spacing"                                   ,
  4                                           ,
  1000                                        ,
  10000                                       ,
  "diffuse_greater_limited_lesser_average"    ,
  "runge_kutta_4"                             ,
  0.001                                       ,
  False                                       ,
  True                                        ,
  "C:/development/data/oregon/astro_curves.h5")