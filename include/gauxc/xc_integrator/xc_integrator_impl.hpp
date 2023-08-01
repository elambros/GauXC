/**
 * GauXC Copyright (c) 2020-2023, The Regents of the University of California,
 * through Lawrence Berkeley National Laboratory (subject to receipt of
 * any required approvals from the U.S. Dept. of Energy). All rights reserved.
 *
 * See LICENSE.txt for details
 */
#pragma once

#include <gauxc/xc_integrator.hpp>

namespace GauXC  {
namespace detail {

/** Base class for XCIntegrator implementation */
template <typename MatrixType>
class XCIntegratorImpl {

public:

  using matrix_type    = MatrixType;
  using value_type     = typename matrix_type::value_type;
  using exc_vxc_type   = typename XCIntegrator<MatrixType>::exc_vxc_type;
  using exc_vxc_type_UKS   = typename XCIntegrator<MatrixType>::exc_vxc_type_UKS;
  using exc_vxc_type_GKS   = typename XCIntegrator<MatrixType>::exc_vxc_type_GKS;
  using exc_grad_type  = typename XCIntegrator<MatrixType>::exc_grad_type;
  using exx_type       = typename XCIntegrator<MatrixType>::exx_type;

protected:

  virtual value_type    integrate_den_( const MatrixType& P ) = 0;
  virtual exc_vxc_type  eval_exc_vxc_ ( const MatrixType& P ) = 0;
  virtual exc_vxc_type_UKS  eval_exc_vxc_ ( const MatrixType& P, const MatrixType& Pz ) = 0;
  virtual exc_vxc_type_GKS  eval_exc_vxc_ ( const MatrixType& P, const MatrixType& Pz, const MatrixType& Px, const MatrixType& Py ) = 0;
  virtual exc_grad_type eval_exc_grad_( const MatrixType& P ) = 0;
  virtual exx_type      eval_exx_     ( const MatrixType&     P, 
                                        const IntegratorSettingsEXX& settings ) = 0;
  virtual const util::Timer& get_timings_() const = 0;
  virtual const LoadBalancer& get_load_balancer_() const = 0;
  virtual LoadBalancer& get_load_balancer_() = 0;
  
public:

  // Default all ctors as base is stateless

  XCIntegratorImpl()                                   = default;
  XCIntegratorImpl( const XCIntegratorImpl& )          = default;
  XCIntegratorImpl( XCIntegratorImpl&&      ) noexcept = default;
  virtual ~XCIntegratorImpl()                 noexcept = default;

  /** Integrate Density (approx N_EL)
   *
   *  @param[in] P The density matrix
   *  @returns Approx Tr[P*S]
   */
 value_type integrate_den( const MatrixType& P ) {
   return integrate_den_(P);
 }

  /** Integrate EXC / VXC (Mean field terms) for RKS
   * 
   *   TODO: add API for UKS/GKS
   *
   *  @param[in] P The alpha density matrix
   *  @returns EXC / VXC in a combined structure
   */
  exc_vxc_type eval_exc_vxc( const MatrixType& P ) {
    return eval_exc_vxc_(P);
  }

  exc_vxc_type_UKS eval_exc_vxc( const MatrixType& P, const MatrixType& Pz ) {
    return eval_exc_vxc_(P, Pz);
  }

  exc_vxc_type_GKS eval_exc_vxc( const MatrixType& P, const MatrixType& Pz, const MatrixType& Px, const MatrixType& Py ) {
    return eval_exc_vxc_(P, Pz, Px, Py);
  }

  /** Integrate EXC gradient for RKS
   * 
   *   TODO: add API for UKS/GKS
   *
   *  @param[in] P The alpha density matrix
   *  @returns EXC gradient
   */
  exc_grad_type eval_exc_grad( const MatrixType& P ) {
    return eval_exc_grad_(P);
  }

  /** Integrate Exact Exchange for RHF
   * 
   *   TODO: add API for UHF/GHF
   *
   *  @param[in] P The alpha density matrix
   *  @returns Excact Exchange Matrix
   */
  exx_type eval_exx( const MatrixType& P, const IntegratorSettingsEXX& settings ) {
    return eval_exx_(P,settings);
  }

  /** Get internal timers
   *
   *  @returns Timer instance for internal timings
   */
  const util::Timer& get_timings() const {
    return get_timings_();
  }


  const LoadBalancer& load_balancer() const {
    return get_load_balancer_();
  }
  LoadBalancer& load_balancer() {
    return get_load_balancer_();
  }
};

}
}
