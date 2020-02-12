#include <dpa/stages/critical_point_classifier.hpp>

#include <Eigen/Dense>

namespace dpa
{
point_cloud critical_point_classifier::classify(const regular_vector_field_3d& original_vector_field)
{
  point_cloud point_cloud {std::vector<vector3>(), std::vector<scalar>()};
  auto vertices = point_cloud.vertices;
  auto colors   = std::get<std::vector<scalar>>(point_cloud.colors);

  vector3 critical_point;
  // TODO: Find points which evaluate to the eps-vector.

  matrix3 gradient;
  // TODO: Compute gradient at the point.

  auto eigenvalues = gradient.eigenvalues();

  auto real_count = 0;
  for (auto i = 0; i < eigenvalues.size(); ++i)
    if (eigenvalues[i].imag() == scalar(0))
      real_count++;

  if (real_count == 3)
  {
    auto positive_count = 0;
    for (auto i = 0; i < eigenvalues.size(); ++i)
      if (eigenvalues[i].real() >= scalar(0))
        positive_count++;

    vertices.push_back(critical_point);
    if (positive_count == 3) colors.push_back(0); // Source.
    if (positive_count == 2) colors.push_back(1); // 1/2 saddle.
    if (positive_count == 1) colors.push_back(2); // 2/1 saddle.
    if (positive_count == 0) colors.push_back(3); // Sink.
  }
  if (real_count == 1)
  {
    auto real_positive            = false;
    auto positive_real_part_count = 0;
    auto negative_real_part_count = 0;
    for (auto i = 0; i < eigenvalues.size(); ++i)
    {
      if (eigenvalues[i].imag() == scalar(0)) // is real.
      {
        if (eigenvalues[i].real() >= scalar(0))
          real_positive = true;
      }
      else // is complex.
        (eigenvalues[i].real() >= scalar(0) ? positive_real_part_count : negative_real_part_count)++;
    }

    if (positive_real_part_count == 2 || negative_real_part_count == 2) 
      vertices.push_back(critical_point);
    if ( real_positive && positive_real_part_count == 2) colors.push_back(4); // Spiral.
    if ( real_positive && negative_real_part_count == 2) colors.push_back(5); // 2/1 spiral.
    if (!real_positive && positive_real_part_count == 2) colors.push_back(6); // 1/2 spiral.
    if (!real_positive && negative_real_part_count == 2) colors.push_back(7); // Spiral sink.
  }

  return point_cloud;
}
}
