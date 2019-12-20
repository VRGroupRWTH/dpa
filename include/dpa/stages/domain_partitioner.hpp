#ifndef DPA_STAGES_DOMAIN_PARTITIONER_HPP
#define DPA_STAGES_DOMAIN_PARTITIONER_HPP

#include <memory>
#include <unordered_map>

#include <boost/mpi/cartesian_communicator.hpp>
#include <boost/mpi/communicator.hpp>

#include <dpa/types/basic_types.hpp>
#include <dpa/types/relative_direction.hpp>

namespace dpa
{
class domain_partitioner
{
public:
  struct partition
  {
    integer  rank               = 0 ;
    ivector3 multi_rank         = {};
    ivector3 offset             = {};
    ivector3 ghosted_offset     = {};
    ivector3 ghosted_block_size = {};
  };

  explicit domain_partitioner  ()                                = default;
  domain_partitioner           (const domain_partitioner&  that) = delete ;
  domain_partitioner           (      domain_partitioner&& temp) = default;
 ~domain_partitioner           ()                                = default;
  domain_partitioner& operator=(const domain_partitioner&  that) = delete ;
  domain_partitioner& operator=(      domain_partitioner&& temp) = default;

  void                                                     set_domain_size       (const ivector3& domain_size, const ivector3& ghost_cell_size);
  
  boost::mpi::communicator*                                communicator          ();
  boost::mpi::cartesian_communicator*                      cartesian_communicator();
  const ivector3&                                          domain_size           () const;
  const ivector3&                                          grid_size             () const;
  const ivector3&                                          block_size            () const;
  const std::unordered_map<relative_direction, partition>& partitions            () const;

protected:
  partition                                                setup_partition       (integer rank) const;

  boost::mpi::communicator                            communicator_           ;
  std::unique_ptr<boost::mpi::cartesian_communicator> cartesian_communicator_ = nullptr;
  ivector3                                            domain_size_            = {};
  ivector3                                            ghost_cell_size_        = {};
  ivector3                                            grid_size_              = {};
  ivector3                                            block_size_             = {};
  std::unordered_map<relative_direction, partition>   partitions_             = {};
};
}

#endif