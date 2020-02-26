import csv

# Format:
# [
#   {rank, name          , iteration_0},
#   {0   , "total_time"  , 789.0      },
#   {0   , "round.0.load", 420        },
#   {0   , "round.0.time", 123.4      },
#   {0   , "round.1.load", 210        },
#   {0   , "round.1.time", 456.4      },
#   ...
#   {1   , "total_time"  , 789.0      },
#   {1   , "round.0.load", 420        },
#   {1   , "round.0.time", 123.4      },
#   {1   , "round.1.load", 210        },
#   {1   , "round.1.time", 456.4      },
#   ...
# ]
def parse_csv(filepath):
  rows = []
  with open(filepath, mode='r') as csv_file:
    csv_reader = csv.DictReader(csv_file)
    for row in csv_reader:
      rows.append(row)
    return rows

# Call on per algorithm per dataset in strong and weak scaling.
# Call on per algorithm per varying in parameter space.
# Format: Nodes to maximum total advection time. Example: 
# {
#   "16" : 789.0,
#   "32" : 345.6, 
#   "64" : 123.4,
#   "128": 45.6
# }
def parse_scaling_benchmark(filepaths):
  rows = parse_csv(filepath)
  # TODO

# Call on per algorithm per dataset in load balancing.
# Format: Rounds to rank to load and time. Example:
# {
#   "round_0": 
#   {
#     "rank_0":
#     {
#       "load": 420,
#       "time": 123.4
#     },
#     ...
#   },
#   ...
# }
def parse_load_balancing_benchmark(filepath):
  rows = parse_csv(filepath)
  # TODO

# Computed as t_0 / t_i.
def compute_speedup(total_benchmark):
  # TODO

# Computed per round as load_max / load_avg.
def compute_load_imbalance(load_balancing_benchmark):
  # TODO

# Call on per dataset in strong and weak scaling.
# Call on per varying in parameter space.
# Line plots of ranks against total times.
def plot_scaling_benchmark(algorithm_total_benchmarks):
  # TODO

# Call on per algorithm per dataset in load balancing.
# Gantt chart of ranks against round times.
def plot_load_balancing_benchmark(load_balancing_benchmark):
  # TODO

if __name__ == "__main__":
  # TODO