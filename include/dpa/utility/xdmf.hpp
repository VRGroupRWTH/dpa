#ifndef DPA_UTILITY_XDMF_HPP
#define DPA_UTILITY_XDMF_HPP

#include <boost/algorithm/string/replace.hpp>

#include <cstddef>
#include <string>

namespace dpa
{
inline std::string create_xdmf(const std::string& filename, const std::size_t vertex_array_size, const std::size_t color_array_size, const std::size_t index_array_size)
{
  std::string xdmf = 
R"(<?xml version ="1.0" ?>
<!DOCTYPE xdmf SYSTEM "Xdmf.dtd" []>
<Xdmf Version="2.0">
  <Domain>
    <Grid Name="$DATASET_NAME">

      <Topology TopologyType="Polyline" NodesPerElement="2" NumberOfElements="$POLYLINE_COUNT">
        <DataItem Dimensions="$INDEX_ARRAY_SIZE" NumberType="UInt" Precision="4" Format="HDF">
          $DATASET_NAME:/indices
        </DataItem>
      </Topology>

      <Geometry GeometryType="XYZ">
        <DataItem Dimensions="$VERTEX_ARRAY_SIZE" NumberType="Float" Precision="4" Format="HDF">
          $DATASET_NAME:/vertices
        </DataItem>
      </Geometry>

      <Attribute Name="Colors" AttributeType="Vector" Center="Node">
        <DataItem Dimensions="$COLOR_ARRAY_SIZE" NumberType="UChar" Precision="1" Format="HDF">
          $DATASET_NAME:/colors
        </DataItem>
      </Attribute>

    </Grid>
  </Domain>
</Xdmf>
)";

  const auto index_element_count  = index_array_size  / 2;
  boost::replace_all(xdmf, "$DATASET_NAME"     , filename);
  boost::replace_all(xdmf, "$VERTEX_ARRAY_SIZE", std::to_string(vertex_array_size  ));
  boost::replace_all(xdmf, "$COLOR_ARRAY_SIZE" , std::to_string(color_array_size   ));
  boost::replace_all(xdmf, "$INDEX_ARRAY_SIZE" , std::to_string(index_array_size   ));
  boost::replace_all(xdmf, "$POLYLINE_COUNT"   , std::to_string(index_element_count));
  return xdmf;
}
}

#endif