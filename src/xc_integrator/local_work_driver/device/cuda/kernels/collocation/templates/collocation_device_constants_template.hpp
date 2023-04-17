/**
 * GauXC Copyright (c) 2020-2023, The Regents of the University of California,
 * through Lawrence Berkeley National Laboratory (subject to receipt of
 * any required approvals from the U.S. Dept. of Energy). All rights reserved.
 *
 * See LICENSE.txt for details
 */
#pragma once

namespace GauXC      {
namespace integrator {
namespace cuda       {

$for( x in const_lines )\
  $(x)
$endfor\

} // namespace cuda
} // namespace integrator
} // namespace GauXC
