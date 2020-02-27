import csv
import pprint

class auto_resize_list(list):
  def __setitem__(self, index, value):
    if index >= len(self):
      self.extend([None] * (index + 1 - len(self)))
    list.__setitem__(self, index, value)

"""Format:
[ # Array ordered by ranks.
  {
    "total_time": ...
    "rounds"    :
    [ # Array ordered by rounds.
      {load: ..., time: ..., load_balancing_time: ..., advection_time: ..., communication_time: ...},
      {load: ..., time: ..., load_balancing_time: ..., advection_time: ..., communication_time: ...},
      {load: ..., time: ..., load_balancing_time: ..., advection_time: ..., communication_time: ...},
      ...
    ]
  },
  ...
]"""
def parse_csv(filepath):
  ranks = auto_resize_list()
  with open(filepath, mode='r') as csv_file:
    csv_reader = csv.DictReader(csv_file)
    for row in csv_reader:
      rank  = int  (row["rank"])
      name  =       row["name"]
      value = float(row["iteration 0"])
      
      if rank >= len(ranks): 
        ranks[rank] = {"rounds": auto_resize_list()}

      if name == "total_time":
        ranks[rank]["total_time"] = value
      else:
        split_name  = name.split(".")
        round_index = int(split_name[1])
        value_type  =     split_name[2]
        if round_index >= len(ranks[rank]["rounds"]):
          ranks[rank]["rounds"][round_index] = {}
        ranks[rank]["rounds"][round_index][value_type] = value
  return ranks

"""Format:
{
  "nodes"   : [16   , 32   , 64   , 128  , ...]
  "times"   : [999.9, 666.6, 333.3, 111.1, ...]
  "speedups": [1    , 1.5  , 3    , 9    , ...]
}
"""
#def parse_scaling_benchmarks(benchmarks):
  # TODO Parse. Start with the benchmark names to get node counts.

"""Format:
[ # Array ordered by rounds.
  {
    "boundary"      : ...,
    "load_imbalance": ...,
    "times"         :
    [ # Array ordered by ranks.
      {load_balancing_time: ..., advection_time: ..., communication_time: ...},
      {load_balancing_time: ..., advection_time: ..., communication_time: ...},
      {load_balancing_time: ..., advection_time: ..., communication_time: ...}
    ]
  },
  ...
]
"""
#def parse_load_balancing_benchmark(benchmark):
  # TODO: Parse. Compute load imbalance.

if __name__ == "__main__":
  ranks = parse_csv("C:/Users/demir/Desktop/astro_1024_n_16_l_gllma_d_1.0_s_4,4,4.h5.benchmark.csv")
  pprint.pprint(ranks)
  #parse_scaling_benchmarks([
  #  "C:/Users/demir/Desktop/astro_1024_n_16_l_gllma_d_1.0_s_4,4,4.h5.benchmark.csv",
  #  "C:/Users/demir/Desktop/astro_1024_n_32_l_gllma_d_1.0_s_4,4,4.h5.benchmark.csv",
  #  "C:/Users/demir/Desktop/astro_1024_n_64_l_gllma_d_1.0_s_4,4,4.h5.benchmark.csv",
  #  "C:/Users/demir/Desktop/astro_1024_n_128_l_gllma_d_1.0_s_4,4,4.h5.benchmark.csv"
  #])
  #parse_scaling_benchmarks([
  #  "C:/Users/demir/Desktop/fusion_1024_n_16_l_gllma_d_1.0_s_4,4,4.h5.benchmark.csv",
  #  "C:/Users/demir/Desktop/fusion_1024_n_32_l_gllma_d_1.0_s_4,4,4.h5.benchmark.csv",
  #  "C:/Users/demir/Desktop/fusion_1024_n_64_l_gllma_d_1.0_s_4,4,4.h5.benchmark.csv",
  #  "C:/Users/demir/Desktop/fusion_1024_n_128_l_gllma_d_1.0_s_4,4,4.h5.benchmark.csv"
  #])
  #parse_load_balancing_benchmark([
  #  "C:/Users/demir/Desktop/astro_1024_n_128_l_gllma_d_1.0_s_4,4,4.h5.benchmark.csv"
  #])