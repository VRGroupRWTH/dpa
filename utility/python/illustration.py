from bokeh.io import export_png
from bokeh.models import ColumnDataSource, LabelSet
from bokeh.plotting import figure, output_file, show

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

rectangles = {
    "x"         : [0, 0, 0, 1, 1, 1, 2, 2, 2], 
    "y"         : [0, 1, 2, 0, 1, 2, 0, 1, 2], 
    "color"     : ["#ffffff", "#aaaaaa", "#ffffff", "#aaaaaa", "#aaaaaa", "#aaaaaa", "#ffffff", "#aaaaaa", "#ffffff"], 
    "text_color": ["#000000", "#000000", "#000000", "#000000", "#008800", "#000000", "#000000", "#000000", "#000000"], 
    "name"      : ["P[-1,-1]", "P[-1,0]", "P[-1,1]", "P[0,-1]", "P[0,0]", "P[0,1]", "P[1,-1]", "P[1,0]", "P[1,1]"]}
lines = {
    "x"    : [-0.5, -0.5, 0.5, 0.5, 1.5, 1.5, 2.5, 2.5, 1.5,  1.5,  0.5, 0.5, -0.5], 
    "y"    : [ 0.5,  1.5, 1.5, 2.5, 2.5, 1.5, 1.5, 0.5, 0.5, -0.5, -0.5, 0.5,  0.5]}
points = {
    "x"    : [
        -0.4, -0.2,
        0.6, 0.8, 1.0, 1.2, 1.4, 
        1.6, 1.8, 2.0, 2.2, 2.4, 1.6, 1.8, 2.0, 2.2, 2.4, 1.6, 1.8, 2.0, 2.2, 2.4,
        0.6, 0.8, 1.0, 1.2, 1.4,
        0.6, 0.8],
    "y"    : [ 
        1.4, 1.4,
        1.4, 1.4, 1.4, 1.4, 1.4, 
        1.4, 1.4, 1.4, 1.4, 1.4, 1.2, 1.2, 1.2, 1.2, 1.2, 1.0, 1.0, 1.0, 1.0, 1.0,
        2.4, 2.4, 2.4, 2.4, 2.4,
        0.4, 0.4],
    "color": [
        "#880000", "#880000", 
        "#008800", "#008800", "#008800", "#880000", "#008888", 
        "#000088", "#000088", "#000088", "#000088", "#000088", "#000088", "#000088", "#000088", "#000088", "#000088", "#008800", "#008800", "#008800", "#008800", "#008800",
        "#880088", "#880088", "#880088", "#880088", "#880088",
        "#008888", "#008888"]
}

rectangle_source = ColumnDataSource(rectangles)
line_source      = ColumnDataSource(lines     )
point_source     = ColumnDataSource(points    )

labels = LabelSet(x="x", y="y", text="name", text_align="center", text_color={'field': 'text_color'}, text_font="helvetica", text_font_size="16pt", x_offset=-70, y_offset=-100, source=rectangle_source, render_mode='canvas')
plot.rect  (x="x", y="y", width=1, height=1, source=rectangle_source, fill_color={'field': 'color'}, line_color="#000000", line_alpha=1)
plot.line  (x="x", y="y", source=line_source, line_color='#008800', line_width=4)
#plot.circle(x="x", y="y", source=point_source, fill_color={'field': 'color'}, size=32)
plot.add_layout(labels)

output_file("illustration.html")
export_png (plot, filename="illustration.png")
show       (plot)
