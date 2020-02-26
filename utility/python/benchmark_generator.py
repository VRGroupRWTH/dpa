import json
import os
from pathlib import Path

# $1: configuration name 
# $2: number of nodes
script_template = """#!/bin/bash
#SBATCH --job-name=$1
#SBATCH --output=$1.log
#SBATCH --time=00:20:00
#SBATCH --mem=128000M
#SBATCH --nodes=$2
#SBATCH --ntasks-per-node=1
#SBATCH --sockets-per-node=4
#SBATCH --cores-per-socket=12
#SBATCH --cpus-per-task=48
#SBATCH --account=rwth0432
module swap intelmpi openmpi/4.0.2
module load gcc/9 cmake/3.13.2
/hpcwork/rwth0432/source/dpa/build/vcpkg/installed/x64-linux/bin/mpiexec $FLAGS_MPI_BATCH --mca btl_openib_allow_ib 1 /hpcwork/rwth0432/source/dpa/build/dpa $1.json
"""

def generate(
  prefix           ,
  nodes            ,
  load_balancer    ,
  dataset_filepath ,
  seed_distribution,
  seed_stride      ):

  load_balancer_full = "none"
  if (load_balancer == "const"):
    load_balancer_full = "diffuse_constant"
  if (load_balancer == "lma"):
    load_balancer_full = "diffuse_lesser_average"
  if (load_balancer == "gllma"):
    load_balancer_full = "diffuse_greater_limited_lesser_average"
  
  name = (Path(dataset_filepath).resolve().stem +
    "_n_" + str(nodes) + 
    "_l_" + load_balancer +
    "_d_" + str(seed_distribution) +
    "_s_" + str(seed_stride[0]) + "," + str(seed_stride[1]) + "," + str(seed_stride[2]))

  half_distribution = seed_distribution / 2.0
  boundaries = {
    "minimum": [0.5 - half_distribution, 0.5 - half_distribution, 0.5 - half_distribution],
    "maximum": [0.5 + half_distribution, 0.5 + half_distribution, 0.5 + half_distribution]}

  script = script_template.replace("$1", name).replace("$2", str(nodes))
  with open(prefix + name + ".sh", 'w') as file:
    file.write(script)

  configuration = {}
  configuration["input_dataset_filepath"               ] = dataset_filepath
  configuration["input_dataset_name"                   ] = "Data3"
  configuration["input_dataset_spacing_name"           ] = "spacing"
  configuration["seed_generation_stride"               ] = [seed_stride[0], seed_stride[1], seed_stride[2]]
  configuration["seed_generation_iterations"           ] = "1000"
  configuration["seed_generation_boundaries"           ] = boundaries
  configuration["particle_advector_particles_per_round"] = "10000000"
  configuration["particle_advector_load_balancer"      ] = load_balancer_full
  configuration["particle_advector_integrator"         ] = "runge_kutta_4"
  configuration["particle_advector_step_size"          ] = 0.001
  configuration["particle_advector_gather_particles"   ] = False
  configuration["particle_advector_record"             ] = True
  configuration["estimate_ftle"                        ] = False
  configuration["output_dataset_filepath"              ] = name + ".h5"
  with open(prefix + name + ".json", 'w') as file:
    json.dump(configuration, file, indent=2)

def generate_strong_scaling(
  nodes            ,
  load_balancers   ,
  dataset_filepaths):
  prefix = "../config/strong_scaling/"
  Path(prefix).mkdir(parents=True, exist_ok=True)
  for n in nodes: 
    for l in load_balancers:
      for f in dataset_filepaths:
        generate(prefix, n, l, f, 1.0, [4, 4, 4])

def generate_weak_scaling(
  nodes            ,
  load_balancers   ,
  dataset_filepaths,
  strides          ):
  prefix = "../config/weak_scaling/"
  Path(prefix).mkdir(parents=True, exist_ok=True)
  for i, n in enumerate(nodes): 
    for l in load_balancers:
      for f in dataset_filepaths:
        generate(prefix, n, l, f, 1.0, strides[i])

def generate_load_balancing(
  load_balancers   ,
  dataset_filepaths):
  prefix = "../config/load_balancing/"
  Path(prefix).mkdir(parents=True, exist_ok=True)
  for l in load_balancers:
    for f in dataset_filepaths:
      generate(prefix, 64, l, f, 0.5, [2, 2, 2])

def generate_parameter_space(
  nodes             ,
  load_balancers    ,
  dataset_filepaths ,
  dataset_scales    ,
  seed_distributions,
  seed_strides      ):
  default_dataset      = "/hpcwork/rwth0432/data/oregon/astro_1024.h5"
  default_distribution = 1.0
  default_stride       = [8, 8, 8]

  prefix = "../config/parameter_space/dataset_complexity/"
  Path(prefix).mkdir(parents=True, exist_ok=True)
  for n in nodes: 
    for l in load_balancers:
      for f in dataset_filepaths:
        generate(prefix, n, l, f, default_distribution, default_stride)

  prefix = "../config/parameter_space/dataset_size/"
  Path(prefix).mkdir(parents=True, exist_ok=True)
  for n in nodes: 
    for l in load_balancers:
      for s in dataset_scales:
        generate(prefix, n, l, s, default_distribution, default_stride)
        
  prefix = "../config/parameter_space/seed_distribution/"
  Path(prefix).mkdir(parents=True, exist_ok=True)
  for n in nodes: 
    for l in load_balancers:
      for d in seed_distributions:
        generate(prefix, n, l, default_dataset, d, default_stride)

  prefix = "../config/parameter_space/seed_size/"
  Path(prefix).mkdir(parents=True, exist_ok=True)
  for n in nodes: 
    for l in load_balancers:
      for s in seed_strides:
        generate(prefix, n, l, default_dataset, default_distribution, s)

nodes             = [16, 32, 64, 128]
load_balancers    = ["none", "const", "lma", "gllma"]
dataset_filepaths = ["/hpcwork/rwth0432/data/oregon/astro_1024.h5", "/hpcwork/rwth0432/data/oregon/fishtank_1024.h5", "/hpcwork/rwth0432/data/oregon/fusion_1024.h5"]
dataset_scales    = ["/hpcwork/rwth0432/data/oregon/astro_1024.h5", "/hpcwork/rwth0432/data/oregon/astro_1536.h5"   , "/hpcwork/rwth0432/data/oregon/astro_2048.h5" ]
distributions     = [1.0, 0.5, 0.25]
strides           = [[8,8,8], [8,8,4], [8,4,4], [4,4,4]]
generate_strong_scaling (nodes, load_balancers, dataset_filepaths)
generate_weak_scaling   (nodes, load_balancers, dataset_filepaths, strides)
generate_load_balancing (       load_balancers, dataset_filepaths)
generate_parameter_space(nodes, load_balancers, dataset_filepaths, dataset_scales, distributions, strides)
