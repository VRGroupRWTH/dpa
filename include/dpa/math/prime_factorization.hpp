#ifndef DPA_MATH_PRIME_FACTORIZATION_HPP
#define DPA_MATH_PRIME_FACTORIZATION_HPP

#include <cmath>
#include <vector>

namespace dpa
{
template<typename type>
std::vector<type> prime_factorize(type value)
{
  std::vector<type> prime_factors;
  
  type denominator(2);
  while (std::pow(denominator, type(2)) <= value)
  {
    if (value % denominator == type(0))
    {
      prime_factors.push_back(denominator);
      value /= denominator;
    }
    else
      ++denominator;
  }

  if (value > type(1))
    prime_factors.push_back(value);

  return prime_factors;
}
}

#endif