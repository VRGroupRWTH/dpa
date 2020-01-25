#ifndef DPA_TYPES_INTEGRATORS_HPP
#define DPA_TYPES_INTEGRATORS_HPP

#include <variant>

#include <boost/numeric/odeint/external/eigen/eigen_algebra.hpp>
#include <boost/numeric/odeint.hpp>

#include <dpa/types/basic_types.hpp>

namespace dpa
{
template <typename state_type>
using euler_integrator                        = boost::numeric::odeint::euler                  <   state_type, scalar, state_type, scalar, boost::numeric::odeint::vector_space_algebra>;
template <typename state_type>
using modified_midpoint_integrator            = boost::numeric::odeint::modified_midpoint      <   state_type, scalar, state_type, scalar, boost::numeric::odeint::vector_space_algebra>;
template <typename state_type>
using runge_kutta_4_integrator                = boost::numeric::odeint::runge_kutta4           <   state_type, scalar, state_type, scalar, boost::numeric::odeint::vector_space_algebra>;
template <typename state_type>
using runge_kutta_cash_karp_54_integrator     = boost::numeric::odeint::runge_kutta_cash_karp54<   state_type, scalar, state_type, scalar, boost::numeric::odeint::vector_space_algebra>;
template <typename state_type>
using runge_kutta_dormand_prince_5_integrator = boost::numeric::odeint::runge_kutta_dopri5     <   state_type, scalar, state_type, scalar, boost::numeric::odeint::vector_space_algebra>;
template <typename state_type>
using runge_kutta_fehlberg_78_integrator      = boost::numeric::odeint::runge_kutta_fehlberg78 <   state_type, scalar, state_type, scalar, boost::numeric::odeint::vector_space_algebra>;
template <typename state_type>
using adams_bashforth_2_integrator            = boost::numeric::odeint::adams_bashforth        <2, state_type, scalar, state_type, scalar, boost::numeric::odeint::vector_space_algebra>;
template <typename state_type>
using adams_bashforth_moulton_2_integrator    = boost::numeric::odeint::adams_bashforth_moulton<2, state_type, scalar, state_type, scalar, boost::numeric::odeint::vector_space_algebra>;

template <typename state_type>
using variant_integrator                      = std::variant<
  euler_integrator                       <state_type>, 
  modified_midpoint_integrator           <state_type>,
  runge_kutta_4_integrator               <state_type>, 
  runge_kutta_cash_karp_54_integrator    <state_type>, 
  runge_kutta_dormand_prince_5_integrator<state_type>, 
  runge_kutta_fehlberg_78_integrator     <state_type>, 
  adams_bashforth_2_integrator           <state_type>,
  adams_bashforth_moulton_2_integrator   <state_type>>;

using variant_scalar_integrator               = variant_integrator<scalar >;
using variant_vector2_integrator              = variant_integrator<vector2>;
using variant_vector3_integrator              = variant_integrator<vector3>;
using variant_vector4_integrator              = variant_integrator<vector4>;
using variant_matrix2_integrator              = variant_integrator<matrix2>;
using variant_matrix3_integrator              = variant_integrator<matrix3>;
using variant_matrix4_integrator              = variant_integrator<matrix4>;
}

#endif