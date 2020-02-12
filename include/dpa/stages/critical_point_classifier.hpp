#ifndef DPA_STAGES_CRITICAL_POINT_CLASSIFIER_HPP
#define DPA_STAGES_CRITICAL_POINT_CLASSIFIER_HPP

#include <limits>

#include <dpa/types/point_cloud.hpp>
#include <dpa/types/regular_fields.hpp>

namespace dpa
{
class critical_point_classifier
{
public:
  static point_cloud classify(const regular_vector_field_3d& vector_field, svector3 resolution, scalar epsilon = std::numeric_limits<scalar>::epsilon());
};
}

#endif