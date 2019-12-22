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
const std::string xdmf_body   = R"(
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

      <Attribute Name="Colors" AttributeType="Vector" Center="Node">
        <DataItem Dimensions="$VERTEX_ARRAY_SIZE" NumberType="UChar" Precision="1" Format="HDF">
          $FILEPATH:/$COLORS_DATASET_NAME
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