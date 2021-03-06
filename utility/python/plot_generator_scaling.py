from bokeh.io import export_png
from bokeh.layouts import gridplot
from bokeh.plotting import figure, output_file, show
from bokeh.models import Range1d
import pprint

import benchmark_parser

maximum_y = 0.0

def create_strong_scaling_filepaths(template, algorithms, node_counts):
  algorithm_filepaths = []
  for algorithm in algorithms:
    node_count_filepaths = []
    for node_count in node_counts:
      node_count_filepaths.append(template.replace("$1", algorithm).replace("$2", str(node_count)))
    algorithm_filepaths.append(node_count_filepaths)
  return algorithm_filepaths

def create_weak_scaling_filepaths  (template, algorithms, node_counts, strides):
  algorithm_filepaths = []
  for algorithm in algorithms:
    node_count_filepaths = []
    for index, node_count in enumerate(node_counts):
      node_count_filepaths.append(template.replace("$1", algorithm).replace("$2", str(node_count)).replace("$3", strides[index]))
    algorithm_filepaths.append(node_count_filepaths)
  return algorithm_filepaths

def generate(name, composite_benchmark):
  global maximum_y
  plot = figure(title=name, x_axis_label='Processes', y_axis_label='Time', sizing_mode="fixed", x_range=list(map(str, composite_benchmark["benchmarks"][0]["nodes"])), plot_width=1024, plot_height=1024)
  for index, benchmark in enumerate(composite_benchmark["benchmarks"]):
    plot.line(list(map(str, benchmark["nodes"])), benchmark["times"], line_width=1, legend_label=composite_benchmark["names"][index], line_color=composite_benchmark["colors"][index])
    maximum_y = max(maximum_y, max(benchmark["times"]))
  plot.legend.location = "bottom_right"
  plot.legend.background_fill_alpha = 0.0
  return plot

def generate_scaling_figure():
  global maximum_y
  algorithms = ["none" , "const", "lma"  , "gllma"]
  colors     = ["black", "red"  , "green", "blue" ]
  nodes      = [16, 32, 64, 128]
  strides    = ["8,8,8", "8,8,4", "8,4,4", "4,4,4"]
  maximum_y  = 0.0

  astro_strong = generate(
    "Strong Scaling - Astrophysics", 
    benchmark_parser.create_composite_scaling_benchmarks(
      algorithms, 
      colors,
      create_strong_scaling_filepaths("../benchmarks/strong_scaling/astro_1024_n_$2_l_$1_d_1.0_s_4,4,4.h5.benchmark.csv"   , algorithms, nodes)))
  fishtank_strong = generate(
    "Strong Scaling - Thermal Hydraulics", 
    benchmark_parser.create_composite_scaling_benchmarks(
      algorithms, 
      colors,
      create_strong_scaling_filepaths("../benchmarks/strong_scaling/fishtank_1024_n_$2_l_$1_d_1.0_s_4,4,4.h5.benchmark.csv", algorithms, nodes)))
  fusion_strong = generate(
    "Strong Scaling - Nuclear Fusion", 
    benchmark_parser.create_composite_scaling_benchmarks(
      algorithms, 
      colors,
      create_strong_scaling_filepaths("../benchmarks/strong_scaling/fusion_1024_n_$2_l_$1_d_1.0_s_4,4,4.h5.benchmark.csv"  , algorithms, nodes)))

  astro_weak = generate(
    "Weak Scaling - Astrophysics", 
    benchmark_parser.create_composite_scaling_benchmarks(
      algorithms, 
      colors,
      create_weak_scaling_filepaths("../benchmarks/weak_scaling/astro_1024_n_$2_l_$1_d_1.0_s_$3.h5.benchmark.csv"          , algorithms, nodes, strides)))
  fishtank_weak = generate(
    "Weak Scaling - Thermal Hydraulics", 
    benchmark_parser.create_composite_scaling_benchmarks(
      algorithms, 
      colors,
      create_weak_scaling_filepaths("../benchmarks/weak_scaling/fishtank_1024_n_$2_l_$1_d_1.0_s_$3.h5.benchmark.csv"       , algorithms, nodes, strides)))
  fusion_weak = generate(
    "Weak Scaling - Nuclear Fusion", 
    benchmark_parser.create_composite_scaling_benchmarks(
      algorithms, 
      colors,
      create_weak_scaling_filepaths("../benchmarks/weak_scaling/fusion_1024_n_$2_l_$1_d_1.0_s_$3.h5.benchmark.csv"         , algorithms, nodes, strides)))

  grid = [
    [astro_strong, fishtank_strong, fusion_strong], 
    [astro_weak  , fishtank_weak  , fusion_weak  ]]

  for row in grid:
    for entry in row:
      entry.y_range=Range1d(0, maximum_y)

  return gridplot(grid, toolbar_location=None)

