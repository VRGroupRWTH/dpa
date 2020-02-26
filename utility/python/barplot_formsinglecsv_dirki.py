import pandas as pd
import numpy as np
import os
import re
import scipy
from bokeh.io import show, output_file, export_png
from bokeh.models import ColumnDataSource, Span
from bokeh.plotting import figure
from bokeh.sampledata.sprint import sprint
from bokeh.io import export_svgs
import csv

# parse function
def parse_benchmark(filepath):
  benchmark = []
  with open(filepath, mode='r') as csv_file:
    csv_reader = csv.DictReader(csv_file)
    for row in csv_reader:
      benchmark.append(row)
  return benchmark

#parsing the data into individual arrays containting all information
#this could be heavily optimized if there is assurance that the order is preserved in all cases
# like structured data vs the unstructured assuption that is made here
rawbm = parse_benchmark("test.csv")
total_times = []
dtf = pd.DataFrame(rawbm)
loaddata = np.array([[int(row["rank"]),int(row["name"].split('.')[1]),float(row["iteration 0"])] for row in rawbm if row["name"].find("load") > -1])
timedata = np.array([[int(row["rank"]),int(row["name"].split('.')[1]),float(row["iteration 0"])] for row in rawbm if (row["name"].find("round") > -1 and row["name"].find("time") > -1)])
totaldata = np.array([[int(row["rank"]),float(row["iteration 0"])] for row in rawbm if row["name"].find("total") > -1])

#calculating the maximum for each data dimension (meta)
maxrank = int(np.max(loaddata[:,0]))
maxround = int(np.max(loaddata[:,1]))

# initializing data matrix
matrixdata = np.zeros((maxrank+1,maxround+1,2))

# sorting unstructured data and assume structured from now in except if something is missing
timedata = timedata[np.argsort(timedata[:,1])]
timedata = timedata[np.argsort(timedata[:,0],kind="mergesort")]


for i in range(len(timedata)) :
  matrixdata[int(timedata[i][0])][int(timedata[i][1])][0] = timedata[i][2]
  matrixdata[int(timedata[i][0])][int(timedata[i][1])][1] = loaddata[i][2]
#print(matrixdata)

#setting up the plot
plot = figure(title="", x_axis_label='Time', y_axis_label='Rank',\
  sizing_mode="stretch_width", x_range=(0, 300))#, plot_width=1920)
plot.output_backend="svg"

plot.yaxis.major_tick_line_color      = None
plot.yaxis.minor_tick_line_color      = None
plot.yaxis.major_label_text_font_size = '0pt'
plot.toolbar.logo                     = None
plot.toolbar_location                 = None

# calculating the offset because we are calculating synchronous rounds
offset = 0.0
maxroundtimes = np.zeros(maxround+1)
for i in range(maxround+1) :
  roundmaximum = np.max(matrixdata[:,i,0])
  maxroundtimes[i] = offset + roundmaximum
  plot.add_layout(Span(location=maxroundtimes[i],dimension='height',line_color='black',line_dash='dashed',line_width=1))
  offset += roundmaximum

#setting up the single bars for hte gantt plot
bars = [[] for i in range(maxround+1)]
for ranknumber,rank in enumerate(matrixdata) :
  for rn,round in enumerate(rank) :
    offset = 0 if rn <= 0 else maxroundtimes[rn-1]
    bars[rn].append([ranknumber,offset,offset+round[0]])
bars = np.array(bars)


#print(bars)
#We plot the round bars now
colorst = ["#AA0000","#00AA00","#0000AA"]
for k in range(maxround+1) :
  plot.hbar(y=bars[k][:,0],height=0.9, left=bars[k][:,1], right=bars[k][:,2], color=colorst[k])
export_png(plot, filename="plot.png", height=1920,width=1080)
export_svgs(plot, filename="plot.svg")
#show(plot)
print("done.")
