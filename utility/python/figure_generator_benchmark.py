import json
import os
from pathlib import Path

# $1: configuration name
script_template = """#!/bin/bash
#SBATCH --job-name=$1
#SBATCH --output=$1.log
#SBATCH --time=00:10:00
#SBATCH --mem=128000M
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=1
#SBATCH --sockets-per-node=4
#SBATCH --cores-per-socket=12
#SBATCH --cpus-per-task=48
#SBATCH --account=rwth0432
module unload intelmpi
module load gcc/9 cmake/3.13.2
/hpcwork/rwth0432/source/dpa/build/vcpkg/installed/x64-linux/bin/mpiexec $FLAGS_MPI_BATCH --mca btl_openib_allow_ib 1 /hpcwork/rwth0432/source/dpa/build/dpa $1.json
"""

def generate(filepath):
  name = (Path(filepath).resolve().stem + "_figure_run")

  script = script_template.replace("$1", name)
  with open("../config/" + name + ".sh", 'w') as file:
    file.write(script)

  configuration = {}
  configuration["input_dataset_filepath"               ] = filepath
  configuration["input_dataset_name"                   ] = "Data3"
  configuration["input_dataset_spacing_name"           ] = "spacing"
  configuration["seed_generation_stride"               ] = [1, 1, 1]
  configuration["seed_generation_iterations"           ] = "1000"
  configuration["particle_advector_particles_per_round"] = "100000000"
  configuration["particle_advector_load_balancer"      ] = "none"
  configuration["particle_advector_integrator"         ] = "runge_kutta_4"
  configuration["particle_advector_step_size"          ] = 0.001
  configuration["particle_advector_gather_particles"   ] = True
  configuration["particle_advector_record"             ] = True
  configuration["estimate_ftle"                        ] = True
  configuration["output_dataset_filepath"              ] = name + ".h5"
  with open("../config/" + name + ".json", 'w') as file:
    json.dump(configuration, file, indent=2)

Path("../config").mkdir(parents=True, exist_ok=True)
dataset_filepaths = ["/hpcwork/rwth0432/data/oregon/astro.h5", "/hpcwork/rwth0432/data/oregon/fishtank.h5", "/hpcwork/rwth0432/data/oregon/fusion.h5"]
for filepath in dataset_filepaths:
  generate(filepath)