#ifndef MPI_DETACH_BOOST_EXTENSIONS_HPP
#define MPI_DETACH_BOOST_EXTENSIONS_HPP

#define BOOST_THREAD_VERSION 5

#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/beast/core/span.hpp>
#include <boost/mpi.hpp>

#include <adept/detach/detach.hpp>

namespace boost::mpi
{
using beast::span; // An independent std::span equivalent is not available in boost yet, hence we utilize beast's implementation.

// Callback interface with unified call syntax for request. TODO: Implement for non-trivials.
inline void detach            (request&        request , const std::function<void()>&                                      callback)
{
  struct state_type
  {
    std::function<void()> callback;
  };

  if (request.trivial()) 
    MPIX_Detach(&request.trivial().get(), [ ] (void* user_data)
    {
      auto*  state = static_cast<state_type*>(user_data);
      state->callback();
      delete state;
    }, new state_type {callback}); // MPI_Detach may complete past the scope of the callback, hence we need to re-instantiate the callback and couple it to the lifetime of MPI_Detach.
}
template <typename type>
void        detach            (request&        request , const std::function<void(type&)>&                                 callback, const type&       data)
{
  struct state_type
  {
    std::function<void(type&)> callback;
    type                       data    ;
  };

  if (request.trivial()) 
    MPIX_Detach(&request.trivial().get(), [ ] (void* user_data)
    {
      auto*  state = static_cast<state_type*>(user_data);
      state->callback(state->data);
      delete state;
    }, new state_type {callback, data});
}

inline void detach_status     (request&        request , const std::function<void(       const MPI_Status&)>&              callback)
{
  struct state_type
  {
    std::function<void(const MPI_Status&)> callback;
  };

  if (request.trivial()) 
    MPIX_Detach_status(&request.trivial().get(), [ ] (void* user_data, MPI_Status* status)
    {
      auto*  state = static_cast<state_type*>(user_data);
      state->callback(*status);
      delete state;
    }, new state_type {callback});
}
template <typename type>
void        detach_status     (request&        request , const std::function<void(type&, const MPI_Status&)>&              callback, const type&       data)
{
  struct state_type
  {
    std::function<void(type&, const MPI_Status&)> callback;
    type                                          data    ;
  };

  if (request.trivial()) 
    MPIX_Detach_status(&request.trivial().get(), [ ] (void* user_data, MPI_Status* status)
    {
      auto*  state = static_cast<state_type*>(user_data);
      state->callback(state->data, *status);
      delete state;
    }, new state_type {callback, data});
}

inline void detach_each       (span<request&>& requests, const std::function<void()>&                                      callback)
{
  struct state_type
  {
    std::function<void()> callback;
  };

  std::vector<MPI_Request> native_requests(requests.size(), MPI_REQUEST_NULL);
  std::vector<state_type*> user_data      (requests.size(), nullptr);
  std::vector<void*>       raw_user_data  (requests.size(), nullptr);
  for (auto i = 0; i < requests.size(); ++i)
  {
    if (requests.data()[i].trivial())
    {
      native_requests[i] = requests.data()[i].trivial().get();
      user_data      [i] = new state_type {callback};
      raw_user_data  [i] = user_data[i];
    }
  }

  MPIX_Detach_each(native_requests.size(), native_requests.data(), [ ] (void* user_data)
  {
    auto*  state = static_cast<state_type*>(user_data);
    state->callback();
    delete state;
  }, raw_user_data.data());
}
template <typename type>
void        detach_each       (span<request&>& requests, const std::function<void(type&)>&                                 callback, const span<type>& data)
{
  struct state_type
  {
    std::function<void(type&)> callback;
    type                       data    ;
  };

  std::vector<MPI_Request> native_requests(requests.size(), MPI_REQUEST_NULL);
  std::vector<state_type*> user_data      (requests.size(), nullptr);
  std::vector<void*>       raw_user_data  (requests.size(), nullptr);
  for (auto i = 0; i < requests.size(); ++i)
  {
    if (requests.data()[i].trivial())
    {
      native_requests[i] = requests.data()[i].trivial().get();
      user_data      [i] = new state_type {callback, data.data_[i]};
      raw_user_data  [i] = user_data[i];
    }
  }

  MPIX_Detach_each(native_requests.size(), native_requests.data(), [ ] (void* user_data)
  {
    auto*  state = static_cast<state_type*>(user_data);
    state->callback(state->data);
    delete state;
  }, raw_user_data.data());
}

inline void detach_each_status(span<request&>& requests, const std::function<void(       const MPI_Status&)>&              callback)
{
  struct state_type
  {
    std::function<void(const MPI_Status&)> callback;
  };

  std::vector<MPI_Request> native_requests(requests.size(), MPI_REQUEST_NULL);
  std::vector<state_type*> user_data      (requests.size(), nullptr);
  std::vector<void*>       raw_user_data  (requests.size(), nullptr);
  for (auto i = 0; i < requests.size(); ++i)
  {
    if (requests.data()[i].trivial())
    {
      native_requests[i] = requests.data()[i].trivial().get();
      user_data      [i] = new state_type {callback};
      raw_user_data  [i] = user_data[i];
    }
  }

  MPIX_Detach_each_status(native_requests.size(), native_requests.data(), [ ] (void* user_data, MPI_Status* status)
  {
    auto*  state = static_cast<state_type*>(user_data);
    state->callback(*status);
    delete state;
  }, raw_user_data.data());
}
template <typename type>
void        detach_each_status(span<request&>& requests, const std::function<void(type&, const MPI_Status&)>&              callback, const span<type>& data)
{
  struct state_type
  {
    std::function<void(type&, const MPI_Status&)> callback;
    type                                          data    ;
  };

  std::vector<MPI_Request> native_requests(requests.size(), MPI_REQUEST_NULL);
  std::vector<state_type*> user_data      (requests.size(), nullptr);
  std::vector<void*>       raw_user_data  (requests.size(), nullptr);
  for (auto i = 0; i < requests.size(); ++i)
  {
    if (requests.data()[i].trivial())
    {
      native_requests[i] = requests.data()[i].trivial().get();
      user_data      [i] = new state_type {callback, data.data_[i]};
      raw_user_data  [i] = user_data[i];
    }
  }

  MPIX_Detach_each_status(native_requests.size(), native_requests.data(), [ ] (void* user_data, MPI_Status* status)
  {
    auto*  state = static_cast<state_type*>(user_data);
    state->callback(state->data, *status);
    delete state;
  }, raw_user_data.data());
}

inline void detach_all        (span<request&>& requests, const std::function<void()>&                                      callback)
{
  struct state_type
  {
    std::function<void()> callback;
  };

  std::vector<MPI_Request> native_requests(requests.size(), MPI_REQUEST_NULL);
  for (auto i = 0; i < requests.size(); ++i)
    if (requests.data()[i].trivial())
      native_requests[i] = requests.data()[i].trivial().get();

  MPIX_Detach_all(native_requests.size(), native_requests.data(), [ ] (void* user_data)
  {
    auto*  state = static_cast<state_type*>(user_data);
    state->callback();
    delete state;
  }, new state_type {callback});
}
template <typename type>
void        detach_all        (span<request&>& requests, const std::function<void(type&)>&                                 callback, const type&       data)
{
  struct state_type
  {
    std::function<void(type&)> callback;
    type                       data    ;
  };

  std::vector<MPI_Request> native_requests(requests.size(), MPI_REQUEST_NULL);
  for (auto i = 0; i < requests.size(); ++i)
    if (requests.data()[i].trivial())
      native_requests[i] = requests.data()[i].trivial().get();

  MPIX_Detach_all(native_requests.size(), native_requests.data(), [ ] (void* user_data)
  {
    auto*  state = static_cast<state_type*>(user_data);
    state->callback(state->data);
    delete state;
  }, new state_type {callback, data});
}

inline void detach_all_status (span<request&>& requests, const std::function<void(       const std::vector<MPI_Status>&)>& callback)
{
  struct state_type
  {
    std::function<void(const std::vector<MPI_Status>&)> callback;
  };

  std::vector<MPI_Request> native_requests(requests.size(), MPI_REQUEST_NULL);
  for (auto i = 0; i < requests.size(); ++i)
    if (requests.data()[i].trivial())
      native_requests[i] = requests.data()[i].trivial().get();

  MPIX_Detach_all_status(native_requests.size(), native_requests.data(), [ ] (void* user_data, std::int32_t count, MPI_Status* status)
  {
    std::vector<MPI_Status> stati(count);
    for (auto i = 0; i < count; ++i)
      stati[i] = status[i];

    auto*  state = static_cast<state_type*>(user_data);
    state->callback(stati);
    delete state;
  }, new state_type {callback});
}
template <typename type>
void        detach_all_status (span<request&>& requests, const std::function<void(type&, const std::vector<MPI_Status>&)>& callback, const type&       data)
{
  struct state_type
  {
    std::function<void(type&, const std::vector<MPI_Status>&)> callback;
    type                                                       data    ;
  };

  std::vector<MPI_Request> native_requests(requests.size(), MPI_REQUEST_NULL);
  for (auto i = 0; i < requests.size(); ++i)
    if (requests.data()[i].trivial())
      native_requests[i] = requests.data()[i].trivial().get();

  MPIX_Detach_all_status(native_requests.size(), native_requests.data(), [ ] (void* user_data, std::int32_t count, MPI_Status* status)
  {
    std::vector<MPI_Status> stati(count);
    for (auto i = 0; i < count; ++i)
      stati[i] = status[i];

    auto*  state = static_cast<state_type*>(user_data);
    state->callback(state->data, stati);
    delete state;
  }, new state_type {callback, data});
}

// Extension function imitators for request.
inline void operator/(request& request, const std::function<void()>&                  callback)
{
  detach       (request, callback);
}
inline void operator/(request& request, const std::function<void(const MPI_Status&)>& callback)
{
  detach_status(request, callback);
}

// Convenience for isend/irecv with unified call syntax for communicator.
inline void isend(const communicator& communicator, std::int32_t destination, std::int32_t tag,                                                        const std::function<void()>&                                   callback)
{
  auto request = communicator.isend(destination, tag);
  detach(request, callback);
}
template <typename type>                                                                                                                                                                                              
void        isend(const communicator& communicator, std::int32_t destination, std::int32_t tag, const type&                 value,                     const std::function<void()>&                                   callback)
{
  auto request = communicator.isend(destination, tag, value);
  detach(request, callback);
}
template <typename type>                                                                                                                                                                                              
void        isend(const communicator& communicator, std::int32_t destination, std::int32_t tag, const skeleton_proxy<type>& value,                     const std::function<void()>&                                   callback)
{
  auto request = communicator.isend(destination, tag, value);
  detach(request, callback);
}
template <typename type>                                                                                                                                                                                              
void        isend(const communicator& communicator, std::int32_t destination, std::int32_t tag, const type*                 value, std::int32_t count, const std::function<void()>&                                   callback)
{
  auto request = communicator.isend(destination, tag, value, count);
  detach(request, callback);
}
template <typename type>                                                                                                                                                                                              
void        isend(const communicator& communicator, std::int32_t destination, std::int32_t tag, const std::vector<type>&    value,                     const std::function<void()>&                                   callback)
{
  auto request = communicator.isend(destination, tag, value);
  detach(request, callback);
}

template <typename user_data_type>
void        isend(const communicator& communicator, std::int32_t destination, std::int32_t tag,                                                        const std::function<void(user_data_type&)>&                    callback, const user_data_type& user_data)
{
  auto request = communicator.isend(destination, tag);
  detach<user_data_type>(request, callback, user_data);
}
template <typename type, typename user_data_type>
void        isend(const communicator& communicator, std::int32_t destination, std::int32_t tag, const type&                 value,                     const std::function<void(user_data_type&)>&                    callback, const user_data_type& user_data)
{
  auto request = communicator.isend(destination, tag, value);
  detach<user_data_type>(request, callback, user_data);
}
template <typename type, typename user_data_type>
void        isend(const communicator& communicator, std::int32_t destination, std::int32_t tag, const skeleton_proxy<type>& value,                     const std::function<void(user_data_type&)>&                    callback, const user_data_type& user_data)
{
  auto request = communicator.isend(destination, tag, value);
  detach<user_data_type>(request, callback, user_data);
}
template <typename type, typename user_data_type>
void        isend(const communicator& communicator, std::int32_t destination, std::int32_t tag, const type*                 value, std::int32_t count, const std::function<void(user_data_type&)>&                    callback, const user_data_type& user_data)
{
  auto request = communicator.isend(destination, tag, value, count);
  detach<user_data_type>(request, callback, user_data);
}
template <typename type, typename user_data_type>
void        isend(const communicator& communicator, std::int32_t destination, std::int32_t tag, const std::vector<type>&    value,                     const std::function<void(user_data_type&)>&                    callback, const user_data_type& user_data)
{
  auto request = communicator.isend(destination, tag, value);
  detach<user_data_type>(request, callback, user_data);
}

inline void isend(const communicator& communicator, std::int32_t destination, std::int32_t tag,                                                        const std::function<void(const MPI_Status&)>&                  callback)
{
  auto request = communicator.isend(destination, tag);
  detach_status(request, callback);
}
template <typename type>
void        isend(const communicator& communicator, std::int32_t destination, std::int32_t tag, const type&                 value,                     const std::function<void(const MPI_Status&)>&                  callback)
{
  auto request = communicator.isend(destination, tag, value);
  detach_status(request, callback);
}
template <typename type>
void        isend(const communicator& communicator, std::int32_t destination, std::int32_t tag, const skeleton_proxy<type>& value,                     const std::function<void(const MPI_Status&)>&                  callback)
{
  auto request = communicator.isend(destination, tag, value);
  detach_status(request, callback);
}
template <typename type>
void        isend(const communicator& communicator, std::int32_t destination, std::int32_t tag, const type*                 value, std::int32_t count, const std::function<void(const MPI_Status&)>&                  callback)
{
  auto request = communicator.isend(destination, tag, value, count);
  detach_status(request, callback);
}
template <typename type>
void        isend(const communicator& communicator, std::int32_t destination, std::int32_t tag, const std::vector<type>&    value,                     const std::function<void(const MPI_Status&)>&                  callback)
{
  auto request = communicator.isend(destination, tag, value);
  detach_status(request, callback);
}

template <typename user_data_type>
void        isend(const communicator& communicator, std::int32_t destination, std::int32_t tag,                                                        const std::function<void(user_data_type&, const MPI_Status&)>& callback, const user_data_type& user_data)
{
  auto request = communicator.isend(destination, tag);
  detach_status<user_data_type>(request, callback, user_data);
}
template <typename type, typename user_data_type>
void        isend(const communicator& communicator, std::int32_t destination, std::int32_t tag, const type&                 value,                     const std::function<void(user_data_type&, const MPI_Status&)>& callback, const user_data_type& user_data)
{
  auto request = communicator.isend(destination, tag, value);
  detach_status<user_data_type>(request, callback, user_data);
}
template <typename type, typename user_data_type>
void        isend(const communicator& communicator, std::int32_t destination, std::int32_t tag, const skeleton_proxy<type>& value,                     const std::function<void(user_data_type&, const MPI_Status&)>& callback, const user_data_type& user_data)
{
  auto request = communicator.isend(destination, tag, value);
  detach_status<user_data_type>(request, callback, user_data);
}
template <typename type, typename user_data_type>
void        isend(const communicator& communicator, std::int32_t destination, std::int32_t tag, const type*                 value, std::int32_t count, const std::function<void(user_data_type&, const MPI_Status&)>& callback, const user_data_type& user_data)
{
  auto request = communicator.isend(destination, tag, value, count);
  detach_status<user_data_type>(request, callback, user_data);
}
template <typename type, typename user_data_type>
void        isend(const communicator& communicator, std::int32_t destination, std::int32_t tag, const std::vector<type>&    value,                     const std::function<void(user_data_type&, const MPI_Status&)>& callback, const user_data_type& user_data)
{
  auto request = communicator.isend(destination, tag, value);
  detach_status<user_data_type>(request, callback, user_data);
}

inline void irecv(const communicator& communicator, std::int32_t source     , std::int32_t tag,                                                        const std::function<void()>&                                   callback)
{
  auto request = communicator.irecv(source, tag);
  detach(request, callback);
}
template <typename type>                                                                                                                                                                                              
void        irecv(const communicator& communicator, std::int32_t source     , std::int32_t tag,       type&                 value,                     const std::function<void()>&                                   callback)
{                                                                                                     
  auto request = communicator.irecv(source, tag, value);                                         
  detach(request, callback);                                                                          
}
template <typename type>                                                                                                                                                                                              
void        irecv(const communicator& communicator, std::int32_t source     , std::int32_t tag,       type*                 value, std::int32_t count, const std::function<void()>&                                   callback)
{                                                                                                     
  auto request = communicator.irecv(source, tag, value, count);                                  
  detach(request, callback);                                                                          
}
template <typename type>                                                                                                                                                                                              
void        irecv(const communicator& communicator, std::int32_t source     , std::int32_t tag,       std::vector<type>&    value,                     const std::function<void()>&                                   callback)
{
  auto request = communicator.irecv(source, tag, value);
  detach(request, callback);
}

template <typename user_data_type>
void        irecv(const communicator& communicator, std::int32_t source     , std::int32_t tag,                                                        const std::function<void(user_data_type&)>&                    callback, const user_data_type& user_data)
{
  auto request = communicator.irecv(source, tag);
  detach<user_data_type>(request, callback, user_data);
}
template <typename type, typename user_data_type>
void        irecv(const communicator& communicator, std::int32_t source     , std::int32_t tag,       type&                 value,                     const std::function<void(user_data_type&)>&                    callback, const user_data_type& user_data)
{                                                                                                     
  auto request = communicator.irecv(source, tag, value);                                         
  detach<user_data_type>(request, callback, user_data);
}
template <typename type, typename user_data_type>
void        irecv(const communicator& communicator, std::int32_t source     , std::int32_t tag,       type*                 value, std::int32_t count, const std::function<void(user_data_type&)>&                    callback, const user_data_type& user_data)
{                                                                                                     
  auto request = communicator.irecv(source, tag, value, count);                                  
  detach<user_data_type>(request, callback, user_data);
}
template <typename type, typename user_data_type>
void        irecv(const communicator& communicator, std::int32_t source     , std::int32_t tag,       std::vector<type>&    value,                     const std::function<void(user_data_type&)>&                    callback, const user_data_type& user_data)
{
  auto request = communicator.irecv(source, tag, value);
  detach<user_data_type>(request, callback, user_data);
}

inline void irecv(const communicator& communicator, std::int32_t source     , std::int32_t tag,                                                        const std::function<void(const MPI_Status&)>&                  callback)
{
  auto request = communicator.irecv(source, tag);
  detach_status(request, callback);
}
template <typename type>                                                                                                                                                                                              
void        irecv(const communicator& communicator, std::int32_t source     , std::int32_t tag,       type&                 value,                     const std::function<void(const MPI_Status&)>&                  callback)
{                                                                                                     
  auto request = communicator.irecv(source, tag, value);                                         
  detach_status(request, callback);
}
template <typename type>                                                                                                                                                                                              
void        irecv(const communicator& communicator, std::int32_t source     , std::int32_t tag,       type*                 value, std::int32_t count, const std::function<void(const MPI_Status&)>&                  callback)
{                                                                                                     
  auto request = communicator.irecv(source, tag, value, count);                                  
  detach_status(request, callback);
}
template <typename type>                                                                                                                                                                                              
void        irecv(const communicator& communicator, std::int32_t source     , std::int32_t tag,       std::vector<type>&    value,                     const std::function<void(const MPI_Status&)>&                  callback)
{
  auto request = communicator.irecv(source, tag, value);
  detach_status(request, callback);
}

template <typename user_data_type>
void        irecv(const communicator& communicator, std::int32_t source     , std::int32_t tag,                                                        const std::function<void(user_data_type&, const MPI_Status&)>& callback, const user_data_type& user_data)
{
  auto request = communicator.irecv(source, tag);
  detach_status<user_data_type>(request, callback, user_data);
}
template <typename type, typename user_data_type>
void        irecv(const communicator& communicator, std::int32_t source     , std::int32_t tag,       type&                 value,                     const std::function<void(user_data_type&, const MPI_Status&)>& callback, const user_data_type& user_data)
{                                                                                                     
  auto request = communicator.irecv(source, tag, value);
  detach_status<user_data_type>(request, callback, user_data);
}
template <typename type, typename user_data_type>
void        irecv(const communicator& communicator, std::int32_t source     , std::int32_t tag,       type*                 value, std::int32_t count, const std::function<void(user_data_type&, const MPI_Status&)>& callback, const user_data_type& user_data)
{                                                                                                     
  auto request = communicator.irecv(source, tag, value, count);
  detach_status<user_data_type>(request, callback, user_data);
}
template <typename type, typename user_data_type>
void        irecv(const communicator& communicator, std::int32_t source     , std::int32_t tag,       std::vector<type>&    value,                     const std::function<void(user_data_type&, const MPI_Status&)>& callback, const user_data_type& user_data)
{
  auto request = communicator.irecv(source, tag, value);
  detach_status<user_data_type>(request, callback, user_data);
}

// Future interface with unified call syntax for request.
inline future<void>                              detach            (request&        request )
{
  auto* promise = new boost::promise<void>();
  auto  future  = promise->get_future();
  detach(request, [promise] ()
  {
    promise->set_value();
    delete promise;
  });
  return std::move(future);
}
template <typename type>                         
future<type>                                     detach            (request&        request , const type&       data)
{
  auto* promise = new boost::promise<type>();
  auto  future  = promise->get_future();
  detach<type>(request, [promise] (type& data)
  {
    promise->set_value(data);
    delete promise;
  }, data);
  return std::move(future);
}

inline future<MPI_Status>                        detach_status     (request&        request )
{
  auto* promise = new boost::promise<MPI_Status>();
  auto  future  = promise->get_future();
  detach_status(request, [promise] (const MPI_Status& status)
  {
    promise->set_value(status);
    delete promise;
  });
  return std::move(future);
}
template <typename type>                         
future<std::pair<type, MPI_Status>>              detach_status     (request&        request , const type&       data)
{
  auto* promise = new boost::promise<std::pair<type, MPI_Status>>();
  auto  future  = promise->get_future();
  detach_status<type>(request, [promise] (type& data, const MPI_Status& status)
  {
    promise->set_value(std::make_pair(data, status));
    delete promise;
  }, data);
  return std::move(future);
}

inline future<void>                              detach_each       (span<request&>& requests)
{
  struct state_type
  {
    std::int32_t         counter;
    boost::promise<void> promise;
  };

  auto* state  = new state_type {requests.size()};
  auto  future = state->promise.get_future();
  detach_each(requests, [state] ()
  {
    state->promise.set_value();
    if (--state->counter == 0)
      delete state;
  });
  return std::move(future);
}
template <typename type>                         
future<type>                                     detach_each       (span<request&>& requests, const span<type>& data)
{
  struct state_type
  {
    std::int32_t         counter;
    boost::promise<type> promise;
  };

  auto* state  = new state_type {requests.size()};
  auto  future = state->promise.get_future();
  detach_each<type>(requests, [state] (type& data)
  {
    state->promise.set_value(data);
    if (--state->counter == 0)
      delete state;
  }, data);
  return std::move(future);
}

inline future<MPI_Status>                        detach_each_status(span<request&>& requests)
{
  struct state_type
  {
    std::int32_t               counter;
    boost::promise<MPI_Status> promise;
  };

  auto* state  = new state_type {requests.size()};
  auto  future = state->promise.get_future();
  detach_each_status(requests, [state] (const MPI_Status& status)
  {
    state->promise.set_value(status);
    if (--state->counter == 0)
      delete state;
  });
  return std::move(future);
}
template <typename type>                         
future<std::pair<type, MPI_Status>>              detach_each_status(span<request&>& requests, const span<type>& data)
{
  struct state_type
  { 
    std::int32_t                                counter;
    boost::promise<std::pair<type, MPI_Status>> promise;
  };

  auto* state  = new state_type {requests.size()};
  auto  future = state->promise.get_future();
  detach_each_status<type>(requests, [state] (type& data, const MPI_Status& status)
  {
    state->promise.set_value(std::make_pair(data, status));
    if (--state->counter == 0)
      delete state;
  }, data);
  return std::move(future);
}

inline future<void>                              detach_all        (span<request&>& requests)
{
  auto* promise = new boost::promise<void>();
  auto  future  = promise->get_future();
  detach_all(requests, [promise] ()
  {
    promise->set_value();
    delete promise;
  });
  return std::move(future);
}
template <typename type>                         
future<type>                                     detach_all        (span<request&>& requests, const type&       data)
{
  auto* promise = new boost::promise<type>();
  auto  future  = promise->get_future();
  detach_all<type>(requests, [promise] (type& data)
  {
    promise->set_value(data);
    delete promise;
  }, data);
  return std::move(future);
}

inline future<std::vector<MPI_Status>>           detach_all_status (span<request&>& requests)
{
  auto* promise = new boost::promise<std::vector<MPI_Status>>();
  auto  future  = promise->get_future();
  detach_all_status(requests, [promise] (const std::vector<MPI_Status>& stati)
  {
    promise->set_value(stati);
    delete promise;
  });
  return std::move(future);
}
template <typename type>               
future<std::pair<type, std::vector<MPI_Status>>> detach_all_status (span<request&>& requests, const type&       data)
{
  auto* promise = new boost::promise<std::pair<type, std::vector<MPI_Status>>>();
  auto  future  = promise->get_future();
  detach_all_status<type>(requests, [promise] (type& data, const std::vector<MPI_Status>& stati)
  {
    promise->set_value(std::make_pair(data, stati));
    delete promise;
  }, data);
  return std::move(future);
}

// Extension function imitators for request.
class future_type { };
inline future<void>                 operator| (request& request, future_type future)
{
  return detach(request);
}
template <typename type>
future<type>                        operator| (request& request, const type& data  )
{
  return detach<type>(request, data);
}

inline future<MPI_Status>           operator||(request& request, future_type future)
{
  return detach_status(request);
}
template <typename type>
future<std::pair<type, MPI_Status>> operator||(request& request, const type& data  )
{
  return detach_status<type>(request, data);
}

// ASIO detach service.
using namespace std::chrono_literals;

class detach_service
{
public:
  template <typename representation, typename period>
  explicit detach_service  (const std::size_t thread_count = 1, const std::chrono::duration<representation, period>& interval = 16ms)
  : work_guard_(make_work_guard(context_))
  , timer_     (context_, interval)
  {
    callback_ = [&, interval] (const boost::system::error_code& code)
    {
      MPIX_Progress(nullptr);

      // Periodicity.
      timer_.expires_at(timer_.expiry() + interval);
      timer_.async_wait(callback_); 
    };

    timer_.async_wait(callback_);

    for (auto i = 0; i < thread_count; ++i)
      threads_.create_thread([&] () { context_.run(); });
  }
  detach_service           (const detach_service&  that) = delete ;
  detach_service           (      detach_service&& temp) = delete ;
 ~detach_service           ()                            = default;
  detach_service& operator=(const detach_service&  that) = delete ;
  detach_service& operator=(      detach_service&& temp) = delete ;

protected:
  boost::thread_group                                        threads_   ;
  asio::io_context                                           context_   ;
  asio::executor_work_guard<asio::io_context::executor_type> work_guard_;
  asio::steady_timer                                         timer_     ;
  std::function<void(const boost::system::error_code&)>      callback_  ;
};
}

#endif