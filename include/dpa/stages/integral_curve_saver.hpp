#ifndef DPA_STAGES_INTEGRAL_CURVE_SAVER_HPP
#define DPA_STAGES_INTEGRAL_CURVE_SAVER_HPP

#include <string>

#include <dpa/stages/domain_partitioner.hpp>
#include <dpa/types/integral_curves.hpp>

namespace dpa
{
class integral_curve_saver
{
public:
  explicit integral_curve_saver  (domain_partitioner* partitioner, const std::string& filepath);
  integral_curve_saver           (const integral_curve_saver&  that) = delete ;
  integral_curve_saver           (      integral_curve_saver&& temp) = default;
 ~integral_curve_saver           ()                                  = default;
  integral_curve_saver& operator=(const integral_curve_saver&  that) = delete ;
  integral_curve_saver& operator=(      integral_curve_saver&& temp) = default;

  void save(const integral_curves& integral_curves);

protected:
  domain_partitioner* partitioner_ = {};
  std::string         filepath_    = {};
};
}

#endif