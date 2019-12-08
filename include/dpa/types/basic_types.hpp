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
}

#endif