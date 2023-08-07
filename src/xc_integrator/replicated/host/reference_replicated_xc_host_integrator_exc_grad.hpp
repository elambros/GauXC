/**
 * GauXC Copyright (c) 2020-2023, The Regents of the University of California,
 * through Lawrence Berkeley National Laboratory (subject to receipt of
 * any required approvals from the U.S. Dept. of Energy). All rights reserved.
 *
 * See LICENSE.txt for details
 */
#pragma once

#include "reference_replicated_xc_host_integrator.hpp"
#include "integrator_util/integrator_common.hpp"
#include "host/local_host_work_driver.hpp"
#include <stdexcept>

namespace GauXC  {
namespace detail {

template <typename ValueType>
void ReferenceReplicatedXCHostIntegrator<ValueType>::
  eval_exc_grad_( int64_t m, int64_t n, const value_type* P,
                 int64_t ldp, value_type* EXC_GRAD ) { 
                 
                 
  const auto& basis = this->load_balancer_->basis();

  // Check that P is sane
  const int64_t nbf = basis.nbf();
  if( m != n ) 
    GAUXC_GENERIC_EXCEPTION("P Must Be Square");
  if( m != nbf ) 
    GAUXC_GENERIC_EXCEPTION("P Must Have Same Dimension as Basis");
  if( ldp < nbf )
    GAUXC_GENERIC_EXCEPTION("Invalid LDP");
                 
                 
  // Get Tasks
  this->load_balancer_->get_tasks();
                 
  // Compute Local contributions to EXC / VXC
  this->timer_.time_op("XCIntegrator.LocalWork", [&](){
    exc_grad_local_work_( P, ldp, EXC_GRAD );
  });


  // Reduce Results
  this->timer_.time_op("XCIntegrator.Allreduce", [&](){

    if( not this->reduction_driver_->takes_host_memory() )
      GAUXC_GENERIC_EXCEPTION("This Module Only Works With Host Reductions");

    const int natoms = this->load_balancer_->molecule().natoms();
    this->reduction_driver_->allreduce_inplace( EXC_GRAD, 3*natoms, ReductionOp::Sum );
  });

}

template <typename ValueType>
void ReferenceReplicatedXCHostIntegrator<ValueType>::
  exc_grad_local_work_( const value_type* P, int64_t ldp, value_type* EXC_GRAD ) {

  // Cast LWD to LocalHostWorkDriver
  auto* lwd = dynamic_cast<LocalHostWorkDriver*>(this->local_work_driver_.get());

  // Setup Aliases
  const auto& func  = *this->func_;
  const auto& basis = this->load_balancer_->basis();
  const auto& mol   = this->load_balancer_->molecule();

  // Get basis map
  BasisSetMap basis_map(basis,mol);

  const int32_t nbf = basis.nbf();
  const int32_t natoms = mol.natoms();

  // Sort tasks on size (XXX: maybe doesnt matter?)
  auto task_comparator = []( const XCTask& a, const XCTask& b ) {
    return (a.points.size() * a.bfn_screening.nbe) > (b.points.size() * b.bfn_screening.nbe);
  };

  auto& tasks = this->load_balancer_->get_tasks();
  std::sort( tasks.begin(), tasks.end(), task_comparator );


  // Check that Partition Weights have been calculated
  auto& lb_state = this->load_balancer_->state();
  if( not lb_state.modified_weights_are_stored ) {
    GAUXC_GENERIC_EXCEPTION("Weights Have Not Beed Modified"); 
  }

  // Zero out integrands
  for( auto i = 0; i < 3*natoms; ++i ) {
    EXC_GRAD[i] = 0.;
  }

  // Loop over tasks
  const size_t ntasks = tasks.size();
  #pragma omp parallel
  {

  XCHostData<value_type> host_data; // Thread local host data

  #pragma omp for schedule(dynamic)
  for( size_t iT = 0; iT < ntasks; ++iT ) {

    // Alias current task
    const auto& task = tasks[iT];

    // Get tasks constants
    const int32_t  npts    = task.points.size();
    const int32_t  nbe     = task.bfn_screening.nbe;
    const int32_t  nshells = task.bfn_screening.shell_list.size();

    const auto* points      = task.points.data()->data();
    const auto* weights     = task.weights.data();
    const int32_t* shell_list = task.bfn_screening.shell_list.data();

    // Allocate enough memory for batch

    // Things that every calc needs
    host_data.nbe_scr .resize( nbe * nbe  );
    host_data.eps     .resize( npts );
    host_data.vrho    .resize( npts );
    host_data.den_scr .resize( 4 * npts );

    if( func.is_lda() ) {
      host_data.basis_eval .resize( 4 * npts * nbe );
      host_data.zmat       .resize( npts * nbe );
    }

    if( func.is_gga() ){
      host_data.basis_eval .resize( 10 * npts * nbe );
      host_data.zmat       .resize( 4  * npts * nbe );
      host_data.gamma      .resize( npts );
      host_data.vgamma     .resize( npts );
    }

    // Alias/Partition out scratch memory
    auto* basis_eval = host_data.basis_eval.data();
    auto* den_eval   = host_data.den_scr.data();
    auto* nbe_scr    = host_data.nbe_scr.data();
    auto* zmat       = host_data.zmat.data();

    auto* zmat_x = zmat   + npts*nbe;
    auto* zmat_y = zmat_x + npts*nbe;
    auto* zmat_z = zmat_y + npts*nbe;

    auto* eps        = host_data.eps.data();
    auto* gamma      = host_data.gamma.data();
    auto* vrho       = host_data.vrho.data();
    auto* vgamma     = host_data.vgamma.data();

    auto* dbasis_x_eval = basis_eval    + npts * nbe;
    auto* dbasis_y_eval = dbasis_x_eval + npts * nbe;
    auto* dbasis_z_eval = dbasis_y_eval + npts * nbe;
    auto* dden_x_eval   = den_eval    + npts;
    auto* dden_y_eval   = dden_x_eval + npts;
    auto* dden_z_eval   = dden_y_eval + npts;

    value_type* d2basis_xx_eval = nullptr;
    value_type* d2basis_xy_eval = nullptr;
    value_type* d2basis_xz_eval = nullptr;
    value_type* d2basis_yy_eval = nullptr;
    value_type* d2basis_yz_eval = nullptr;
    value_type* d2basis_zz_eval = nullptr;

    if( func.is_gga() ) {
      d2basis_xx_eval = dbasis_z_eval   + npts * nbe;
      d2basis_xy_eval = d2basis_xx_eval + npts * nbe;
      d2basis_xz_eval = d2basis_xy_eval + npts * nbe;
      d2basis_yy_eval = d2basis_xz_eval + npts * nbe;
      d2basis_yz_eval = d2basis_yy_eval + npts * nbe;
      d2basis_zz_eval = d2basis_yz_eval + npts * nbe;
    }


    // Get the submatrix map for batch
    auto [submat_map, foo] = 
      gen_compressed_submat_map( basis_map, task.bfn_screening.shell_list, nbf, nbf );

    // Evaluate Collocation Gradient (+ Hessian)
    if( func.is_gga() )
      lwd->eval_collocation_hessian( npts, nshells, nbe, points, basis, shell_list, 
        basis_eval, dbasis_x_eval, dbasis_y_eval, dbasis_z_eval, d2basis_xx_eval,
        d2basis_xy_eval, d2basis_xz_eval, d2basis_yy_eval, d2basis_yz_eval,
        d2basis_zz_eval );
    else
      lwd->eval_collocation_gradient( npts, nshells, nbe, points, basis, shell_list, 
        basis_eval, dbasis_x_eval, dbasis_y_eval, dbasis_z_eval );


    // Evaluate X matrix (P * B/Bx/By/Bz) -> store in Z
    // XXX: This assumes that bfn + gradients are contiguous in memory
    if( func.is_gga() ) {
      lwd->eval_xmat( 4*npts, nbf, nbe, submat_map, P, ldp, basis_eval, nbe,
        zmat, nbe, nbe_scr );
    } else {
      lwd->eval_xmat( npts, nbf, nbe, submat_map, P, ldp, basis_eval, nbe,
        zmat, nbe, nbe_scr );
    }


    // Evaluate U and V variables
    if( func.is_gga() )
      lwd->eval_uvvar_gga_rks( npts, nbe, basis_eval, dbasis_x_eval, dbasis_y_eval,
        dbasis_z_eval, zmat, nbe, den_eval, dden_x_eval, dden_y_eval, dden_z_eval,
        gamma );
     else
      lwd->eval_uvvar_lda_rks( npts, nbe, basis_eval, zmat, nbe, den_eval );


    // Evaluate XC functional
    if( func.is_gga() )
      func.eval_exc_vxc( npts, den_eval, gamma, eps, vrho, vgamma );
    else
      func.eval_exc_vxc( npts, den_eval, eps, vrho );


    // Increment EXC Gradient
    size_t bf_off = 0;
    for( auto ish = 0; ish < nshells; ++ish ) {
      const int sh_idx = shell_list[ish];
      const int sh_sz  = basis[sh_idx].size();
      const int iAt    = basis_map.shell_to_center( sh_idx );

      double g_acc_x(0), g_acc_y(0), g_acc_z(0);
      for( int ibf = 0, mu = bf_off; ibf < sh_sz; ++ibf, ++mu )
      for( int ipt = 0; ipt < npts; ++ipt ) {

	      const int32_t mu_i = mu + ipt*nbe;

	      // LDA Contributions
        const double vrho_ipt = weights[ipt] * vrho[ipt];

	      const double z = zmat[mu_i]; // Z = N * B

	      const double dbx = dbasis_x_eval[mu_i]; // B_x
	      const double dby = dbasis_y_eval[mu_i]; // B_y
	      const double dbz = dbasis_z_eval[mu_i]; // B_z

	      g_acc_x += vrho_ipt * z * dbx;
	      g_acc_y += vrho_ipt * z * dby;
	      g_acc_z += vrho_ipt * z * dbz;

	      if( func.is_gga() ) {
      	  // GGA Contributions
          const double vgamma_ipt = weights[ipt] * vgamma[ipt];

          const double dden_x = dden_x_eval[ipt];
          const double dden_y = dden_y_eval[ipt];
          const double dden_z = dden_z_eval[ipt];

	        const double zx = zmat_x[mu_i]; // Z_x = N * B_x
	        const double zy = zmat_y[mu_i]; // Z_y = N * B_y
	        const double zz = zmat_z[mu_i]; // Z_z = N * B_z

	        const double d2bxx = d2basis_xx_eval[mu_i]; // B^2_xx
	        const double d2bxy = d2basis_xy_eval[mu_i]; // B^2_xy
	        const double d2bxz = d2basis_xz_eval[mu_i]; // B^2_xz
	        const double d2byy = d2basis_yy_eval[mu_i]; // B^2_yy
	        const double d2byz = d2basis_yz_eval[mu_i]; // B^2_yz
	        const double d2bzz = d2basis_zz_eval[mu_i]; // B^2_zz

	        // sum_j B^2_{ij} * d_j n
	        double d2_term_x = d2bxx * dden_x + d2bxy * dden_y + d2bxz * dden_z;
	        double d2_term_y = d2bxy * dden_x + d2byy * dden_y + d2byz * dden_z;
	        double d2_term_z = d2bxz * dden_x + d2byz * dden_y + d2bzz * dden_z;

	        // sum_j (d_j n) * Z^j
	        double d11_zmat_term = dden_x * zx + dden_y * zy + dden_z * zz;

	        g_acc_x += 2 * vgamma_ipt * ( z * d2_term_x + dbx * d11_zmat_term );
	        g_acc_y += 2 * vgamma_ipt * ( z * d2_term_y + dby * d11_zmat_term );
	        g_acc_z += 2 * vgamma_ipt * ( z * d2_term_z + dbz * d11_zmat_term );
	      }

      } // loop over bfns + grid points

      #pragma omp atomic
      EXC_GRAD[3*iAt + 0] += -2 * g_acc_x;
      #pragma omp atomic
      EXC_GRAD[3*iAt + 1] += -2 * g_acc_y;
      #pragma omp atomic
      EXC_GRAD[3*iAt + 2] += -2 * g_acc_z;

      bf_off += sh_sz; // Increment basis offset

    } // End loop over shells 
        
  } // End loop over tasks

  } // OpenMP Region

  
}

}
}
