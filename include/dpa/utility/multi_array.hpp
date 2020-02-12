#ifndef DPA_UTILITY_MULTI_ARRAY_HPP
#define DPA_UTILITY_MULTI_ARRAY_HPP

#include <boost/type.hpp>

namespace dpa
{
template<typename type, typename function_type, size_t dimensions = type::dimensionality>
struct iterate_internal
{
  void operator() (type& array, const function_type& function) const
  {
    for (auto element : array)
      iterate_internal<decltype(element), function_type>()(element, function);
  }
};
template<typename type, typename function_type>
struct iterate_internal<type, function_type, 1>
{
  void operator()(type& array, const function_type& function) const
  {
    for (auto& element : array)
      function(element);
  }
};
template<typename type, typename function_type>
static void iterate(type& array, const function_type& function)
{
  iterate_internal<type, function_type>() (array, function);
}
}

#endif // FOR_EACH_HPP