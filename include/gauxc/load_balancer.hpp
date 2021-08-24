#pragma once

#include <gauxc/molgrid.hpp>
#include <gauxc/molmeta.hpp>
#include <gauxc/basisset.hpp>
#include <gauxc/xc_task.hpp>
#include <gauxc/util/timer.hpp>

#ifdef GAUXC_ENABLE_MPI
#include <mpi.h>
#endif

namespace GauXC {

namespace detail {
  class LoadBalancerImpl;
}

struct LoadBalancerState {
  bool modified_weights_are_stored = false;
};

class LoadBalancer {

  using basis_type = BasisSet<double>;
  using pimpl_type = detail::LoadBalancerImpl;
  std::unique_ptr<pimpl_type> pimpl_;

public:

  LoadBalancer();

  LoadBalancer( std::unique_ptr<pimpl_type>&& pimpl );

  LoadBalancer( const LoadBalancer& );
  LoadBalancer( LoadBalancer&& ) noexcept;

  ~LoadBalancer() noexcept;

  const std::vector<XCTask>& get_tasks() const;
        std::vector<XCTask>& get_tasks()      ;
  
  const util::Timer& get_timings() const;

  size_t max_npts()       const;
  size_t max_nbe()        const;
  size_t max_npts_x_nbe() const;

  const Molecule& molecule() const;
  const MolMeta&  molmeta()  const;
  const basis_type& basis()  const;
  
  LoadBalancerState& state();

#ifdef GAUXC_ENABLE_MPI
  MPI_Comm comm() const;
#endif

};



class LoadBalancerFactory {

public:

  LoadBalancerFactory() = delete;

  LoadBalancerFactory( ExecutionSpace ex, std::string kernel_name );

#ifdef GAUXC_ENABLE_MPI
  LoadBalancer get_instance( MPI_Comm comm, const Molecule& mol, 
    const MolGrid& mg, const BasisSet<double>& );
  std::shared_ptr<LoadBalancer> get_shared_instance( MPI_Comm comm, const Molecule& mol, 
    const MolGrid& mg, const BasisSet<double>& );
#else
  LoadBalancer get_instance( const Molecule& mol, const MolGrid& mg, 
    const BasisSet<double>& );
  std::shared_ptr<LoadBalancer> get_shared_instance( const Molecule& mol, const MolGrid& mg, 
    const BasisSet<double>& );
#endif

private:

  ExecutionSpace ex_;
  std::string    kernel_name_;

};




#if 0
// Factories

namespace factory {

#ifdef GAUXC_ENABLE_MPI

std::shared_ptr<LoadBalancer> make_default_load_balancer(
  MPI_Comm, const Molecule&, const MolGrid&, const BasisSet<double>&
);
std::shared_ptr<LoadBalancer> make_default_load_balancer(
  MPI_Comm, const Molecule&, const MolGrid&, const BasisSet<double>&, 
  const MolMeta&
);
std::shared_ptr<LoadBalancer> make_default_load_balancer(
  MPI_Comm, const Molecule&, const MolGrid&, const BasisSet<double>&, 
  std::shared_ptr<MolMeta>
);

#else

std::shared_ptr<LoadBalancer> make_default_load_balancer(
  const Molecule&, const MolGrid&, const BasisSet<double>&
);
std::shared_ptr<LoadBalancer> make_default_load_balancer(
  const Molecule&, const MolGrid&, const BasisSet<double>&, 
  const MolMeta&
);
std::shared_ptr<LoadBalancer> make_default_load_balancer(
  const Molecule&, const MolGrid&, const BasisSet<double>&, 
  std::shared_ptr<MolMeta>
);

#endif

}
#endif

}
