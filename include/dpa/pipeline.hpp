#ifndef DPA_PIPELINE_HPP
#define DPA_PIPELINE_HPP

#include <cstdint>

namespace dpa
{
class pipeline
{
public:
  std::int32_t run(std::int32_t argc, char** argv);
};
}

#endif