#ifndef DPA_UTILITY_XDMF_HPP
#define DPA_UTILITY_XDMF_HPP

#include <boost/algorithm/string/replace.hpp>

#include <cstddef>
#include <string>

namespace dpa
{
inline std::string create_xdmf(const std::string& dataset_name, const std::size_t vertex_count, const std::size_t index_count)
{
  std::string xdmf = 
R"(<?xml version ="1.0" ?>
<!DOCTYPE xdmf SYSTEM "Xdmf.dtd" []>
<Xdmf Version="2.0">
  <Domain>
    <Grid Name="$DATASET_NAME">
      <Topology TopologyType="Polyline">
        <DataItem Dimensions="$INDEX_COUNT" NumberType="Uint" Precision="4" Format="HDF">
          $DATASET_NAME:/indices
        </DataItem>
      </Topology>
      <Geometry GeometryType="XYZ">
        <DataItem Dimensions="$VERTEX_COUNT" NumberType="Float" Precision="4" Format="HDF">
          $DATASET_NAME:/vertices
        </DataItem>
      </Geometry>
      <Attribute Name="Colors" AttributeType="Scalar" Center="Node">
        <DataItem Dimensions="$VERTEX_COUNT" NumberType="Uint" Precision="1" Format="HDF">
          $DATASET_NAME:/colors
        </DataItem>
      </Attribute>
    </Grid>
  </Domain>
</Xdmf>
)";

  boost::replace_all(xdmf, "$DATASET_NAME", dataset_name);
  boost::replace_all(xdmf, "$VERTEX_COUNT", std::to_string(vertex_count));
  boost::replace_all(xdmf, "$INDEX_COUNT" , std::to_string(index_count ));
  return xdmf;
}
}

#endif