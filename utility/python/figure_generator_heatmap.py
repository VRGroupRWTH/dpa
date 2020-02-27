from bokeh.io import export_png
from bokeh.layouts import row
from bokeh.models import Arrow, ColorBar, ColumnDataSource, LabelSet, LinearColorMapper, VeeHead
from bokeh.plotting import figure, output_file, show
from colour import Color
from PIL import Image
from random import randint
import copy
import numpy

def generate_random    (width, height, maximum_load):
    data  = {"x": [], "y": [], "value": []}
    for x in range(0, width):
        for y in range(0, height):
            data["x"    ].append(x)
            data["y"    ].append(y)
            data["value"].append(randint(0, maximum_load))
    return data
def generate_from_image(width, height, filepath):
    data  = {"x": [], "y": [], "value": []}
    image = Image.open(filepath).convert('LA').resize((width, height))
    for x in range(0, width):
        for y in range(0, height):
            data["x"    ].append(x)
            data["y"    ].append(y)
            data["value"].append(image.getpixel((image.size[0] - 1 - x, image.size[1] - 1 - y))[0])
    return data
def generate_fixed     ():
    data  = {"x": [], "y": [], "value": [
        400, 300, 150, 350, 200, 
        50 , 900, 750, 450, 550, 
        250, 150, 300, 50 , 300, 
        350, 650, 100, 500, 400, 
        100, 400, 200, 250, 800]}
    for x in range(0, 5):
        for y in range(0, 5):
            data["x"    ].append(x)
            data["y"    ].append(y)
    return data
def generate_example   ():
    data  = {"x": [], "y": [], "value": [
        0   , 0   , 1000, 0   , 0   , 
        0   , 1000, 1000, 1000, 0   , 
        1000, 1000, 0   , 1000, 1000, 
        0   , 1000, 1000, 1000, 0   , 
        0   , 0   , 1000, 0   , 0   ]}
    for x in range(0, 5):
        for y in range(0, 5):
            data["x"    ].append(x)
            data["y"    ].append(y)
    return data

def load_balance_orig  (data, width, height):
    balanced_data       = copy.deepcopy(data)
    incoming_arrow_data = {"x_start": [], "y_start": [], "x_end": [], "y_end": []}
    outgoing_arrow_data = {"x_start": [], "y_start": [], "x_end": [], "y_end": []}

    dimensions          = 2
    alpha               = 1 - 2 / (dimensions + 1)

    for x in range(0, width):
        for y in range(0, height):
            index            = numpy.ravel_multi_index((x, y), (width, height))
            neighbor_indices = [-1, -1, -1, -1]

            if (x > 0)        : neighbor_indices[0] = numpy.ravel_multi_index((x - 1, y    ), (width, height))
            if (x < width - 1): neighbor_indices[1] = numpy.ravel_multi_index((x + 1, y    ), (width, height))
            if (y > 0)        : neighbor_indices[2] = numpy.ravel_multi_index((x    , y - 1), (width, height))
            if (y < width - 1): neighbor_indices[3] = numpy.ravel_multi_index((x    , y + 1), (width, height))
            
            workload       = data["value"][index]
            
            west_workload  = data["value"][neighbor_indices[0]] if neighbor_indices[0] != -1 else -1
            east_workload  = data["value"][neighbor_indices[1]] if neighbor_indices[1] != -1 else -1
            south_workload = data["value"][neighbor_indices[2]] if neighbor_indices[2] != -1 else -1
            north_workload = data["value"][neighbor_indices[3]] if neighbor_indices[3] != -1 else -1
    
            west_outgoing  = 0
            east_outgoing  = 0
            south_outgoing = 0
            north_outgoing = 0

            if (west_workload  != - 1 and west_workload < workload) : 
                west_outgoing  = int(alpha * (workload - west_workload ))
            if (east_workload  != - 1 and east_workload < workload) : 
                east_outgoing  = int(alpha * (workload - east_workload ))
            if (south_workload != -1 and south_workload < workload) : 
                south_outgoing = int(alpha * (workload - south_workload))
            if (north_workload != -1 and north_workload < workload) : 
                north_outgoing = int(alpha * (workload - north_workload))
       
            balanced_data["value"][index]               -= (west_outgoing + east_outgoing + south_outgoing + north_outgoing)
            balanced_data["value"][neighbor_indices[0]] +=  west_outgoing 
            balanced_data["value"][neighbor_indices[1]] +=  east_outgoing 
            balanced_data["value"][neighbor_indices[2]] +=  south_outgoing
            balanced_data["value"][neighbor_indices[3]] +=  north_outgoing

            if west_outgoing  > 0:
                outgoing_arrow_data["x_start"].append(x)
                outgoing_arrow_data["y_start"].append(y)
                outgoing_arrow_data["x_end"  ].append(x - 0.65)
                outgoing_arrow_data["y_end"  ].append(y)
            if east_outgoing  > 0:
                outgoing_arrow_data["x_start"].append(x)
                outgoing_arrow_data["y_start"].append(y)
                outgoing_arrow_data["x_end"  ].append(x + 0.65)
                outgoing_arrow_data["y_end"  ].append(y)
            if south_outgoing > 0:
                outgoing_arrow_data["x_start"].append(x)
                outgoing_arrow_data["y_start"].append(y)
                outgoing_arrow_data["x_end"  ].append(x)
                outgoing_arrow_data["y_end"  ].append(y - 0.65)
            if north_outgoing > 0:
                outgoing_arrow_data["x_start"].append(x)
                outgoing_arrow_data["y_start"].append(y)
                outgoing_arrow_data["x_end"  ].append(x)
                outgoing_arrow_data["y_end"  ].append(y + 0.65)

    result = {}
    result["data"               ] = balanced_data
    result["incoming_arrow_data"] = incoming_arrow_data
    result["outgoing_arrow_data"] = outgoing_arrow_data

    return result
