from bokeh.io import export_png, export_svgs
from bokeh.layouts import row, column, gridplot
from bokeh.plotting import figure, output_file, show
from bokeh.models   import Span, LinearAxis, Range1d
import pprint
import numpy as np
import time

import benchmark_parser

def generate(name, data):
  #pprint.pprint(data)
  plot = figure(title=name, x_axis_label='Nodes', y_axis_label='Time', sizing_mode="scale_height")
  plot.line(data["nodes"], data["times"], line_width=1, line_color="black")
  return plot

if __name__ == "__main__":

  #setting up folder information

  fold = "./boenschmarks_ali/"
  sscale = fold+"strong_scaling/"
  wscale = fold+"weak_scaling/"
  paramspace = fold+"parameter_space/"
  lbalance = fold+"load_balancing/"

  """
  fl1 = benchmark_parser.folderlist(sscale,["fusion_1024","const"])
  fl2 = benchmark_parser.folderlist(sscale,["fusion_1024","_gllma"])
  fl3 = benchmark_parser.folderlist(sscale,["fusion_1024","_lma"])
  fl4 = benchmark_parser.folderlist(sscale,["fusion_1024","none"])
  il1 = benchmark_parser.folderlist(sscale,["fishtank_1024","const"])
  il2 = benchmark_parser.folderlist(sscale,["fishtank_1024","_gllma"])
  il3 = benchmark_parser.folderlist(sscale,["fishtank_1024","_lma"])
  il4 = benchmark_parser.folderlist(sscale,["fishtank_1024","none"])
  al1 = benchmark_parser.folderlist(sscale,["astro_1024","const"])
  al2 = benchmark_parser.folderlist(sscale,["astro_1024","_gllma"])
  al3 = benchmark_parser.folderlist(sscale,["astro_1024","_lma"])
  al4 = benchmark_parser.folderlist(sscale,["astro_1024","none"])
  ap1 = benchmark_parser.parse_scaling_benchmarks(al1)
  ap2 = benchmark_parser.parse_scaling_benchmarks(al2)
  ap3 = benchmark_parser.parse_scaling_benchmarks(al3)
  ap4 = benchmark_parser.parse_scaling_benchmarks(al4)
  fp1 = benchmark_parser.parse_scaling_benchmarks(fl1)
  fp2 = benchmark_parser.parse_scaling_benchmarks(fl2)
  fp3 = benchmark_parser.parse_scaling_benchmarks(fl3)
  fp4 = benchmark_parser.parse_scaling_benchmarks(fl4)
  ip1 = benchmark_parser.parse_scaling_benchmarks(il1)
  ip2 = benchmark_parser.parse_scaling_benchmarks(il2)
  ip3 = benchmark_parser.parse_scaling_benchmarks(il3)
  ip4 = benchmark_parser.parse_scaling_benchmarks(il4)
  pla1 = generate("astro_1024 const",     ap1)
  pla2 = generate("astro_1024 gllma",     ap2)
  pla3 = generate("astro_1024 lma",       ap3)
  pla4 = generate("astro_1024 none",      ap4)
  pli1 = generate("fishtank_1024 const",  ip1)
  pli2 = generate("fishtank_1024 gllma",  ip2)
  pli3 = generate("fishtank_1024 lma",    ip3)
  pli4 = generate("fishtank_1024 none",   ip4)
  plf1 = generate("fusion_1024 const",    fp1)
  plf2 = generate("fusion_1024 gllma",    fp2)
  plf3 = generate("fusion_1024 lma",      fp3)
  plf4 = generate("fusion_1024 none",     fp4)
  

  plot = gridplot([[pla1, pla2, pla3, pla4],
                   [plf1, plf2, plf3, plf4],
                   [pli1, pli2, pli3, pli4]])

  output_file("scaling.html")
  export_png(plot, filename="scaling.png")
  #show(plot)
  """
  #plot1 = generate("astro_2014",benchmark_parser.parse_scaling_benchmarks(\
  # benchmark_parser.folderlist(sscale,"astro_1024")))
  #plot2 = generate("fusion_1024", benchmark_parser.parse_scaling_benchmarks(\
  #  benchmark_parser.folderlist(sscale,"fusion_1024")))
  #plot3 = generate("fishtank_1024", benchmark_parser.parse_scaling_benchmarks(\
  #  benchmark_parser.folderlist(sscale,"fishtank_1024")))

  #plot = row(plot1, plot2,plot3)
  #output_file("scaling.html")
  #export_png (plot, filename="scaling.png")
  #show       (plot)


  
  #setting up the plot
  balanceplot = figure(title="Loadbalancing for Astro dataset", \
                x_axis_label='Time', y_axis_label='Process', \
    sizing_mode="stretch_width")#, x_range=(0, 300))#, plot_width=1920)
  balanceplot.yaxis.major_tick_line_color      = None
  balanceplot.yaxis.minor_tick_line_color      = None
  balanceplot.yaxis.major_label_text_font_size = '12pt'
  balanceplot.toolbar.logo                     = None
  balanceplot.toolbar_location                 = None
  balanceplot.background_fill_color            = "#DADADA"
 

  # getting data
  balancedata = [benchmark_parser.parse_load_balancing_benchmark(f) \
    for f in benchmark_parser.folderlist(lbalance,["astro","gllma"])]
  
  test = balancedata[0]
  spanlines = []

  # generating vlines / span lines
  bound = 0.0
  last_bound = 0.0
  rounds = []
  imbalance = []
  for round in test :
    imbalance.append(round["load_imbalance"])
    rounds.append(bound)
    bound = bound + round["bound_time"]
    boundspan = Span(location=bound, dimension='height', line_color='black', \
      line_dash='dashed')
    balanceplot.add_layout(boundspan)
    spanlines.append(boundspan)
    rank = 0
    for dat in round["times"] :
      #henlo
      offset = last_bound
      balanceplot.hbar(y=rank, left=offset, height=0.9,
                       right = offset+dat["load_balancing_time"], color="#FF7777")
      offset = offset + dat["load_balancing_time"]
      balanceplot.hbar(y=rank, left=offset, height=0.9,
                       right = offset+dat["advection_time"], color="#77FF77")
      offset = offset + dat["advection_time"]
      balanceplot.hbar(y=rank, left=offset, height=0.9,
                       right = offset+dat["communication_time"], color="#7777FF")
      rank = rank + 1
    last_bound = bound
  balanceplot.add_layout(LinearAxis(y_range_name="imbalance", axis_label="Loas Imbalance"), 'right')
  balanceplot.extra_y_ranges={"imbalance":Range1d(start=0,end=np.max(imbalance))}
  balanceplot.line(x=rounds,y=imbalance,color="black",y_range_name="imbalance")
  
  uniquefname = str(int(round(time.time() * 1000)))
  export_png(balanceplot, filename="loadbalance_"+uniquefname+".png")
  show(balanceplot)
  print("done")
  
  