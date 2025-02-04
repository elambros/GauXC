/**
 * GauXC Copyright (c) 2020-2023, The Regents of the University of California,
 * through Lawrence Berkeley National Laboratory (subject to receipt of
 * any required approvals from the U.S. Dept. of Energy). All rights reserved.
 *
 * See LICENSE.txt for details
 */
#pragma once

#include <gauxc/basisset.hpp>

namespace GauXC {

void gau2grid_collocation( size_t                  npts, 
                           size_t                  nshells,
                           size_t                  nbe,
                           const double*           points, 
                           const BasisSet<double>& basis,
                           const int32_t*          shell_mask,
                           double*                 basis_eval );

void gau2grid_collocation_gradient( size_t                  npts, 
                                    size_t                  nshells,
                                    size_t                  nbe,
                                    const double*           points, 
                                    const BasisSet<double>& basis,
                                    const int32_t*          shell_mask,
                                    double*                 basis_eval, 
                                    double*                 dbasis_x_eval, 
                                    double*                 dbasis_y_eval,
                                    double*                 dbasis_z_eval );


void gau2grid_collocation_hessian( size_t                  npts, 
                                   size_t                  nshells,
                                   size_t                  nbe,
                                   const double*           points, 
                                   const BasisSet<double>& basis,
                                   const int32_t*          shell_mask,
                                   double*                 basis_eval, 
                                   double*                 dbasis_x_eval, 
                                   double*                 dbasis_y_eval,
                                   double*                 dbasis_z_eval, 
                                   double*                 d2basis_xx_eval, 
                                   double*                 d2basis_xy_eval,
                                   double*                 d2basis_xz_eval,
                                   double*                 d2basis_yy_eval,
                                   double*                 d2basis_yz_eval,
                                   double*                 d2basis_zz_eval);
}
