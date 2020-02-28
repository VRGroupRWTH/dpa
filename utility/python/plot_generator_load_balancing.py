from bokeh.io import export_png
from bokeh.layouts import gridplot
from bokeh.plotting import figure, output_file, show
from bokeh.models   import Span, LinearAxis, Range1d
import numpy as np

import benchmark_parser

def generate(name, load_balancing_benchmark):
  plot = figure(title=name, x_axis_label='Time', y_axis_label='Processes', sizing_mode="stretch_width")
  plot.yaxis.major_tick_line_color      = None
  plot.yaxis.minor_tick_line_color      = None
  plot.yaxis.major_label_text_font_size = '12pt'
  plot.toolbar.logo                     = None
  plot.toolbar_location                 = None

  load_imbalances = []
  time_offsets    = []
  time_offset     = 0.0
  for round in load_balancing_benchmark:
    load_imbalances.append(round["load_imbalance"])
    time_offsets   .append(time_offset)

    for index, rank in enumerate(round["times"]):
      load_balancing_offset = time_offset           + rank["load_balancing_time"]
      advection_offset      = load_balancing_offset + rank["advection_time"     ]
      communication_offset  = advection_offset      + rank["communication_time" ]

      plot.hbar(y=index, height=0.7, left=time_offset          , right=load_balancing_offset, color="#66b3ff")
      plot.hbar(y=index, height=0.7, left=load_balancing_offset, right=advection_offset     , color="#66ffb3")
      plot.hbar(y=index, height=0.7, left=advection_offset     , right=communication_offset , color="#ffb366")

    plot.line([time_offset, time_offset], [-0.5, len(round["times"]) -0.5], color="black")

    time_offset += round["bound_time"]

  # Plot load imbalance factor on the right scale.
  lif = "LIF"
  plot.add_layout     (LinearAxis(y_range_name=lif, axis_label=lif), 'right')
  plot.extra_y_ranges={lif: Range1d(start=0, end=np.max(load_imbalances))}
  plot.line           (x=time_offsets, y=load_imbalances, color="black", y_range_name=lif)
  
  return plot
  
def generate_load_balancing_figure():
  template      = "../benchmarks/load_balancing/$2_1024_n_64_l_$1_d_0.5_s_2,2,2.h5.benchmark.csv"
  # algorithms    = ["none", "const", "lma", "gllma"]
  # datasets      = ["astro", "fishtank" , "fusion" ]
  algorithms = ["const"]
  datasets   = ["astro"]
  algorithm_names = ["None", "Constant", "LMA", "GLLMA"]
  dataset_names   = ["Astrophysics", "Thermal Hydraulics" , "Nuclear Fusion"]
  
  grid = []
  for algorithm in algorithms:
    row = []
    for index, dataset in enumerate(datasets):
      row.append(generate(dataset_names[index] + " - " + algorithm_names[index], benchmark_parser.parse_load_balancing_benchmark(template.replace("$1", algorithm).replace("$2", dataset))))
    grid.append(row)

  return gridplot(grid, toolbar_location=None)

if __name__ == "__main__":
  plot = generate_load_balancing_figure()
  output_file("load_balancing.html")
  export_png (plot, filename="load_balancing.png")
  show       (plot)
