#ifndef DPA_BASIC_TYPES_HPP
#define DPA_BASIC_TYPES_HPP

#define EIGEN_INITIALIZE_MATRICES_BY_ZERO
#define EIGEN_USE_MKL 
#define EIGEN_USE_MKL_ALL
#define MKL_DIRECT_CALL

#include <cstdint>

#include <Eigen/Core>
#include <Eigen/Geometry>

namespace dpa
{
using scalar   = float;
using integer  = std::int32_t;
using size     = std::size_t;
               
using vector2  = Eigen::Matrix<scalar , 2, 1>;
using vector3  = Eigen::Matrix<scalar , 3, 1>;
using vector4  = Eigen::Matrix<scalar , 4, 1>;
using ivector2 = Eigen::Matrix<integer, 2, 1>;
using ivector3 = Eigen::Matrix<integer, 3, 1>;
using ivector4 = Eigen::Matrix<integer, 4, 1>;
using svector2 = Eigen::Matrix<size   , 2, 1>;
using svector3 = Eigen::Matrix<size   , 3, 1>;
using svector4 = Eigen::Matrix<size   , 4, 1>;

using matrix2  = Eigen::Matrix<scalar , 2, 2>;
using matrix3  = Eigen::Matrix<scalar , 3, 3>;
using matrix4  = Eigen::Matrix<scalar , 4, 4>;
using imatrix2 = Eigen::Matrix<integer, 2, 2>;
using imatrix3 = Eigen::Matrix<integer, 3, 3>;
using imatrix4 = Eigen::Matrix<integer, 4, 4>;
using smatrix2 = Eigen::Matrix<size   , 2, 2>;
using smatrix3 = Eigen::Matrix<size   , 3, 3>;
using smatrix4 = Eigen::Matrix<size   , 4, 4>;

using aabb2    = Eigen::AlignedBox<scalar, 2>;
using aabb3    = Eigen::AlignedBox<scalar, 3>;
using aabb4    = Eigen::AlignedBox<scalar, 4>;

template <typename type>
type invalid_value ()
{
  return -1 * type::Ones();
}
template <typename type>
type terminal_value()
{
  return -2 * type::Ones();
}

template <typename type, std::size_t dimensions>
struct vector_traits
{

};
template <>
struct vector_traits<scalar , 2>
{
  using type = vector2;
};
template <>
struct vector_traits<scalar , 3>
{
  using type = vector3;
};
template <>
struct vector_traits<scalar , 4>
{
  using type = vector4;
};
template <>
struct vector_traits<integer, 2>
{
  using type = ivector2;
};
template <>
struct vector_traits<integer, 3>
{
  using type = ivector3;
};
template <>
struct vector_traits<integer, 4>
{
  using type = ivector4;
};
template <>
struct vector_traits<size   , 2>
{
  using type = svector2;
};
template <>
struct vector_traits<size   , 3>
{
  using type = svector3;
};
template <>
struct vector_traits<size   , 4>
{
  using type = svector4;
};

template <typename type, std::size_t dimensions>
struct matrix_traits
{

};
template <>
struct matrix_traits<scalar , 2>
{
  using type = matrix2;
};
template <>
struct matrix_traits<scalar , 3>
{
  using type = matrix3;
};
template <>
struct matrix_traits<scalar , 4>
{
  using type = matrix4;
};
template <>
struct matrix_traits<integer, 2>
{
  using type = imatrix2;
};
template <>
struct matrix_traits<integer, 3>
{
  using type = imatrix3;
};
template <>
struct matrix_traits<integer, 4>
{
  using type = imatrix4;
};
template <>
struct matrix_traits<size   , 2>
{
  using type = smatrix2;
};
template <>
struct matrix_traits<size   , 3>
{
  using type = smatrix3;
};
template <>
struct matrix_traits<size   , 4>
{
  using type = smatrix4;
};
}

#endif