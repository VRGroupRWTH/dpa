#ifndef DPA_STAGES_ARGUMENT_PARSER_HPP
#define DPA_STAGES_ARGUMENT_PARSER_HPP

#include <string>

#include <dpa/types/arguments.hpp>

namespace dpa
{
class argument_parser
{
public:
  static arguments parse(const std::string& filepath);
};
}

#endif