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
    collection_size_type count        = 0;
    item_version_type    item_version = 0;
    ar >> BOOST_SERIALIZATION_NVP(count);
    if (boost::archive::library_version_type(3) < ar.get_library_version())
      ar >> BOOST_SERIALIZATION_NVP(item_version);

    stl::collection_load_impl(ar, v, count, item_version);
  }
  template<class Archive, class T, class A>
  void serialize(Archive& ar,       tbb::concurrent_vector<T, A>& v, const unsigned int version)
  {
    split_free(ar, v, version);
  }
}
}

#endif