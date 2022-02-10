#pragma once

#include <gauxc/molgrid.hpp>

namespace GauXC {

#if 0
  atomic_scal_factor_map get_default_scaling_factors( RadialQuad rq, 
    AtomicNumber maxZ );

  atomic_grid_size_map get_default_grid_sizes( AtomicGridSizeDefault G, 
    AtomicNumber maxZ );

  atomic_scal_factor_map slater_radii_64();
  atomic_scal_factor_map slater_radii_30();
  atomic_scal_factor_map clementi_radii_67();
#else
  double slater_radius_64(AtomicNumber);
  double slater_radius_30(AtomicNumber);
  double clementi_radius_67(AtomicNumber);
  double default_atomic_radius(AtomicNumber);

  RadialScale default_mk_radial_scaling_factor( AtomicNumber );
  RadialScale default_mhl_radial_scaling_factor( AtomicNumber );
  RadialScale default_ta_radial_scaling_factor( AtomicNumber );

  PrunedAtomicGridSpecification robust_psi4_pruning_scheme(
    UnprunedAtomicGridSpecification
  );

  struct MolGridFactory {

    static atomic_grid_spec_map create_grid_spec( RadialQuad, 
      AtomicGridSizeDefault, const Molecule& );

    static atomid_grid_map generate_molgrid( RadialQuad, AtomicGridSizeDefault,
      const Molecule& );

  };
  
#endif

}

