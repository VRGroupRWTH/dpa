#ifndef DPA_MATH_PERMUTE_FOR_HPP
#define DPA_MATH_PERMUTE_FOR_HPP

#include <cstddef>
#include <functional>

#include <tbb/tbb.h>

namespace dpa
{
// Ducks [] and .size() on the type.
template <typename type>
void permute_for(
  const std::function<void(const type&)>& function, 
  const type&                             start   ,
  const type&                             end     ,
  const type&                             step    )
{
  std::function<void(type, std::size_t)> permute_for_internal =
    [&] (type indices, std::size_t depth)
    {
      if (depth < start.size())
      {
        for (auto i = start[depth]; i < end[depth]; i += step[depth])
        {
          indices[depth] = i;
          permute_for_internal(indices, depth + 1);
        }
      }
      else
        function(indices);
    };
  permute_for_internal(type(), 0);
}

// Ducks [] and .size() on the type.
template <typename type>
void parallel_permute_for(
  const std::function<void(const type&)>& function, 
  const type&                             start   ,
  const type&                             end     ,
  const type&                             step    )
{
  std::function<void(type, std::size_t)> permute_for_internal =
    [&] (type indices, std::size_t depth)
    {
      if (depth < start.size())
        tbb::parallel_for(start[depth], end[depth], step[depth], [&] (const std::size_t index)
        {
          indices[depth] = index;
          permute_for_internal(indices, depth + 1);
        });
      else
        function(indices);
    };
  permute_for_internal(type(), 0);
}
}

#endif