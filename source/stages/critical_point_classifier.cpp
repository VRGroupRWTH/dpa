#include <dpa/stages/critical_point_classifier.hpp>

#include <Eigen/Dense>

namespace dpa
{
point_cloud critical_point_classifier::classify(const regular_vector_field_3d& vector_field, svector3 resolution, scalar epsilon)
{
  point_cloud point_cloud {std::vector<vector3>(), std::vector<scalar>()};
  auto& vertices = point_cloud.vertices;
  auto& colors   = std::get<std::vector<scalar>>(point_cloud.colors);

  const auto start_index = svector3::Zero();
  const auto end_index   = reinterpret_cast<const svector3&>(*vector_field.data.shape()).array() * resolution.array();
  const auto increment   = svector3::Ones();
  const auto spacing     = vector_field.spacing.array() / resolution.cast<scalar>().array();
  tbb::mutex mutex;
  parallel_permute_for<svector3>([&] (const svector3& index)
  {
    const vector3 point = vector_field.offset.array() + spacing.array() * index.cast<scalar>().array();
    if (!vector_field.contains(point) || vector_field.interpolate(point).norm() > epsilon)
      return;

    matrix3 gradient;
    for (std::size_t dimension = 0; dimension < 3; ++dimension)
    {
      auto prev_index = index, next_index = index;
      if (index[dimension] > 0)                        prev_index[dimension] -= 1;
      if (index[dimension] < end_index[dimension] - 1) next_index[dimension] += 1;
    
      const auto prev_point = vector_field.offset.array() + spacing.array() * prev_index.cast<scalar>().array();
      const auto next_point = vector_field.offset.array() + spacing.array() * next_index.cast<scalar>().array();
      
      gradient.col(dimension).array() = (vector_field.interpolate(next_point) - vector_field.interpolate(prev_point)) / (2 * spacing[dimension]);
    }

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

      tbb::mutex::scoped_lock lock(mutex);
      vertices.push_back(point);
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

      tbb::mutex::scoped_lock lock(mutex);
      if (positive_real_part_count == 2 || negative_real_part_count == 2) 
        vertices.push_back(point);
      if ( real_positive && positive_real_part_count == 2) colors.push_back(4); // Spiral.
      if ( real_positive && negative_real_part_count == 2) colors.push_back(5); // 2/1 spiral.
      if (!real_positive && positive_real_part_count == 2) colors.push_back(6); // 1/2 spiral.
      if (!real_positive && negative_real_part_count == 2) colors.push_back(7); // Spiral sink.
    }
  }, start_index, end_index, increment);

  return point_cloud;
}
}
