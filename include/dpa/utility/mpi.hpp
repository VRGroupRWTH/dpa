#ifndef DPA_UTILITY_MPI_HPP
#define DPA_UTILITY_MPI_HPP

#include <boost/mpi.hpp>

namespace boost
{
namespace mpi
{
// Patch to boost/mpi/request.hpp:40
// request make_trivial()
// {
//   return request(new request::trivial_handler());
// }

namespace detail
{
template<typename type>
request ibroadcast_impl         (const communicator& communicator,       type* values   , std::int32_t count,                   std::int32_t root, mpl::true_)
{
  auto request  = request::make_trivial();
  auto mpi_type = get_mpi_datatype<type>(*values);
  BOOST_MPI_CHECK_RESULT(MPI_Ibcast, (values, count, mpi_type, root, MPI_Comm(communicator), &request.trivial().get()));
  return request;
}
template<typename type>
request igather_impl            (const communicator& communicator, const type* in_values, std::int32_t count, type* out_values, std::int32_t root, mpl::true_)
{
  auto request  = request::make_trivial();
  auto mpi_type = get_mpi_datatype<type>(*in_values);
  BOOST_MPI_CHECK_RESULT(MPI_Igather, (in_values, count, mpi_type, out_values, count, mpi_type, root, MPI_Comm(communicator), &request.trivial().get()));
  return request;
}

template<typename type>
request ineighbor_alltoall_impl (const communicator& communicator, const type* in_values, std::int32_t count, type* out_values,                    mpl::true_)
{
  auto request  = request::make_trivial();
  auto mpi_type = get_mpi_datatype<type>(*in_values);
  BOOST_MPI_CHECK_RESULT(MPI_Ineighbor_alltoall, (in_values, count, mpi_type, out_values, count, mpi_type, MPI_Comm(communicator), &request.trivial().get()));
  return request;
}
template<typename type>
request ineighbor_alltoallv_impl(const communicator& communicator, const type* in_values, const std::int32_t* counts, const std::int32_t* displacements, type* out_values, mpl::true_)
{
  auto request  = request::make_trivial();
  auto mpi_type = get_mpi_datatype<type>(*in_values);
  BOOST_MPI_CHECK_RESULT(MPI_Ineighbor_alltoallv, (in_values, counts, displacements, mpi_type, out_values, counts, displacements, mpi_type, MPI_Comm(communicator), &request.trivial().get()));
  return request;
}
}

template<typename type>
request ibroadcast              (const communicator& communicator,       type& value    ,                                       std::int32_t root)
{
  return detail::ibroadcast_impl(communicator, &value, 1    , root, is_mpi_datatype<type>());
}
template<typename type>
request ibroadcast              (const communicator& communicator,       type* values   , std::int32_t count,                   std::int32_t root)
{
  return detail::ibroadcast_impl(communicator, values, count, root, is_mpi_datatype<type>());
} 
template<typename type>
request igather                 (const communicator& communicator, const type& in_value ,                     type* out_values, std::int32_t root)
{
  return detail::igather_impl(communicator, &in_value, 1    , out_values, root, is_mpi_datatype<type>());
}
template<typename type>
request igather                 (const communicator& communicator, const type* in_values, std::int32_t count, type* out_values, std::int32_t root)
{
  return detail::igather_impl(communicator, in_values, count, out_values, root, is_mpi_datatype<type>());
}

template<typename type>
request ineighbor_alltoall      (const communicator& communicator, const type* in_values, std::int32_t count, type* out_values)
{
  return detail::ineighbor_alltoall_impl(communicator, in_values, count, out_values, is_mpi_datatype<type>());
}
template<typename type>
request ineighbor_alltoallv     (const communicator& communicator, const type* in_values, const std::int32_t* counts, const std::int32_t* displacements, type* out_values)
{
  return detail::ineighbor_alltoallv_impl(communicator, in_values, counts, displacements, out_values, is_mpi_datatype<type>());
}
}
}

#endif