#include <cstdint>

#include <dpa/pipeline.hpp>
#include "dpa/types/regular_fields.hpp"

#include <Eigen/Dense>
#include <iostream>

template <typename grid_type>
void print_grid(const grid_type& grid)
{
  auto shape = grid.data.shape();
  for (auto y = 0; y < shape[1]; ++y)
  {
    for (auto x = 0; x < shape[0]; ++x)
    {
      std::cout << grid.data[x][y] << "\n";
    }
    std::cout << "\n";
  }
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
  gradient_1.apply([ ] (const std::array<std::size_t, 2>& index, dpa::vector2& element)
  {
    element.normalize();
  });
  auto gradient_2  = gradient_1 .gradient ();
  gradient_2.apply([ ] (const std::array<std::size_t, 2>& index, dpa::matrix2& element)
  {
    //element        = element / element.norm();
    const auto svd = element.jacobiSvd(Eigen::ComputeFullU | Eigen::ComputeFullV);
    element = svd.matrixU() * Eigen::DiagonalMatrix<dpa::scalar, 2, 2>(svd.singularValues());

    //const Eigen::SelfAdjointEigenSolver<dpa::matrix2> solver(element.transpose().eval() * element);
    //const auto eigenvalues  = solver.eigenvalues ();
    //const auto eigenvectors = solver.eigenvectors();
    //element = eigenvectors * Eigen::DiagonalMatrix<dpa::scalar, 2, 2>(eigenvalues[0], eigenvalues[1]);
  });
  auto potential_1 = gradient_2.potential();
  potential_1.apply([ ] (const std::array<std::size_t, 2>& index, dpa::vector2& element)
  {
    element.normalize();
  });
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
  
  //dpa::regular_vector_field_2d original {boost::multi_array<dpa::vector2, 2>(boost::extents[3][3]), dpa::vector2::Zero(), dpa::vector2::Ones(), 1.0f / 3.0f * dpa::vector2::Ones()};
  //auto one_over_sqrt_2 = 1.0 / std::sqrt(2.0);
  //original.data[0][0] = dpa::vector2(-one_over_sqrt_2, -one_over_sqrt_2);
  //original.data[1][0] = dpa::vector2( 0.0            , -1.0            );
  //original.data[2][0] = dpa::vector2( one_over_sqrt_2, -one_over_sqrt_2);
  //original.data[0][1] = dpa::vector2(-1.0            ,  0.0            );
  //original.data[1][1] = dpa::vector2( 0.0            ,  0.0            );
  //original.data[2][1] = dpa::vector2( 1.0            ,  0.0            );
  //original.data[0][2] = dpa::vector2(-one_over_sqrt_2,  one_over_sqrt_2);
  //original.data[1][2] = dpa::vector2( 0.0            ,  1.0            );
  //original.data[2][2] = dpa::vector2( one_over_sqrt_2,  one_over_sqrt_2);
  //
  //auto gradient_1  = original   .gradient ();
  //auto potential_1 = gradient_1 .potential();
  //auto potential_2 = potential_1.potential();
  //
  //std::cout << "Original (Vector) \n";
  //print_grid<>(original   );
  //std::cout << "Gradient 1 (Tensor) \n";
  //print_grid<>(gradient_1 );
  //std::cout << "Potential 1 (Vector)\n";
  //print_grid<>(potential_1);
  //std::cout << "Potential 2 (Scalar)\n";
  //print_grid<>(potential_2);

  //dpa::regular_vector_field_2d original {boost::multi_array<dpa::vector2, 2>(boost::extents[3][3]), dpa::vector2::Zero(), dpa::vector2::Ones(), 1.0f / 3.0f * dpa::vector2::Ones()};
  //auto one_over_sqrt_2 = 1.0 / std::sqrt(2.0);
  //original.data[0][0] = dpa::vector2( one_over_sqrt_2,  one_over_sqrt_2);
  //original.data[1][0] = dpa::vector2( 0.0            ,  1.0            );
  //original.data[2][0] = dpa::vector2(-one_over_sqrt_2,  one_over_sqrt_2);
  //original.data[0][1] = dpa::vector2( 1.0            ,  0.0            );
  //original.data[1][1] = dpa::vector2( 0.0            ,  0.0            );
  //original.data[2][1] = dpa::vector2(-1.0            ,  0.0            );
  //original.data[0][2] = dpa::vector2( one_over_sqrt_2, -one_over_sqrt_2);
  //original.data[1][2] = dpa::vector2( 0.0            , -1.0            );
  //original.data[2][2] = dpa::vector2(-one_over_sqrt_2, -one_over_sqrt_2);
  //
  //auto potential_1 = original   .potential();
  //auto gradient_1  = potential_1.gradient ();
  //
  //std::cout << "Original (Vector) \n";
  //print_grid<>(original   );
  //std::cout << "Potential 1 (Scalar)\n";
  //print_grid<>(potential_1);
  //std::cout << "Gradient 1 (Vector) \n";
  //print_grid<>(gradient_1);

  return dpa::pipeline().run(argc, argv);
}
