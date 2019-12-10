#ifndef DPA_MATH_LINEAR_INTERPOLATION_HPP
#define DPA_MATH_LINEAR_INTERPOLATION_HPP

namespace dpa
{
template <typename data_type, typename weight_type>
data_type linear_interpolate(const data_type& x, const data_type& y, const weight_type weight)
{
  return (weight_type(1) - weight) * x + weight * y;
}
}

#endif