def generate_parameter_space_figure():
  global maximum_y

  algorithms = ["none" , "const", "lma"  , "gllma"]
  colors     = ["black", "red"  , "green", "blue" ]
  nodes      = [16, 32, 64, 128]
  strides    = ["8,8,8", "8,8,4", "8,4,4", "4,4,4"]
  maximum_y  = 0.0

  data_complexity_astro = generate(
    "Data Complexity - Astrophysics", 
    benchmark_parser.create_composite_scaling_benchmarks(
      algorithms, 
      colors,
      create_strong_scaling_filepaths("../benchmarks/parameter_space/dataset_complexity/astro_1024_n_$2_l_$1_d_1.0_s_4,4,4.h5.benchmark.csv"   , algorithms, nodes)))
  data_complexity_fishtank = generate(
    "Data Complexity - Thermal Hydraulics", 
    benchmark_parser.create_composite_scaling_benchmarks(
      algorithms, 
      colors,
      create_strong_scaling_filepaths("../benchmarks/parameter_space/dataset_complexity/fishtank_1024_n_$2_l_$1_d_1.0_s_4,4,4.h5.benchmark.csv", algorithms, nodes)))
  data_complexity_fusion = generate(
    "Data Complexity - Nuclear Fusion", 
    benchmark_parser.create_composite_scaling_benchmarks(
      algorithms, 
      colors,
      create_strong_scaling_filepaths("../benchmarks/parameter_space/dataset_complexity/fusion_1024_n_$2_l_$1_d_1.0_s_4,4,4.h5.benchmark.csv"  , algorithms, nodes)))
  
  data_size_1024 = generate(
    "Data Size - 1024^3", 
    benchmark_parser.create_composite_scaling_benchmarks(
      algorithms, 
      colors,
      create_strong_scaling_filepaths("../benchmarks/parameter_space/dataset_size/astro_1024_n_$2_l_$1_d_1.0_s_4,4,4.h5.benchmark.csv"   , algorithms, nodes)))
  data_size_1536 = generate(
    "Data Size - 1536^3", 
    benchmark_parser.create_composite_scaling_benchmarks(
      algorithms, 
      colors,
      create_strong_scaling_filepaths("../benchmarks/parameter_space/dataset_size/astro_1536_n_$2_l_$1_d_1.0_s_6,6,6.h5.benchmark.csv", algorithms, nodes)))
  data_size_2048 = generate(
    "Data Size - 2048^3", 
    benchmark_parser.create_composite_scaling_benchmarks(
      algorithms, 
      colors,
      create_strong_scaling_filepaths("../benchmarks/parameter_space/dataset_size/astro_2048_n_$2_l_$1_d_1.0_s_8,8,8.h5.benchmark.csv", algorithms, nodes)))

  seed_distribution_1 = generate(
    "Seed Distribution - 1.0", 
    benchmark_parser.create_composite_scaling_benchmarks(
      algorithms, 
      colors,
      create_strong_scaling_filepaths("../benchmarks/parameter_space/seed_distribution/astro_1024_n_$2_l_$1_d_1.0_s_4,4,4.h5.benchmark.csv", algorithms, nodes)))
  seed_distribution_05 = generate(
    "Seed Distribution - 0.5", 
    benchmark_parser.create_composite_scaling_benchmarks(
      algorithms, 
      colors,
      create_strong_scaling_filepaths("../benchmarks/parameter_space/seed_distribution/astro_1024_n_$2_l_$1_d_0.5_s_2,2,2.h5.benchmark.csv", algorithms, nodes)))
  seed_distribution_025 = generate(
    "Seed Distribution - 0.25", 
    benchmark_parser.create_composite_scaling_benchmarks(
      algorithms, 
      colors,
      create_strong_scaling_filepaths("../benchmarks/parameter_space/seed_distribution/astro_1024_n_$2_l_$1_d_0.25_s_1,1,1.h5.benchmark.csv", algorithms, nodes)))

  seed_set_884 = generate(
    "Seed Set - [8, 8, 4]", 
    benchmark_parser.create_composite_scaling_benchmarks(
      algorithms, 
      colors,
      create_strong_scaling_filepaths("../benchmarks/parameter_space/seed_stride/astro_1024_n_$2_l_$1_d_1.0_s_8,8,4.h5.benchmark.csv", algorithms, nodes)))
  seed_set_844 = generate(
    "Seed Set - [8, 4, 4]", 
    benchmark_parser.create_composite_scaling_benchmarks(
      algorithms, 
      colors,
      create_strong_scaling_filepaths("../benchmarks/parameter_space/seed_stride/astro_1024_n_$2_l_$1_d_1.0_s_8,4,4.h5.benchmark.csv", algorithms, nodes)))
  seed_set_444 = generate(
    "Seed Set - [4, 4, 4]", 
    benchmark_parser.create_composite_scaling_benchmarks(
      algorithms, 
      colors,
      create_strong_scaling_filepaths("../benchmarks/parameter_space/seed_stride/astro_1024_n_$2_l_$1_d_1.0_s_4,4,4.h5.benchmark.csv", algorithms, nodes)))
  
  grid = [
   [data_complexity_astro, data_complexity_fishtank, data_complexity_fusion],
   [data_size_1024       , data_size_1536          , data_size_2048        ],  # Scales stride.
   [seed_distribution_1  , seed_distribution_05    , seed_distribution_025 ],  # Scales stride.
   [seed_set_884         , seed_set_844            , seed_set_444          ]]

  for row in grid:
    for entry in row:
      entry.y_range=Range1d(0, maximum_y)

  return gridplot(grid, toolbar_location=None)

if __name__ == "__main__":
  plot = generate_scaling_figure()
  output_file("scaling.html")
  export_png (plot, filename="scaling.png")
  show       (plot)
  
  plot = generate_parameter_space_figure()
  output_file("parameter_space.html")
  export_png (plot, filename="parameter_space.png")
  show       (plot)
