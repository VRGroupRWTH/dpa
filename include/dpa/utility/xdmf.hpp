#ifndef DPA_UTILITY_XDMF_HPP
#define DPA_UTILITY_XDMF_HPP

#include <string>

namespace dpa
{
const std::string xdmf_header = R"(<?xml version ="1.0" ?>
<!DOCTYPE xdmf SYSTEM "Xdmf.dtd" []>
<Xdmf Version="2.0">
  <Domain>
)";
const std::string xdmf_body_points = R"(
    <Grid Name="Grid">
      <Topology TopologyType="Polyvertex" NodesPerElement="1" NumberOfElements="$VERTEX_COUNT" />
      <Geometry GeometryType="XYZ">
        <DataItem Dimensions="$VERTEX_ARRAY_SIZE" NumberType="Float" Precision="4" Format="HDF">
          $FILEPATH:/vertices
        </DataItem>
      </Geometry>
      <Attribute Name="Colors" AttributeType="$COLOR_ATTRIBUTE_TYPE" Center="Node">
        <DataItem Dimensions="$COLOR_ARRAY_SIZE" NumberType="$COLOR_ARRAY_TYPE" Precision="$COLOR_PRECISION" Format="HDF">
          $FILEPATH:/colors
        </DataItem>
      </Attribute>
    </Grid>
)";
const std::string xdmf_body_lines = R"(
    <Grid Name="$GRID_NAME">

      <Topology TopologyType="Polyline" NodesPerElement="2" NumberOfElements="$POLYLINE_COUNT">
        <DataItem Dimensions="$INDEX_ARRAY_SIZE" NumberType="UInt" Precision="$INDEX_PRECISION" Format="HDF">
          $FILEPATH:/$INDICES_DATASET_NAME
        </DataItem>
      </Topology>

      <Geometry GeometryType="XYZ">
        <DataItem Dimensions="$VERTEX_ARRAY_SIZE" NumberType="Float" Precision="4" Format="HDF">
          $FILEPATH:/$VERTICES_DATASET_NAME
        </DataItem>
      </Geometry>

      <Attribute Name="Colors" AttributeType="$COLOR_ATTRIBUTE_TYPE" Center="Node">
        <DataItem Dimensions="$COLOR_ARRAY_SIZE" NumberType="$COLOR_ARRAY_TYPE" Precision="$COLOR_PRECISION" Format="HDF">
          $FILEPATH:/$COLORS_DATASET_NAME
        </DataItem>
      </Attribute>

    </Grid>
)";
const std::string xdmf_body_volume = R"(
    <Grid Name="Grid" GridType="Uniform">
      <Topology TopologyType="3DCoRectMesh" NumberOfElements="$SIZE"/>
      <Geometry GeometryType="ORIGIN_DXDYDZ">
        <DataItem Dimensions="3" NumberType="Float" Precision="4" Format="XML">
          $ORIGIN
        </DataItem>
        <DataItem Dimensions="3" NumberType="Float" Precision="4" Format="XML">
          $SPACING
        </DataItem>
      </Geometry>
      <Attribute Name="Volume" AttributeType="Scalar" Center="Node">
        <DataItem Dimensions="$SIZE" NumberType="Float" Precision="4" Format="HDF">
          $FILEPATH:/$DATASET_NAME
        </DataItem>
      </Attribute>
    </Grid>
)";
const std::string xdmf_footer = R"(
  </Domain>
</Xdmf>
)";
}

#endif