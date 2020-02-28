import csv
import ntpath
import os

class auto_resize_list(list):
  def __setitem__(self, index, value):
    if index >= len(self):
      self.extend([None] * (index + 1 - len(self)))
    list.__setitem__(self, index, value)

"""Format:
{
  maximum_time: ...
  ranks       : 
  [ # Array ordered by ranks.
    {
      total_time: ...
      rounds    :
      [ # Array ordered by rounds.
        {load: ..., time: ..., load_balancing_time: ..., advection_time: ..., communication_time: ...},
        {load: ..., time: ..., load_balancing_time: ..., advection_time: ..., communication_time: ...},
        {load: ..., time: ..., load_balancing_time: ..., advection_time: ..., communication_time: ...},
        ...
      ]
    },
    ...
  ]
}"""
def parse_benchmark(filepath):
  benchmark = {"maximum_time": 0.0, "ranks": auto_resize_list()}

  with open(filepath, mode='r') as csv_file:
    csv_reader = csv.DictReader(csv_file)
    
    for row in csv_reader:
      rank  = int  (row["rank"])
      name  =       row["name"]
      value = float(row["iteration 0"])
      
      if rank >= len(benchmark["ranks"]): # Order dependent.
        benchmark["ranks"][rank] = {"rounds": auto_resize_list()} 

      if name == "total_time":
        benchmark["ranks"][rank]["total_time"] = value
        benchmark["maximum_time"] = max(benchmark["maximum_time"], value)
      else:
        split_name  = name.split(".")
        round_index = int(split_name[1])
        value_type  =     split_name[2]
      
        if round_index >= len(benchmark["ranks"][rank]["rounds"]): # Order dependent.
          benchmark["ranks"][rank]["rounds"][round_index] = {} 
      
        benchmark["ranks"][rank]["rounds"][round_index][value_type] = value
  
  return benchmark

"""Format:
{
  "nodes"   : [16   , 32   , 64   , 128  , ...]
  "times"   : [999.9, 666.6, 333.3, 111.1, ...]
  "speedups": [1    , 1.5  , 3    , 9    , ...]
}
"""
def parse_scaling_benchmarks(filepaths):
  scaling = {"nodes": [], "times": [], "speedups": []}

  for filepath in filepaths:
    time = parse_benchmark(filepath)["maximum_time"]
  
    scaling["nodes"].append(int(ntpath.basename(filepath).split("_")[3]))
    scaling["times"].append(time)

    if (len(scaling["times"]) > 0):
      scaling["speedups"].append(scaling["times"][0] / time)
    else:
      scaling["speedups"].append(1)
  
  return scaling

"""Format:
[ # Array ordered by rounds.
  {
    "bound_time"    : ...,
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
def parse_load_balancing_benchmark(filepath):
  load_balancing = auto_resize_list()
  
  benchmark = parse_benchmark(filepath)

  for rank_index, rank in enumerate(benchmark["ranks"]):
    for round_index, round in enumerate(rank["rounds"]):

      if round_index >= len(load_balancing): # Order dependent.
        load_balancing[round_index] = {"bound_time": 0.0, "load_imbalance": 0.0, "times": auto_resize_list()} 

      if rank_index >= len(load_balancing[round_index]["times"]): # Order dependent.
        load_balancing[round_index]["times"][rank_index] = {
          "load_balancing_time": round["load_balancing_time"],
          "advection_time"     : round["advection_time"     ],
          "communication_time" : round["communication_time" ]
        }

      load_balancing[round_index]["bound_time"] = max(load_balancing[round_index]["bound_time"], round["time"])
  
  for round_index, round in enumerate(load_balancing):
    total   = 0.0
    maximum = 0.0
    for rank_index, rank in enumerate(round["times"]):
      load    = benchmark["ranks"][rank_index]["rounds"][round_index]["load"]
      total  += load
      maximum = max(maximum, load)
    load_balancing[round_index]["load_imbalance"] = maximum / (total / len(load_balancing))
  return load_balancing

"""Format:
{
  "names"     : ["none" , "const", "lma", "gllma"],
  "colors"    : ["black", "blue" , "red", "green"],
  "benchmarks": # 16 measurements for 4 algorithms and 4 node counts.
  [
    scaling_benchmark(),
    scaling_benchmark(),
    scaling_benchmark(),
    scaling_benchmark()
  ]
}
"""
def create_composite_scaling_benchmarks(names, colors, benchmark_filepaths):
  composite_benchmark = {"names": names, "colors": colors, "benchmarks": []}
  for filepaths in benchmark_filepaths:
    composite_benchmark["benchmarks"].append(parse_scaling_benchmarks(filepaths)) # Order has to be the same as names.
  return composite_benchmark