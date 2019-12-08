#include <dpa/stages/domain_partitioner.hpp>

#include <dpa/math/prime_factorization.hpp>

namespace dpa
{
void                                                domain_partitioner::set_domain_size       (const ivector3& domain_size)
{
  domain_size_ = domain_size;

  auto prime_factors = prime_factorize(communicator_.size());
  auto current_size  = domain_size_;
  grid_size_.setConstant(1);
  while (!prime_factors.empty())
  {
    auto dimension = 0;
    for (auto i = 0; i < 3; ++i)
      if (current_size[i] > current_size[dimension])
        dimension = i;

    current_size[dimension] /= prime_factors.back();
    grid_size_  [dimension] *= prime_factors.back();
    prime_factors.pop_back();
  }
  
  block_size_             = domain_size_.array() / grid_size_.array();

  cartesian_communicator_ = std::make_unique<boost::mpi::cartesian_communicator>(communicator_, boost::mpi::cartesian_topology(std::vector<boost::mpi::cartesian_dimension>
  {
    boost::mpi::cartesian_dimension(grid_size_[0]),
    boost::mpi::cartesian_dimension(grid_size_[1]),
    boost::mpi::cartesian_dimension(grid_size_[2])
  }));

  local_partition_ = setup_partition(cartesian_communicator_->rank());

  const auto shift_x = cartesian_communicator_->shifted_ranks(0, 1);
  const auto shift_y = cartesian_communicator_->shifted_ranks(1, 1);
  const auto shift_z = cartesian_communicator_->shifted_ranks(2, 1);
  shift_x.first  != MPI_PROC_NULL ? negative_x_partition_ = setup_partition(shift_x.first ) : negative_x_partition_.reset();
  shift_x.second != MPI_PROC_NULL ? positive_x_partition_ = setup_partition(shift_x.second) : positive_x_partition_.reset();
  shift_y.first  != MPI_PROC_NULL ? negative_y_partition_ = setup_partition(shift_y.first ) : negative_y_partition_.reset();
  shift_y.second != MPI_PROC_NULL ? positive_y_partition_ = setup_partition(shift_y.second) : positive_y_partition_.reset();
  shift_z.first  != MPI_PROC_NULL ? negative_z_partition_ = setup_partition(shift_z.first ) : negative_z_partition_.reset();
  shift_z.second != MPI_PROC_NULL ? positive_z_partition_ = setup_partition(shift_z.second) : positive_z_partition_.reset();
}

boost::mpi::communicator*                           domain_partitioner::communicator          ()
{                                    
  return &communicator_;              
}
boost::mpi::cartesian_communicator*                 domain_partitioner::cartesian_communicator()
{
  return cartesian_communicator_.get();
}
const ivector3&                                     domain_partitioner::domain_size           () const
{                                    
  return domain_size_;               
}                                    
const ivector3&                                     domain_partitioner::grid_size             () const
{                                    
  return grid_size_;                 
}                                    
const ivector3&                                     domain_partitioner::block_size            () const
{                                    
  return block_size_;
}

const domain_partitioner::partition&                domain_partitioner::local_partition       () const
{
  return local_partition_;
}
const std::optional<domain_partitioner::partition>& domain_partitioner::positive_x_partition  () const
{
  return positive_x_partition_;
}
const std::optional<domain_partitioner::partition>& domain_partitioner::negative_x_partition  () const
{
  return negative_x_partition_;
}
const std::optional<domain_partitioner::partition>& domain_partitioner::positive_y_partition  () const
{
  return positive_y_partition_;
}
const std::optional<domain_partitioner::partition>& domain_partitioner::negative_y_partition  () const
{
  return negative_y_partition_;
}
const std::optional<domain_partitioner::partition>& domain_partitioner::positive_z_partition  () const
{
  return positive_z_partition_;
}
const std::optional<domain_partitioner::partition>& domain_partitioner::negative_z_partition  () const
{
  return negative_y_partition_;
}
  
domain_partitioner::partition                       domain_partitioner::setup_partition       (integer rank) const
{
  const auto raw_multi_rank = cartesian_communicator_->coordinates(rank);
  const auto multi_rank     = ivector3(raw_multi_rank[0], raw_multi_rank[1], raw_multi_rank[2]);
  const auto offset         = block_size_.array() * multi_rank.array();
  return partition {rank, multi_rank, offset};
}
}