def load_balance_lma   (data, width, height):
    balanced_data       = copy.deepcopy(data)
    incoming_arrow_data = {"x_start": [], "y_start": [], "x_end": [], "y_end": []}
    outgoing_arrow_data = {"x_start": [], "y_start": [], "x_end": [], "y_end": []}
    
    for x in range(0, width):
        for y in range(0, height):
            index            = numpy.ravel_multi_index((x, y), (width, height))
            neighbor_indices = [-1, -1, -1, -1]
            
            if (x > 0)        : neighbor_indices[0] = numpy.ravel_multi_index((x - 1, y    ), (width, height))
            if (x < width - 1): neighbor_indices[1] = numpy.ravel_multi_index((x + 1, y    ), (width, height))
            if (y > 0)        : neighbor_indices[2] = numpy.ravel_multi_index((x    , y - 1), (width, height))
            if (y < width - 1): neighbor_indices[3] = numpy.ravel_multi_index((x    , y + 1), (width, height))
            
            workload       = data["value"][index]

            west_workload  = data["value"][neighbor_indices[0]] if neighbor_indices[0] != -1 else -1
            east_workload  = data["value"][neighbor_indices[1]] if neighbor_indices[1] != -1 else -1
            south_workload = data["value"][neighbor_indices[2]] if neighbor_indices[2] != -1 else -1
            north_workload = data["value"][neighbor_indices[3]] if neighbor_indices[3] != -1 else -1
            
            west_outgoing  = 0
            east_outgoing  = 0
            south_outgoing = 0
            north_outgoing = 0

            # Take the mean of neighbors with workload less than this process.
            local_sum   = workload
            local_count = 1
            if (west_workload  != -1 and west_workload  < workload):
                local_sum   += west_workload
                local_count += 1
            if (east_workload  != -1 and east_workload  < workload):
                local_sum   += east_workload
                local_count += 1
            if (south_workload != -1 and south_workload < workload):
                local_sum   += south_workload
                local_count += 1
            if (north_workload != -1 and north_workload < workload):
                local_sum   += north_workload
                local_count += 1
            local_mean  = int(local_sum / local_count)

            # Take the mean of neighbors with workload less than the mean until all remaining neighbors are below the mean.
            for i in range(0, 4):
                west_contributes  = False
                east_contributes  = False
                south_contributes = False
                north_contributes = False

                local_sum   = workload
                local_count = 1

                if (west_workload  != -1 and west_workload  < local_mean):
                    local_sum        += west_workload
                    local_count      += 1
                    west_contributes = True
                if (east_workload  != -1 and east_workload  < local_mean):
                    local_sum        += east_workload
                    local_count      += 1
                    east_contributes = True
                if (south_workload != -1 and south_workload < local_mean):
                    local_sum         += south_workload
                    local_count       += 1
                    south_contributes = True
                if (north_workload != -1 and north_workload < local_mean):
                    local_sum         += north_workload
                    local_count       += 1
                    north_contributes = True
                local_mean = int(local_sum / local_count)

            if (west_contributes ) : 
                west_outgoing  = int(local_mean - west_workload )
            if (east_contributes ) : 
                east_outgoing  = int(local_mean - east_workload )
            if (south_contributes) : 
                south_outgoing = int(local_mean - south_workload)
            if (north_contributes) : 
                north_outgoing = int(local_mean - north_workload)

            balanced_data["value"][index]               -= (west_outgoing + east_outgoing + south_outgoing + north_outgoing)
            balanced_data["value"][neighbor_indices[0]] +=  west_outgoing 
            balanced_data["value"][neighbor_indices[1]] +=  east_outgoing 
            balanced_data["value"][neighbor_indices[2]] +=  south_outgoing
            balanced_data["value"][neighbor_indices[3]] +=  north_outgoing

            if west_outgoing  > 0:
                outgoing_arrow_data["x_start"].append(x)
                outgoing_arrow_data["y_start"].append(y)
                outgoing_arrow_data["x_end"  ].append(x - 0.65)
                outgoing_arrow_data["y_end"  ].append(y)
            if east_outgoing  > 0:
                outgoing_arrow_data["x_start"].append(x)
                outgoing_arrow_data["y_start"].append(y)
                outgoing_arrow_data["x_end"  ].append(x + 0.65)
                outgoing_arrow_data["y_end"  ].append(y)
            if south_outgoing > 0:
                outgoing_arrow_data["x_start"].append(x)
                outgoing_arrow_data["y_start"].append(y)
                outgoing_arrow_data["x_end"  ].append(x)
                outgoing_arrow_data["y_end"  ].append(y - 0.65)
            if north_outgoing > 0:
                outgoing_arrow_data["x_start"].append(x)
                outgoing_arrow_data["y_start"].append(y)
                outgoing_arrow_data["x_end"  ].append(x)
                outgoing_arrow_data["y_end"  ].append(y + 0.65)

    result = {}
    result["data"               ] = balanced_data
    result["incoming_arrow_data"] = incoming_arrow_data
    result["outgoing_arrow_data"] = outgoing_arrow_data

    return result
