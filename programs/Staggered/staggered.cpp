/*********************************************
 * Simulates staggered fermions with a gauge *
 * interaction                               *
 *********************************************/


#include "staggered.h"
#include "../plumbing/inputs.h"
#include "../plumbing/algorithms/hmc.h"
#include "../plumbing/gauge_field.h"
#include "../plumbing/fermion_field.h"




using SG = gauge_action<N, double>;
using Dtype = dirac_staggered<VEC,SUN>;
using SF = fermion_action<SG, VEC, Dtype>;




template<typename action_type>
class full_action{
  public:
    action_type &act;
    field<SUN> (&gauge)[NDIM];

    full_action(action_type &_act) : act(_act), gauge(_act.gauge){}

    //The gauge action
    double action(){
      return act.action();
    }

    /// Gaussian random momentum for each element
    void generate_momentum(){
      act.generate_momentum();
    }

    // Update the momentum with the gauge field
    void integrate(int steps, double dt){
      for(int step=0; step < steps; step++){
        act.integrator_step(dt/steps);
      }
    }
};










int main(int argc, char **argv){

  input parameters = input();
  parameters.import("parameters");
  double beta = parameters.get("beta");
  double mass = parameters.get("mass");
  int seed = parameters.get("seed");
	double hmc_steps = parameters.get("hmc_steps");
	double traj_length = parameters.get("traj_length");

  lattice->setup( nd[0], nd[1], nd[2], nd[3], argc, argv );
  seed_random(seed);








  // Test the force calculation by varying one gauge link
  // (Needs to be moved to tests)
  gauge_action<N, double> ga(1.0);
  // Starting with a unit configuration
  foralldir(dir){
    onsites(ALL){
      ga.gauge[dir][X].random();
    }
  }
  for(int ng = 0; ng < ga.n_generators(); ng++){
    foralldir(dir){
      onsites(ALL){
        ga.momentum[dir][X] = 0;
      }
    }

    double eps = 1e-6;
    SUN g1 = ga.gauge[0].get_value_at(50);
    SUN h = 1;
    h += eps * ga.generator(ng);
    SUN g12 = h*g1;

    double s1 = plaquette_sum(ga.gauge);

    if(mynode()==0){
      ga.gauge[0].set_value_at(g12,50);
      //ga.gauge[0].set_value_at(g12,lattice->neighb[1][50]);
    }
    ga.gauge[0].mark_changed(ALL);
    double s2 = plaquette_sum(ga.gauge);

    if(mynode()==0)
      ga.gauge[0].set_value_at(g1, 50);
    ga.gauge[0].mark_changed(ALL);

    gauge_force(ga.gauge, ga.momentum, 1.0/N);
    SUN f = ga.momentum[0].get_value_at(50);
    double diff = (f*ga.generator(ng)).trace().re - (s2-s1)/eps;

    if(mynode()==0) {
      //hila::output << "Force 1 " << (f*ga.generator(ng)).trace().re << "\n";
      //hila::output << "Force 2 " << (s2-s1)/eps << "\n";
      //hila::output << "Force " << ng << " diff " << diff << "\n";
      h = ga.generator(ng);
      assert( diff*diff < eps*eps*1000 && "Gauge force" );
    }


    // Check also the momentum action and derivative
    ga.generate_momentum();

    s1 = momentum_action(ga.momentum);
    h = ga.momentum[0].get_value_at(0);
    h += eps * ga.generator(ng);
    if(mynode()==0)
      ga.momentum[0].set_value_at(h, 0);
    s2 = momentum_action(ga.momentum);

    diff = (h*ga.generator(ng)).trace().re + (s2-s1)/eps;
    if(mynode()==0) {
      //hila::output << "Mom 1 " << (h*ga.generator(ng)).trace().re << "\n";
      //hila::output << "Mom 2 " << (s2-s1)/eps << "\n";
      //hila::output << "Mom " << ng << " diff " << diff << "\n";
      h = ga.generator(ng);
      assert( diff*diff < eps*eps*1000 && "Momentum derivative" );
    }
  }



  // Now the actual simulation
  ga = SG(beta);
  ga.set_unity();

  Dtype D(mass, ga.gauge);
  SF fa(ga, D);
  full_action<SF> action(fa);
  for(int step = 0; step < 5; step ++){
    update_hmc(action, hmc_steps, traj_length);
    double plaq = plaquette(ga.gauge);
    output0 << "Plaq: " << plaq << "\n";
  }



  finishrun();

  return 0;
}
