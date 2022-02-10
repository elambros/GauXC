#pragma once

#include <gauxc/molecule.hpp>
#include <gauxc/grid.hpp>
#include <gauxc/grid_factory.hpp>

#include <unordered_map>

namespace GauXC {

using atomic_grid_map = std::unordered_map< AtomicNumber, Grid >;
using atomic_grid_spec_map = std::unordered_map< AtomicNumber, atomic_grid_variant>;

namespace detail {
  class MolGridImpl;
}

class MolGrid {

  std::shared_ptr<detail::MolGridImpl> pimpl_;

public:

  MolGrid( const atomic_grid_map& );
  MolGrid( const atomic_grid_spec_map& );

  MolGrid( const MolGrid& );
  MolGrid( MolGrid&& ) noexcept;

  ~MolGrid() noexcept;

  size_t natoms_uniq() const;
  const Grid& get_grid( AtomicNumber ) const;
        Grid& get_grid( AtomicNumber )      ;

  size_t max_nbatches() const;

};

}
