#ifndef DPA_UTILITY_SERIALIZATION_CONCURRENT_VECTOR_HPP
#define DPA_UTILITY_SERIALIZATION_CONCURRENT_VECTOR_HPP

#include <boost/serialization/split_free.hpp>
#include <boost/serialization/collections_load_imp.hpp>
#include <boost/serialization/collections_save_imp.hpp>
#include <tbb/tbb.h>

// Implementation based on dnn.
namespace boost
{
namespace serialization
{
  template<class Archive, class T, class A>
  void save     (Archive& ar, const tbb::concurrent_vector<T, A>& v, const unsigned int version)
  {
    stl::save_collection     (ar, v);
  }
  template<class Archive, class T, class A>
  void load     (Archive& ar,       tbb::concurrent_vector<T, A>& v, const unsigned int version)
  {
    stl::collection_load_impl(ar, v, collection_size_type(v.size()), item_version_type(version));
  }
  template<class Archive, class T, class A>
  void serialize(Archive& ar,       tbb::concurrent_vector<T, A>& v, const unsigned int version)
  {
    split_free(ar, v, version);
  }
}
}

#endif