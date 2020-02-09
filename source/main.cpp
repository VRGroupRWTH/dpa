#include <cstdint>

#include <dpa/pipeline.hpp>
#include "dpa/types/regular_fields.hpp"

#include <iostream>

template <typename grid_type>
void print_grid(const grid_type& grid)
{
  auto shape = grid.data.shape();
  //for (auto z = 0; z < shape[2]; ++z)
  //{
    //std::cout << "Z" << z << "\n";
    for (auto y = 0; y < shape[1]; ++y)
    {
      for (auto x = 0; x < shape[0]; ++x)
      {
        std::cout << grid.data[x][y] << " ";
      }
      std::cout << "\n";
    }
    //std::cout << "\n";
  //}
}

std::int32_t main(std::int32_t argc, char** argv)
{
  dpa::regular_scalar_field_2d original {boost::multi_array<dpa::scalar, 2>(boost::extents[3][3]), dpa::vector2::Zero(), dpa::vector2::Ones(), 1.0f / 3.0f * dpa::vector2::Ones()};
  original.data[1][1] = 0.22f;
  original.data[0][1] = 0.11f;
  original.data[2][1] = 0.11f;
  original.data[1][0] = 0.11f;
  original.data[1][2] = 0.11f;

  auto gradient_1  = original   .gradient ();
  auto gradient_2  = gradient_1 .gradient ();
  auto potential_1 = gradient_2 .potential();
  auto potential_2 = potential_1.potential();

  std::cout << "Original (Scalar) \n";
  print_grid<>(original   );
  std::cout << "Gradient 1 (Vector) \n";
  print_grid<>(gradient_1 );
  std::cout << "Gradient 2 (Tensor) \n";
  print_grid<>(gradient_2 );
  std::cout << "Potential 1 (Vector)\n";
  print_grid<>(potential_1);
  std::cout << "Potential 2 (Scalar)\n";
  print_grid<>(potential_2);

  return dpa::pipeline().run(argc, argv);
}
