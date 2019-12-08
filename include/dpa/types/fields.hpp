#ifndef DPA_TYPES_FIELDS_HPP
#define DPA_TYPES_FIELDS_HPP

#include <dpa/types/basic_types.hpp>
#include <dpa/types/cartesian_grid.hpp>

namespace dpa
{
using scalar_field_2d              = cartesian_grid<scalar , vector2, ivector2>;
using scalar_field_3d              = cartesian_grid<scalar , vector3, ivector3>;
using scalar_field_4d              = cartesian_grid<scalar , vector4, ivector4>;

using vector_field_2d              = cartesian_grid<vector2, vector2, ivector2>;
using vector_field_3d              = cartesian_grid<vector3, vector3, ivector3>;
using vector_field_4d              = cartesian_grid<vector4, vector4, ivector4>;

using matrix_field_2d              = cartesian_grid<matrix2, vector2, ivector2>;
using matrix_field_3d              = cartesian_grid<matrix3, vector3, ivector3>;
using matrix_field_4d              = cartesian_grid<matrix4, vector4, ivector4>;

using time_variant_scalar_field_2d = cartesian_grid<scalar , vector3, ivector3>;
using time_variant_scalar_field_3d = cartesian_grid<scalar , vector4, ivector4>;

using time_variant_vector_field_2d = cartesian_grid<vector2, vector3, ivector3>;
using time_variant_vector_field_3d = cartesian_grid<vector3, vector4, ivector4>;

using time_variant_matrix_field_2d = cartesian_grid<matrix2, vector3, ivector3>;
using time_variant_matrix_field_3d = cartesian_grid<matrix3, vector4, ivector4>;
}

#endif