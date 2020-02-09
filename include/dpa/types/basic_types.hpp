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
using byte     = std::uint8_t;
using size     = std::size_t;

using vector2  = Eigen::Matrix<scalar , 1, 2>;
using vector3  = Eigen::Matrix<scalar , 1, 3>;
using vector4  = Eigen::Matrix<scalar , 1, 4>;
using ivector2 = Eigen::Matrix<integer, 1, 2>;
using ivector3 = Eigen::Matrix<integer, 1, 3>;
using ivector4 = Eigen::Matrix<integer, 1, 4>;
using bvector2 = Eigen::Matrix<byte   , 1, 2>;
using bvector3 = Eigen::Matrix<byte   , 1, 3>;
using bvector4 = Eigen::Matrix<byte   , 1, 4>;
using svector2 = Eigen::Matrix<size   , 1, 2>;
using svector3 = Eigen::Matrix<size   , 1, 3>;
using svector4 = Eigen::Matrix<size   , 1, 4>;

using matrix2  = Eigen::Matrix<scalar , 2, 2>;
using matrix3  = Eigen::Matrix<scalar , 3, 3>;
using matrix4  = Eigen::Matrix<scalar , 4, 4>;
using imatrix2 = Eigen::Matrix<integer, 2, 2>;
using imatrix3 = Eigen::Matrix<integer, 3, 3>;
using imatrix4 = Eigen::Matrix<integer, 4, 4>;
using bmatrix2 = Eigen::Matrix<byte   , 2, 2>;
using bmatrix3 = Eigen::Matrix<byte   , 3, 3>;
using bmatrix4 = Eigen::Matrix<byte   , 4, 4>;
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

template <typename _type, std::size_t _dimensions>
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
struct vector_traits<byte   , 2>
{
  using type = bvector2;
};
template <>
struct vector_traits<byte   , 3>
{
  using type = bvector3;
};
template <>
struct vector_traits<byte   , 4>
{
  using type = bvector4;
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

template <typename _type, std::size_t _dimensions>
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
struct matrix_traits<byte   , 2>
{
  using type = bmatrix2;
};
template <>
struct matrix_traits<byte   , 3>
{
  using type = bmatrix3;
};
template <>
struct matrix_traits<byte   , 4>
{
  using type = bmatrix4;
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

template <typename _type, std::size_t _dimensions>
struct gradient_traits
{
  using type = void;
};
template <>
struct gradient_traits<scalar  , 2>
{
  using type = vector2;
};
template <>
struct gradient_traits<scalar  , 3>
{
  using type = vector3;
};
template <>
struct gradient_traits<scalar  , 4>
{
  using type = vector4;
};
template <>
struct gradient_traits<integer , 2>
{
  using type = ivector2;
};
template <>
struct gradient_traits<integer , 3>
{
  using type = ivector3;
};
template <>
struct gradient_traits<integer , 4>
{
  using type = ivector4;
};
template <>
struct gradient_traits<byte    , 2>
{
  using type = bvector2;
};
template <>
struct gradient_traits<byte    , 3>
{
  using type = bvector3;
};
template <>
struct gradient_traits<byte    , 4>
{
  using type = bvector4;
};
template <>
struct gradient_traits<size    , 2>
{
  using type = svector2;
};
template <>
struct gradient_traits<size    , 3>
{
  using type = svector3;
};
template <>
struct gradient_traits<size    , 4>
{
  using type = svector4;
};
template <>
struct gradient_traits<vector2 , 2>
{
  using type = matrix2;
};
template <>
struct gradient_traits<vector3 , 3>
{
  using type = matrix3;
};
template <>
struct gradient_traits<vector4 , 4>
{
  using type = matrix4;
};
template <>
struct gradient_traits<ivector2, 2>
{
  using type = imatrix2;
};
template <>
struct gradient_traits<ivector3, 3>
{
  using type = imatrix3;
};
template <>
struct gradient_traits<ivector4, 4>
{
  using type = imatrix4;
};
template <>
struct gradient_traits<bvector2, 2>
{
  using type = bmatrix2;
};
template <>
struct gradient_traits<bvector3, 3>
{
  using type = bmatrix3;
};
template <>
struct gradient_traits<bvector4, 4>
{
  using type = bmatrix4;
};
template <>
struct gradient_traits<svector2, 2>
{
  using type = smatrix2;
};
template <>
struct gradient_traits<svector3, 3>
{
  using type = smatrix3;
};
template <>
struct gradient_traits<svector4, 4>
{
  using type = smatrix4;
};

template <typename _type, std::size_t _dimensions>
struct potential_traits
{
  using type = void;
};
template <>
struct potential_traits<vector2 , 2>
{
  using type = scalar;
};
template <>
struct potential_traits<vector3 , 3>
{
  using type = scalar;
};
template <>
struct potential_traits<vector4 , 4>
{
  using type = scalar;
};
template <>
struct potential_traits<ivector2, 2>
{
  using type = integer;
};
template <>
struct potential_traits<ivector3, 3>
{
  using type = integer;
};
template <>
struct potential_traits<ivector4, 4>
{
  using type = integer;
};
template <>
struct potential_traits<bvector2, 2>
{
  using type = byte;
};
template <>
struct potential_traits<bvector3, 3>
{
  using type = byte;
};
template <>
struct potential_traits<bvector4, 4>
{
  using type = byte;
};
template <>
struct potential_traits<svector2, 2>
{
  using type = size;
};
template <>
struct potential_traits<svector3, 3>
{
  using type = size;
};
template <>
struct potential_traits<svector4, 4>
{
  using type = size;
};
template <>
struct potential_traits<matrix2 , 2>
{
  using type = vector2;
};
template <>
struct potential_traits<matrix3 , 3>
{
  using type = vector3;
};
template <>
struct potential_traits<matrix4 , 4>
{
  using type = vector4;
};
template <>
struct potential_traits<imatrix2, 2>
{
  using type = ivector2;
};
template <>
struct potential_traits<imatrix3, 3>
{
  using type = ivector3;
};
template <>
struct potential_traits<imatrix4, 4>
{
  using type = ivector4;
};
template <>
struct potential_traits<bmatrix2, 2>
{
  using type = bvector2;
};
template <>
struct potential_traits<bmatrix3, 3>
{
  using type = bvector3;
};
template <>
struct potential_traits<bmatrix4, 4>
{
  using type = bvector4;
};
template <>
struct potential_traits<smatrix2, 2>
{
  using type = svector2;
};
template <>
struct potential_traits<smatrix3, 3>
{
  using type = svector3;
};
template <>
struct potential_traits<smatrix4, 4>
{
  using type = svector4;
};
}

#endif