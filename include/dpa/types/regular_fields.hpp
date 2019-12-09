#ifndef DPA_TYPES_REGULAR_FIELDS_HPP
#define DPA_TYPES_REGULAR_FIELDS_HPP

#include <dpa/types/basic_types.hpp>
#include <dpa/types/regular_grid.hpp>

namespace dpa
{
using regular_scalar_field_2d              = regular_grid<scalar , 2>;
using regular_scalar_field_3d              = regular_grid<scalar , 3>;
using regular_scalar_field_4d              = regular_grid<scalar , 4>;

using regular_vector_field_2d              = regular_grid<vector2, 2>;
using regular_vector_field_3d              = regular_grid<vector3, 3>;
using regular_vector_field_4d              = regular_grid<vector4, 4>;

using regular_matrix_field_2d              = regular_grid<matrix2, 2>;
using regular_matrix_field_3d              = regular_grid<matrix3, 3>;
using regular_matrix_field_4d              = regular_grid<matrix4, 4>;

using regular_time_variant_scalar_field_2d = regular_grid<scalar , 3>;
using regular_time_variant_scalar_field_3d = regular_grid<scalar , 4>;

using regular_time_variant_vector_field_2d = regular_grid<vector2, 3>;
using regular_time_variant_vector_field_3d = regular_grid<vector3, 4>;

using regular_time_variant_matrix_field_2d = regular_grid<matrix2, 3>;
using regular_time_variant_matrix_field_3d = regular_grid<matrix3, 4>;
}

#endif