def load_balance_gllma (data, width, height):
    balanced_data       = copy.deepcopy(data)
    incoming_arrow_data = {"x_start": [], "y_start": [], "x_end": [], "y_end": []}
    outgoing_arrow_data = {"x_start": [], "y_start": [], "x_end": [], "y_end": []}

    west_quota          = [0] * width * height
    east_quota          = [0] * width * height
    south_quota         = [0] * width * height
    north_quota         = [0] * width * height
    
    for x in range(0, width):
        for y in range(0, height):
            index            = numpy.ravel_multi_index((x, y), (width, height))
            neighbor_indices = [-1, -1, -1, -1]

            if (x > 0)        : neighbor_indices[0] = numpy.ravel_multi_index((x - 1, y    ), (width, height))
            if (x < width - 1): neighbor_indices[1] = numpy.ravel_multi_index((x + 1, y    ), (width, height))
            if (y > 0)        : neighbor_indices[2] = numpy.ravel_multi_index((x    , y - 1), (width, height))
            if (y < width - 1): neighbor_indices[3] = numpy.ravel_multi_index((x    , y + 1), (width, height))
            
            workload       = data["value"][index]
            
            west_workload  = data["value"][neighbor_indices[0]] if neighbor_indices[0] != -1 else -1
            east_workload  = data["value"][neighbor_indices[1]] if neighbor_indices[1] != -1 else -1
            south_workload = data["value"][neighbor_indices[2]] if neighbor_indices[2] != -1 else -1
            north_workload = data["value"][neighbor_indices[3]] if neighbor_indices[3] != -1 else -1
            
            # Take the mean of neighbors with workload more than this process.
            local_sum     = workload
            local_count   = 1
            if (west_workload  != -1 and west_workload  > workload):
                local_sum   += west_workload
                local_count += 1
            if (east_workload  != -1 and east_workload  > workload):
                local_sum   += east_workload
                local_count += 1
            if (south_workload != -1 and south_workload > workload):
                local_sum   += south_workload
                local_count += 1
            if (north_workload != -1 and north_workload > workload):
                local_sum   += north_workload
                local_count += 1
            local_mean = int(local_sum / local_count)

            # Take the mean of neighbors with workload more than the mean until all remaining neighbors are above the mean.
            for i in range(0, 4):
                west_contributes  = False
                east_contributes  = False
                south_contributes = False
                north_contributes = False

                local_sum   = workload
                local_count = 1

                if (west_workload  != -1 and west_workload  > local_mean):
                    local_sum        += west_workload
                    local_count      += 1
                    west_contributes = True
                if (east_workload  != -1 and east_workload  > local_mean):
                    local_sum        += east_workload
                    local_count      += 1
                    east_contributes = True
                if (south_workload != -1 and south_workload > local_mean):
                    local_sum         += south_workload
                    local_count       += 1
                    south_contributes = True
                if (north_workload != -1 and north_workload > local_mean):
                    local_sum         += north_workload
                    local_count       += 1
                    north_contributes = True
                local_mean = int(local_sum / local_count)

            # Compute quota.
            quota_workload        = local_mean - workload
            quota_neighbor_totals = 0
            if (west_contributes ) : 
                quota_neighbor_totals += west_workload
            if (east_contributes ) : 
                quota_neighbor_totals += east_workload
            if (south_contributes) : 
                quota_neighbor_totals += south_workload
            if (north_contributes) : 
                quota_neighbor_totals += north_workload
            
            if (west_contributes ) : 
                west_quota [index] = int(quota_workload * west_workload  / quota_neighbor_totals)
            if (east_contributes ) : 
                east_quota [index] = int(quota_workload * east_workload  / quota_neighbor_totals)
            if (south_contributes) : 
                south_quota[index] = int(quota_workload * south_workload / quota_neighbor_totals)
            if (north_contributes) : 
                north_quota[index] = int(quota_workload * north_workload / quota_neighbor_totals)

    for x in range(0, width):
        for y in range(0, height):
            index            = numpy.ravel_multi_index((x, y), (width, height))
            neighbor_indices = [-1, -1, -1, -1]

            if (x > 0)        : neighbor_indices[0] = numpy.ravel_multi_index((x - 1, y    ), (width, height))
            if (x < width - 1): neighbor_indices[1] = numpy.ravel_multi_index((x + 1, y    ), (width, height))
            if (y > 0)        : neighbor_indices[2] = numpy.ravel_multi_index((x    , y - 1), (width, height))
            if (y < width - 1): neighbor_indices[3] = numpy.ravel_multi_index((x    , y + 1), (width, height))
            
            workload       = data["value"][index]

            west_workload  = data["value"][neighbor_indices[0]] if neighbor_indices[0] != -1 else -1
            east_workload  = data["value"][neighbor_indices[1]] if neighbor_indices[1] != -1 else -1
            south_workload = data["value"][neighbor_indices[2]] if neighbor_indices[2] != -1 else -1
            north_workload = data["value"][neighbor_indices[3]] if neighbor_indices[3] != -1 else -1
            
            west_outgoing  = 0
            east_outgoing  = 0
            south_outgoing = 0
            north_outgoing = 0

            # Take the mean of neighbors with workload less than this process.
            local_sum   = workload
            local_count = 1
            if (west_workload  != -1 and west_workload  < workload):
                local_sum   += west_workload
                local_count += 1
            if (east_workload  != -1 and east_workload  < workload):
                local_sum   += east_workload
                local_count += 1
            if (south_workload != -1 and south_workload < workload):
                local_sum   += south_workload
                local_count += 1
            if (north_workload != -1 and north_workload < workload):
                local_sum   += north_workload
                local_count += 1
            local_mean  = int(local_sum / local_count)

            # Take the mean of neighbors with workload less than the mean until all remaining neighbors are below the mean.
            for i in range(0, 4):
                west_contributes  = False
                east_contributes  = False
                south_contributes = False
                north_contributes = False

                local_sum   = workload
                local_count = 1

                if (west_workload  != -1 and west_workload  < local_mean):
                    local_sum        += west_workload
                    local_count      += 1
                    west_contributes = True
                if (east_workload  != -1 and east_workload  < local_mean):
                    local_sum        += east_workload
                    local_count      += 1
                    east_contributes = True
                if (south_workload != -1 and south_workload < local_mean):
                    local_sum         += south_workload
                    local_count       += 1
                    south_contributes = True
                if (north_workload != -1 and north_workload < local_mean):
                    local_sum         += north_workload
                    local_count       += 1
                    north_contributes = True
                local_mean = int(local_sum / local_count)

            # Compute redistribution, based on quota.
            if (west_contributes ):
                west_outgoing  = min(int(local_mean - west_workload ), east_quota [neighbor_indices[0]]) if neighbor_indices[0] != -1 else 0
            if (east_contributes ):
                east_outgoing  = min(int(local_mean - east_workload ), west_quota [neighbor_indices[1]]) if neighbor_indices[1] != -1 else 0
            if (south_contributes):
                south_outgoing = min(int(local_mean - south_workload), north_quota[neighbor_indices[2]]) if neighbor_indices[2] != -1 else 0
            if (north_contributes):
                north_outgoing = min(int(local_mean - north_workload), south_quota[neighbor_indices[3]]) if neighbor_indices[3] != -1 else 0

            balanced_data["value"][index]               -= (west_outgoing + east_outgoing + south_outgoing + north_outgoing)
            balanced_data["value"][neighbor_indices[0]] +=  west_outgoing 
            balanced_data["value"][neighbor_indices[1]] +=  east_outgoing 
            balanced_data["value"][neighbor_indices[2]] +=  south_outgoing
            balanced_data["value"][neighbor_indices[3]] +=  north_outgoing

            if neighbor_indices[0] != -1 and east_quota [neighbor_indices[0]] > 0:
                incoming_arrow_data["x_start"].append(x)
                incoming_arrow_data["y_start"].append(y)
                incoming_arrow_data["x_end"  ].append(x - 0.65)
                incoming_arrow_data["y_end"  ].append(y)
            if neighbor_indices[1] != -1 and west_quota [neighbor_indices[1]] > 0:
                incoming_arrow_data["x_start"].append(x)
                incoming_arrow_data["y_start"].append(y)
                incoming_arrow_data["x_end"  ].append(x + 0.65)
                incoming_arrow_data["y_end"  ].append(y)
            if neighbor_indices[2] != -1 and north_quota[neighbor_indices[2]] > 0:
                incoming_arrow_data["x_start"].append(x)
                incoming_arrow_data["y_start"].append(y)
                incoming_arrow_data["x_end"  ].append(x)
                incoming_arrow_data["y_end"  ].append(y - 0.65)
            if neighbor_indices[3] != -1 and south_quota[neighbor_indices[3]] > 0:
                incoming_arrow_data["x_start"].append(x)
                incoming_arrow_data["y_start"].append(y)
                incoming_arrow_data["x_end"  ].append(x)
                incoming_arrow_data["y_end"  ].append(y + 0.65)
            if west_outgoing  > 0:
                outgoing_arrow_data["x_start"].append(x)
                outgoing_arrow_data["y_start"].append(y)
                outgoing_arrow_data["x_end"  ].append(x - 0.65)
                outgoing_arrow_data["y_end"  ].append(y)
            if east_outgoing  > 0:
                outgoing_arrow_data["x_start"].append(x)
                outgoing_arrow_data["y_start"].append(y)
                outgoing_arrow_data["x_end"  ].append(x + 0.65)
                outgoing_arrow_data["y_end"  ].append(y)
            if south_outgoing > 0:
                outgoing_arrow_data["x_start"].append(x)
                outgoing_arrow_data["y_start"].append(y)
                outgoing_arrow_data["x_end"  ].append(x)
                outgoing_arrow_data["y_end"  ].append(y - 0.65)
            if north_outgoing > 0:
                outgoing_arrow_data["x_start"].append(x)
                outgoing_arrow_data["y_start"].append(y)
                outgoing_arrow_data["x_end"  ].append(x)
                outgoing_arrow_data["y_end"  ].append(y + 0.65)

    result = {}
    result["data"               ] = balanced_data
    result["incoming_arrow_data"] = incoming_arrow_data
    result["outgoing_arrow_data"] = outgoing_arrow_data

    return result

