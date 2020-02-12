#ifndef DPA_STAGES_POINT_CLOUD_SAVER_HPP
#define DPA_STAGES_POINT_CLOUD_SAVER_HPP

#include <string>

#include <dpa/stages/domain_partitioner.hpp>
#include <dpa/types/point_cloud.hpp>

namespace dpa
{
class point_cloud_saver
{
public:
  explicit point_cloud_saver  (domain_partitioner* partitioner, const std::string& filepath);
  point_cloud_saver           (const point_cloud_saver&  that) = delete ;
  point_cloud_saver           (      point_cloud_saver&& temp) = default;
 ~point_cloud_saver           ()                               = default;
  point_cloud_saver& operator=(const point_cloud_saver&  that) = delete ;
  point_cloud_saver& operator=(      point_cloud_saver&& temp) = default;

  void save(const point_cloud& point_cloud);

protected:
  domain_partitioner* partitioner_ = {};
  std::string         filepath_    = {};
};
}

#endif