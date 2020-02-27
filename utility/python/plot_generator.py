from bokeh.io import export_png
from bokeh.layouts import row
from bokeh.plotting import figure, output_file, show
from bokeh.models   import Span
import pprint

import benchmark_parser

def generate(name, data):
  pprint.pprint(data)
  plot = figure(title=name, x_axis_label='Nodes', y_axis_label='Time', sizing_mode="scale_height")
  plot.line(data["nodes"], data["times"], line_width=1, line_color="black")
  return plot

if __name__ == "__main__":
  plot1 = generate("astro_1024", benchmark_parser.parse_scaling_benchmarks([
    "C:/Users/demir/Desktop/astro_1024_n_16_l_gllma_d_1.0_s_4,4,4.h5.benchmark.csv",
    "C:/Users/demir/Desktop/astro_1024_n_32_l_gllma_d_1.0_s_4,4,4.h5.benchmark.csv",
    "C:/Users/demir/Desktop/astro_1024_n_64_l_gllma_d_1.0_s_4,4,4.h5.benchmark.csv",
    "C:/Users/demir/Desktop/astro_1024_n_128_l_gllma_d_1.0_s_4,4,4.h5.benchmark.csv"
  ]))

  plot2 = generate("fusion_1024", benchmark_parser.parse_scaling_benchmarks([
    "C:/Users/demir/Desktop/fusion_1024_n_16_l_gllma_d_1.0_s_4,4,4.h5.benchmark.csv",
    "C:/Users/demir/Desktop/fusion_1024_n_32_l_gllma_d_1.0_s_4,4,4.h5.benchmark.csv",
    "C:/Users/demir/Desktop/fusion_1024_n_64_l_gllma_d_1.0_s_4,4,4.h5.benchmark.csv",
    "C:/Users/demir/Desktop/fusion_1024_n_128_l_gllma_d_1.0_s_4,4,4.h5.benchmark.csv"
  ]))

plot = row(plot1, plot2)
output_file("scaling.html")
export_png (plot, filename="scaling.png")
show       (plot)
