#ifndef DPA_BASIC_TYPES_HPP
#define DPA_BASIC_TYPES_HPP

#define EIGEN_INITIALIZE_MATRICES_BY_ZERO

#include <cstdint>

#include <Eigen/Core>

namespace dpa
{
using scalar   = float;
using integer  = std::int32_t;
               
using vector2  = Eigen::Vector2f;
using vector3  = Eigen::Vector3f;
using vector4  = Eigen::Vector4f;
               
using ivector2 = Eigen::Vector2i;
using ivector3 = Eigen::Vector3i;
using ivector4 = Eigen::Vector4i;
               
using matrix2  = Eigen::Matrix2f;
using matrix3  = Eigen::Matrix3f;
using matrix4  = Eigen::Matrix4f;
               
using imatrix2 = Eigen::Matrix2f;
using imatrix3 = Eigen::Matrix3f;
using imatrix4 = Eigen::Matrix4f;

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
}

#endif