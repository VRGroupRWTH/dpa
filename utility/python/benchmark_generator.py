import json
from pathlib import Path

# $1: configuration name 
# $2: application filepath 
# $3: number of nodes
script_template = """#!/bin/bash
#SBATCH --job-name=$1
#SBATCH --output=$1.log
#SBATCH --time=00:10:00
#SBATCH --mem=128000M
#SBATCH --nodes=$3
#SBATCH --ntasks-per-node=1
#SBATCH --sockets-per-node=4
#SBATCH --cores-per-socket=12
#SBATCH --account=rwth0432
$MPIEXEC $FLAGS_MPI_BATCH $2 $1.json
"""

def generate(
  nodes                  ,
  input_dataset_filepath ,
  stride                 ,
  iterations             ,
  boundaries             ,
  particles_per_round    ,
  load_balancer          ):
  load_balancer_shorthand = "none"
  if (load_balancer == "diffuse_constant"):
    load_balancer_shorthand = "const"
  if (load_balancer == "diffuse_lesser_average"):
    load_balancer_shorthand = "lma"
  if (load_balancer == "diffuse_greater_limited_lesser_average"):
    load_balancer_shorthand = "gllma"
  
  name = (Path(input_dataset_filepath).resolve().stem + 
    "_n"   + str(nodes)  + 
    "_s"   + str(stride) + 
    "_i"   + iterations  + 
    "_b"   + str(boundaries["minimum"][0]) + ","
           + str(boundaries["minimum"][1]) + ","
           + str(boundaries["minimum"][2]) + ","
           + str(boundaries["maximum"][0]) + ","
           + str(boundaries["maximum"][1]) + ","
           + str(boundaries["maximum"][2]) +
    "_ppr" + particles_per_round + 
    "_lb_" + load_balancer_shorthand)

  script = (script_template.
    replace("$1", name).
    replace("$2", "/hpcwork/ad784563/source/dpa/build/dpa").
    replace("$3", str(nodes)))
  with open("../configs/" + name + ".sh", 'w') as file:
    file.write(script)

  configuration = {}
  configuration["input_dataset_filepath"               ] = input_dataset_filepath
  configuration["input_dataset_name"                   ] = "Data3"
  configuration["input_dataset_spacing_name"           ] = "spacing"
  configuration["seed_generation_stride"               ] = [stride, stride, stride]
  configuration["seed_generation_iterations"           ] = iterations
  configuration["seed_generation_boundaries"           ] = boundaries
  configuration["particle_advector_particles_per_round"] = particles_per_round
  configuration["particle_advector_load_balancer"      ] = load_balancer
  configuration["particle_advector_integrator"         ] = "runge_kutta_4"
  configuration["particle_advector_step_size"          ] = 0.001
  configuration["particle_advector_gather_particles"   ] = True
  configuration["particle_advector_record"             ] = True
  configuration["output_dataset_filepath"              ] = name + ".h5"
  with open("../configs/" + name + ".json", 'w') as file:
    json.dump(configuration, file, indent=2)

def combine(
  nodes                  ,
  input_dataset_filepath ,
  stride                 ,
  iterations             ,
  boundaries             ,
  particles_per_round    ,
  load_balancer          ):
  for n in nodes: 
    for d in input_dataset_filepath:
      for s in stride:
        for i in iterations:
          for b in boundaries:
            for ppr in particles_per_round:
              for lb in load_balancer:
                generate(n, d, s, i, b, ppr, lb)

combine(
  [32, 64, 128, 256],
  ["/hpcwork/ad784563/data/oregon/astro.h5", "/hpcwork/ad784563/data/oregon/fishtank.h5", "/hpcwork/ad784563/data/oregon/fusion.h5"],
  [1.0, 2.0, 4.0, 8.0],
  ["1000", "10000"],
  [{"minimum": [0.4, 0.4, 0.4], "maximum": [0.6, 0.6, 0.6]}],
  ["10000000", "100000000"],
  ["none", "diffuse_constant", "diffuse_lesser_average", "diffuse_greater_limited_lesser_average"]
)