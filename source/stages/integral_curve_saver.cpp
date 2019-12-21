#include <dpa/stages/integral_curve_saver.hpp>

#include <filesystem>
#include <fstream>
#include <variant>

#include <boost/algorithm/string/replace.hpp>
#include <boost/mpi.hpp>
#include <tbb/tbb.h>

#undef min
#undef max

namespace dpa
{
const std::string xdmf_preamble  = R"(<?xml version ="1.0" ?>
<!DOCTYPE xdmf SYSTEM "Xdmf.dtd" []>
<Xdmf Version="2.0">
  <Domain>
)";
const std::string xdmf_body      = R"(
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
const std::string xdmf_postamble = R"(
  </Domain>
</Xdmf>
)";

integral_curve_saver::integral_curve_saver (domain_partitioner* partitioner, const std::string& filepath) : partitioner_(partitioner), filepath_(filepath + std::string(".rank") + std::to_string(partitioner_->cartesian_communicator()->rank()))
{
  file_ = H5Fcreate(filepath_.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
}
integral_curve_saver::~integral_curve_saver()
{
  H5Fclose(file_);
}

void integral_curve_saver::save_integral_curves(const integral_curves_3d& integral_curves, bool use_64_bit_indices)
{
  std::vector<std::string> xdmfs(integral_curves.size(), xdmf_body);

  for (auto curve_index = 0; curve_index < integral_curves.size(); ++curve_index)
  //tbb::parallel_for(std::size_t(0), integral_curves.size(), std::size_t(1), [&] (const std::size_t curve_index)
  {
    auto& vertices = integral_curves[curve_index];

    if (vertices.empty())
      return;

    std::vector<std::array<std::uint8_t, 3>> colors(vertices.size(), std::array<std::uint8_t, 3>{0, 0, 0});
    tbb::parallel_for(std::size_t(0), vertices.size(), std::size_t(1), [&] (const std::size_t vertex_index)
    {
      if (vertices[vertex_index] != terminal_value<vector3>())
      {
        vector3 tangent;
        if (vertex_index     > 1               && vertices[vertex_index - 1] != terminal_value<vector3>())
          tangent = (vertices[vertex_index    ] - vertices[vertex_index - 1]).normalized();
        if (vertex_index + 1 < vertices.size() && vertices[vertex_index + 1] != terminal_value<vector3>())
          tangent = ((tangent + (vertices[vertex_index + 1] - vertices[vertex_index]).normalized()) / scalar(2)).normalized();

        colors[vertex_index] = std::array<std::uint8_t, 3>
        {
          std::uint8_t(255 * std::abs(tangent[0])),
          std::uint8_t(255 * std::abs(tangent[1])),
          std::uint8_t(255 * std::abs(tangent[2]))
        };
      }
    });

    std::variant<std::vector<std::uint32_t>, std::vector<std::uint64_t>> indices;
    if (use_64_bit_indices)
      indices = std::vector<std::uint64_t>(2 * vertices.size(), std::numeric_limits<std::uint64_t>::max());
    else
      indices = std::vector<std::uint32_t>(2 * vertices.size(), std::numeric_limits<std::uint32_t>::max());

    std::visit([&] (auto& cast_indices) 
    {
      tbb::parallel_for(std::size_t(0), vertices.size() - 1, std::size_t(1), [&] (const std::size_t vertex_index)
      {
        if (vertices[vertex_index] != terminal_value<vector3>() && vertices[vertex_index + 1] != terminal_value<vector3>())
        {
          cast_indices[2 * vertex_index    ] = vertex_index;
          cast_indices[2 * vertex_index + 1] = vertex_index + 1;
        }
      });
      cast_indices.erase(std::remove(cast_indices.begin(), cast_indices.end(), use_64_bit_indices ? std::numeric_limits<std::uint64_t>::max() : std::numeric_limits<std::uint32_t>::max()), cast_indices.end());
    }, indices);

    const auto vertex_element_count = 3 * std::size_t(vertices.size());
    const auto color_element_count  = 3 * std::size_t(vertices.size());
    const auto index_count          =     std::size_t(use_64_bit_indices ? std::get<std::vector<std::uint64_t>>(indices).size() : std::get<std::vector<std::uint32_t>>(indices).size());
    const auto name_prefix          = std::string("_rank_") + std::to_string(partitioner_->cartesian_communicator()->rank()) + "_round_" + std::to_string(curve_index) + "_";
    const auto vertices_name        = name_prefix + "vertices";
    const auto colors_name          = name_prefix + "colors"  ;
    const auto indices_name         = name_prefix + "indices" ;
    const auto vertices_space       = H5Screate_simple(1, &vertex_element_count, nullptr);
    const auto colors_space         = H5Screate_simple(1, &color_element_count , nullptr);
    const auto indices_space        = H5Screate_simple(1, &index_count         , nullptr);
    const auto vertices_dataset     = H5Dcreate2(file_, vertices_name.c_str(), H5T_NATIVE_FLOAT                                          , vertices_space, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    const auto colors_dataset       = H5Dcreate2(file_, colors_name  .c_str(), H5T_NATIVE_UINT8                                          , colors_space  , H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    const auto indices_dataset      = H5Dcreate2(file_, indices_name .c_str(), use_64_bit_indices ? H5T_NATIVE_UINT64 : H5T_NATIVE_UINT32, indices_space , H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

    H5Dwrite(vertices_dataset, H5T_NATIVE_FLOAT                                          , vertices_space, vertices_space, H5P_DEFAULT, vertices.data()->data());
    H5Dwrite(colors_dataset  , H5T_NATIVE_UINT8                                          , colors_space  , colors_space  , H5P_DEFAULT, colors  .data()->data());
    H5Dwrite(indices_dataset , use_64_bit_indices ? H5T_NATIVE_UINT64 : H5T_NATIVE_UINT32, indices_space , indices_space , H5P_DEFAULT, use_64_bit_indices ? reinterpret_cast<void*>(std::get<std::vector<std::uint64_t>>(indices).data()) : std::get<std::vector<std::uint32_t>>(indices).data());
    
    H5Sclose(vertices_space  );
    H5Sclose(colors_space    );
    H5Sclose(indices_space   );
    H5Dclose(vertices_dataset);
    H5Dclose(colors_dataset  );
    H5Dclose(indices_dataset );
    
    boost::replace_all(xdmfs[curve_index], "$GRID_NAME"            , name_prefix);
    boost::replace_all(xdmfs[curve_index], "$POLYLINE_COUNT"       , std::to_string(index_count / 2));
    boost::replace_all(xdmfs[curve_index], "$FILEPATH"             , std::filesystem::path(filepath_).filename().string());
    boost::replace_all(xdmfs[curve_index], "$INDICES_DATASET_NAME" , indices_name );
    boost::replace_all(xdmfs[curve_index], "$VERTICES_DATASET_NAME", vertices_name);
    boost::replace_all(xdmfs[curve_index], "$COLORS_DATASET_NAME"  , colors_name  );
    boost::replace_all(xdmfs[curve_index], "$INDEX_ARRAY_SIZE"     , std::to_string(index_count));
    boost::replace_all(xdmfs[curve_index], "$INDEX_PRECISION"      , use_64_bit_indices ? "8" : "4");
    boost::replace_all(xdmfs[curve_index], "$VERTEX_ARRAY_SIZE"    , std::to_string(vertex_element_count));
  //});
  }

  std::ofstream stream(filepath_ + ".xdmf");
  stream << xdmf_preamble << std::accumulate(xdmfs.begin(), xdmfs.end(), std::string("")) << xdmf_postamble;
}
}