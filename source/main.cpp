#include <cstdint>

#include <dpa/pipeline.hpp>

std::int32_t main(std::int32_t argc, char** argv)
{  
  return dpa::pipeline().run(argc, argv);
}