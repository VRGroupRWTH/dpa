#include <cstdint>
#include <iostream>

#include <dpa/pipeline.hpp>
#include <dpa/types/regular_fields.hpp>

std::int32_t main(std::int32_t argc, char** argv)
{
  auto scalars = dpa::regular_scalar_field_2d {boost::multi_array<float, 2>(boost::extents[3][3]), dpa::vector2(0, 0), dpa::vector2(1, 1), dpa::vector2(0.5, 0.5)};
  scalars.apply_window_parallel([ ] (const std::array<std::size_t, 2>& index, dpa::scalar& element, dpa::regular_scalar_field_2d::array_view_type& elements)
  {
    element = elements.num_elements();
  }, std::array<std::size_t, 2>{3, 3});
  auto laplacian                = scalars.laplacian                     ();
  auto structure_tensor         = scalars.structure_tensor              (std::array<std::size_t, 2>{3, 3});

  auto vectors                  = scalars.gradient                      ();
  auto divergence               = vectors.divergence                    ();
  auto vorticity                = vectors.vorticity                     ();
  auto q_criterion              = vectors.q_criterion                   ();

  auto tensors                  = vectors.gradient                      ();
  auto decomposition            = tensors.eigen_decomposition           ();
  auto reoriented_decomposition = tensors.reoriented_eigen_decomposition(std::array<std::size_t, 2>{3, 3});
  auto fa                       = tensors.fractional_anisotropy         ();
  auto ra                       = tensors.relative_anisotropy           ();
  auto vr                       = tensors.volume_ratio                  ();
  auto ad                       = tensors.axial_diffusivity             ();
  auto md                       = tensors.mean_diffusivity              ();
  auto rd                       = tensors.relative_diffusivity          ();

  tensors.apply_parallel([&] (const std::array<std::size_t, 2>& index, dpa::matrix2& element)
  {
    for (auto i = 0; i < 2; ++i)
      element.col(i) = reoriented_decomposition.data(index).first.col(i) * reoriented_decomposition.data(index).second[i];
  });
  
  auto shape = scalars.data.shape();
  for (auto y = 0; y < shape[1]; ++y)
  {
    for (auto x = 0; x < shape[0]; ++x)
      std::cout << scalars.data[x][y] << " ";
    std::cout << "\n";
  }
  for (auto y = 0; y < shape[1]; ++y)
  {
    for (auto x = 0; x < shape[0]; ++x)
      std::cout << vectors.data[x][y] << " ";
    std::cout << "\n";
  }
  for (auto y = 0; y < shape[1]; ++y)
  {
    for (auto x = 0; x < shape[0]; ++x)
      std::cout << tensors.data[x][y] << "\n";
    std::cout << "\n";
  }

  return dpa::pipeline().run(argc, argv);
}
