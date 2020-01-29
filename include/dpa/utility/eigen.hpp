#ifndef DPA_UTILITY_EIGEN_HPP
#define DPA_UTILITY_EIGEN_HPP

#include <Eigen/Core>

namespace std
{
template<typename scalar_type, int row_count, int column_count, int options, int max_rows, int max_cols>
struct tuple_size<Eigen::Matrix<scalar_type, row_count, column_count, options, max_rows, max_cols>>
{
  static_assert(row_count != Eigen::Dynamic && column_count != Eigen::Dynamic, "tuple_size is only supported for fixed size matrices.");
  static const size_t value = row_count * column_count;
};
}

#endif