def plot               (data, maximum_load, incoming_arrow_data, outgoing_arrow_data):
    plot = figure(title="", x_axis_label='', y_axis_label='', sizing_mode="scale_height")
    plot.xgrid.grid_line_color            = None
    plot.ygrid.grid_line_color            = None
    plot.xaxis.major_tick_line_color      = None
    plot.xaxis.minor_tick_line_color      = None
    plot.xaxis.major_label_text_color     = '#ffffff'
    plot.xaxis.major_label_text_font_size = '0px'
    plot.xaxis.axis_line_color            = "#ffffff"
    plot.yaxis.major_tick_line_color      = None
    plot.yaxis.minor_tick_line_color      = None
    plot.yaxis.major_label_text_color     = '#ffffff'
    plot.yaxis.major_label_text_font_size = '0px'
    plot.yaxis.axis_line_color            = "#ffffff"
    plot.toolbar.logo                     = None
    plot.toolbar_location                 = None

    colors     = Color("white").range_to(Color("red"), maximum_load)
    colors_hex = []
    for color in colors:
        colors_hex.append(color.hex_l)

    source = ColumnDataSource(data)
    mapper = LinearColorMapper(palette=colors_hex, low=0, high=len(colors_hex))
    labels = LabelSet(x="x", y="y", text="value", text_align="center", text_font="helvetica", text_font_size="16pt", x_offset=0, y_offset=-12, source=source, render_mode='canvas')
    plot.rect(x="x", y="y", width=1, height=1, source=source, fill_color={'field': 'value', 'transform': mapper}, line_color="#000000", line_alpha=0)
    plot.add_layout(labels)
    
    if incoming_arrow_data:
        incoming_arrow_source = ColumnDataSource(incoming_arrow_data)
        incoming_arrows = Arrow(end=VeeHead(fill_color="gray"), source=incoming_arrow_source, x_start='x_start', y_start='y_start', x_end='x_end', y_end='y_end', line_color="gray", line_alpha=0)
        plot.add_layout(incoming_arrows)

    if outgoing_arrow_data:
        outgoing_arrow_source = ColumnDataSource(outgoing_arrow_data)
        outgoing_arrows = Arrow(end=VeeHead(fill_color="black"), source=outgoing_arrow_source, x_start='x_start', y_start='y_start', x_end='x_end', y_end='y_end', line_color="black", line_alpha=0)
        plot.add_layout(outgoing_arrows)

    return plot
    
width               = 5
height              = 5
maximum_load        = 1000

#data                = generate_example   ()
data                = generate_fixed     ()
#data                = generate_random   (width, height, maximum_load)

balanced_data_orig  = load_balance_orig (data, width, height)
balanced_data_lma   = load_balance_lma  (data, width, height)
balanced_data_gllma = load_balance_gllma(data, width, height)

#plots = row(
#    plot(data                       , maximum_load, None, None), 
#    plot(balanced_data_orig ["data"], maximum_load, None, None),
#    plot(balanced_data_lma  ["data"], maximum_load, None, None),
#    plot(balanced_data_gllma["data"], maximum_load, None, None))
#plots = row(
#    plot(data                       , maximum_load, None, balanced_data_lma["outgoing_arrow_data"]),
#    plot(balanced_data_lma  ["data"], maximum_load, None, None))
plots = row(
    plot(data                       , maximum_load, balanced_data_gllma["incoming_arrow_data"], None),
    plot(data                       , maximum_load, balanced_data_gllma["incoming_arrow_data"], balanced_data_gllma["outgoing_arrow_data"]),
    plot(balanced_data_gllma["data"], maximum_load, None, None))

output_file("heatmap.html")
export_png (plots, filename="heatmap.png")
show       (plots)
