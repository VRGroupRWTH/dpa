#ifndef DPA_STAGES_DOMAIN_PARTITIONER_HPP
#define DPA_STAGES_DOMAIN_PARTITIONER_HPP

#include <optional>

#include <boost/mpi/cartesian_communicator.hpp>
#include <boost/mpi/communicator.hpp>

#include <dpa/types/basic_types.hpp>

namespace dpa
{
class domain_partitioner
{
public:
  struct partition
  {
    integer  rank       = 0 ;
    ivector3 multi_rank = {};
    ivector3 offset     = {};
  };

  explicit domain_partitioner  ()                                = default;
  domain_partitioner           (const domain_partitioner&  that) = delete ;
  domain_partitioner           (      domain_partitioner&& temp) = default;
 ~domain_partitioner           ()                                = default;
  domain_partitioner& operator=(const domain_partitioner&  that) = delete ;
  domain_partitioner& operator=(      domain_partitioner&& temp) = default;

  void                                set_domain_size       (const ivector3& domain_size);
  
  boost::mpi::communicator*           communicator          ();
  boost::mpi::cartesian_communicator* cartesian_communicator();
  const ivector3&                     domain_size           () const;
  const ivector3&                     grid_size             () const;
  const ivector3&                     block_size            () const;

  const partition&                    local_partition       () const;
  const std::optional<partition>&     positive_x_partition  () const;
  const std::optional<partition>&     negative_x_partition  () const;
  const std::optional<partition>&     positive_y_partition  () const;
  const std::optional<partition>&     negative_y_partition  () const;
  const std::optional<partition>&     positive_z_partition  () const;
  const std::optional<partition>&     negative_z_partition  () const;

protected:
  partition                           setup_partition       (integer rank) const;

  boost::mpi::communicator                            communicator_           ;
  std::unique_ptr<boost::mpi::cartesian_communicator> cartesian_communicator_ = nullptr;
  ivector3                                            domain_size_            = {};
  ivector3                                            grid_size_              = {};
  ivector3                                            block_size_             = {};

  partition                                           local_partition_        = {};
  std::optional<partition>                            positive_x_partition_   = {};
  std::optional<partition>                            negative_x_partition_   = {};
  std::optional<partition>                            positive_y_partition_   = {};
  std::optional<partition>                            negative_y_partition_   = {};
  std::optional<partition>                            positive_z_partition_   = {};
  std::optional<partition>                            negative_z_partition_   = {};
};
}

#endif