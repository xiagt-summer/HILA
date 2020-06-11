#include "test.h"
#include "datatypes/vector.h"
#include "datatypes/sun.h"
#include "datatypes/representations.h"
#include "plumbing/field.h"
#include "dirac/staggered.h"
#include "dirac/wilson.h"
#include "hmc/update.h"
#include "hmc/gauge_field.h"
#include "hmc/fermion_field.h"

constexpr int N = 3;




template<typename dirac, typename matrix>
void check_forces(double mass_parameter){
  field<SU<N>> gauge[NDIM];
  field<SU<N>> momentum[NDIM];
  double eps = 1e-5;

  dirac D(mass_parameter, gauge);
  fermion_action fa(D, momentum);

  for(int ng = 0; ng < 1 ; ng++ ){ //matrix::generator_count(); ng++){
    foralldir(dir){
      onsites(ALL){
        gauge[dir][X] = 1;
      }
    }
    fa.draw_gaussian_fields();
    foralldir(dir){
      momentum[dir][ALL] = 0;
    }

    matrix g1 = gauge[0].get_value_at(50);
    matrix h = matrix(1) + cmplx(0, eps) * matrix::generator(ng);
    matrix g12 = h*g1;

    field<typename dirac::vector_type> psi, chi, tmp, tmp2;
    #if NDIM > 3
    psi.set_boundary_condition(TUP, boundary_condition_t::ANTIPERIODIC);
    psi.set_boundary_condition(TDOWN, boundary_condition_t::ANTIPERIODIC);
    #endif
    chi.copy_boundary_condition(psi);
    tmp.copy_boundary_condition(psi);
    tmp2.copy_boundary_condition(psi);

    onsites(D.par){
      if(disable_avx[X]==0){};
      psi[X].gaussian();
      chi[X].gaussian();
    }
    
    double s1 = 0;
    D.apply(psi,tmp);
    onsites(D.par){
      s1 += chi[X].rdot(tmp[X]);
    }

    if(mynode()==0){
      gauge[0].set_value_at(g12,50);
    }
    gauge[0].mark_changed(ALL);

    double s2 = 0;
    D.apply(psi,tmp);
    onsites(D.par){
      s2 += chi[X].rdot(tmp[X]);
    }

    if(mynode()==0)
      gauge[0].set_value_at(g1, 50);
    gauge[0].mark_changed(ALL);

    D.force(chi, psi, momentum, 1);

    matrix f = momentum[0].get_value_at(50);
    double f1 = (s2-s1)/eps;
    double f2 = (f*cmplx(0,1)*matrix::generator(ng)).trace().re;
    double diff = f2-f1;

    if(mynode()==0) {
      //hila::output << "Action 1 " << s1 << "\n";
      //hila::output << "Action 2 " << s2 << "\n";
      //hila::output << "Calculated deriv " << f2 << "\n";
      //hila::output << "Actual deriv " << (s2-s1)/eps << "\n";
      //hila::output << "Fermion deriv " << ng << " diff " << diff << "\n";
      assert( diff*diff < eps*10 && "Fermion deriv" );
    }



    onsites(D.par){
      if(disable_avx[X]==0){};
      psi[X].gaussian();
      chi[X].gaussian();
    }

    s1 = 0;
    D.dagger(psi,tmp);
    onsites(D.par){
      s1 += chi[X].rdot(tmp[X]);
    }

    if(mynode()==0){
      gauge[0].set_value_at(g12,50);
    }
    gauge[0].mark_changed(ALL);

    s2 = 0;
    D.dagger(psi,tmp);
    onsites(D.par){
      s2 += chi[X].rdot(tmp[X]);
    }

    if(mynode()==0)
      gauge[0].set_value_at(g1, 50);
    gauge[0].mark_changed(ALL);

    D.force(chi, psi, momentum, -1);
    f = momentum[0].get_value_at(50);
    f1 = (s2-s1)/eps;
    f2 = (f*cmplx(0,1)*matrix::generator(ng)).trace().re;
    diff = f2-f1;

    if(mynode()==0) {
      //hila::output << "Action 1 " << s1 << "\n";
      //hila::output << "Action 2 " << s2 << "\n";
      //hila::output << "Calculated deriv " << f2 << "\n";
      //hila::output << "Actual deriv " << (s2-s1)/eps << "\n";
      //hila::output << "Fermion dg deriv " << ng << " diff " << diff << "\n";
      assert( diff*diff < eps*10 && "Fermion dg deriv" );
    }



    foralldir(dir){
      momentum[dir][ALL] = 0;
    }

    if(mynode()==0){
      gauge[0].set_value_at(g12,50);
    }
    gauge[0].mark_changed(ALL);
    s2 = fa.action();

    if(mynode()==0)
      gauge[0].set_value_at(g1, 50);
    gauge[0].mark_changed(ALL);
    s1 = fa.action();

    fa.force_step(1.0);
    f = momentum[0].get_value_at(50);
    f1 = (s2-s1)/eps;
    f2 = (f*cmplx(0,1)*matrix::generator(ng)).trace().re;
    diff = f2-f1;

    if(mynode()==0) {
      //hila::output << "Action 1 " << s1 << "\n";
      //hila::output << "Action 2 " << s2 << "\n";
      //hila::output << "Calculated force " << f2 << "\n";
      //hila::output << "Actual force " << (s2-s1)/eps << "\n";
      //hila::output << "Fermion force " << ng << " diff " << diff << "\n";
      assert( diff*diff < eps*10 && "Fermion force" );
    }
  }
}




template<typename dirac, typename sun, typename repr>
void check_represented_forces(double mass_parameter){
  field<sun> gauge[NDIM];
  field<sun> momentum[NDIM];
  field<repr> rgauge[NDIM];
  field<squarematrix<repr::size,cmplx<double>>> rmom[NDIM];
  double eps = 1e-5;

  dirac D(mass_parameter, rgauge);
  high_representation_fermion_action fa(D, momentum, gauge, rgauge);

  for(int ng = 0; ng < sun::generator_count(); ng++){
    foralldir(dir){
      onsites(ALL){
        gauge[dir][X] = 1;
        rgauge[dir][X] = 1;
      }
    }
    fa.draw_gaussian_fields();
    foralldir(dir){
      momentum[dir][ALL] = 0;
      rmom[dir][ALL] = 0;
    }

    sun g1 = gauge[0].get_value_at(50);
    sun h = sun(1) + eps * cmplx(0,1)*sun::generator(ng);
    sun g12 = h*g1;

    field<typename dirac::vector_type> psi, chi, tmp, tmp2;
    #if NDIM > 3
    psi.set_boundary_condition(TUP, boundary_condition_t::ANTIPERIODIC);
    psi.set_boundary_condition(TDOWN, boundary_condition_t::ANTIPERIODIC);
    #endif
    chi.copy_boundary_condition(psi);
    tmp.copy_boundary_condition(psi);
    tmp2.copy_boundary_condition(psi);

    onsites(D.par){
      if(disable_avx[X]==0){};
      psi[X].gaussian();
      chi[X].gaussian();
    }
    
    double s1 = 0;
    D.apply(psi,tmp);
    onsites(D.par){
      s1 += chi[X].rdot(tmp[X]);
    }

    if(mynode()==0){
      gauge[0].set_value_at(g12,50);
    }
    gauge[0].mark_changed(ALL);
    foralldir(dir){
      onsites(ALL){
        rgauge[dir][X].represent(gauge[dir][X]);
      }
    }

    double s2 = 0;
    D.apply(psi,tmp);
    onsites(D.par){
      s2 += chi[X].rdot(tmp[X]);
    }

    if(mynode()==0)
      gauge[0].set_value_at(g1, 50);
    gauge[0].mark_changed(ALL);
    foralldir(dir){
      onsites(ALL){
        rgauge[dir][X].represent(gauge[dir][X]);
      }
    }

    D.force(chi, psi, rmom, 1);

    SU<repr::size,double> rf = rmom[0].get_value_at(50);
    sun f = repr::project_force(rf);
    double f1 = (s2-s1)/eps;
    double f2 = (f*cmplx(0,1)*sun::generator(ng)).trace().re;
    double diff = f2-f1;

    if(mynode()==0) {
      //hila::output << "Action 1 " << s1 << "\n";
      //hila::output << "Action 2 " << s2 << "\n";
      //hila::output << "Calculated deriv " << f2 << "\n";
      //hila::output << "Actual deriv " << (s2-s1)/eps << "\n";
      //hila::output << "Fermion deriv " << ng << " diff " << diff << "\n";
      assert( diff*diff < eps*10 && "Fermion deriv" );
    }



    onsites(D.par){
      if(disable_avx[X]==0){};
      psi[X].gaussian();
      chi[X].gaussian();
    }

    s1 = 0;
    D.dagger(psi,tmp);
    onsites(D.par){
      s1 += chi[X].rdot(tmp[X]);
    }

    if(mynode()==0){
      gauge[0].set_value_at(g12,50);
    }
    gauge[0].mark_changed(ALL);
    foralldir(dir){
      onsites(ALL){
        rgauge[dir][X].represent(gauge[dir][X]);
      }
    }

    s2 = 0;
    D.dagger(psi,tmp);
    onsites(D.par){
      s2 += chi[X].rdot(tmp[X]);
    }

    if(mynode()==0)
      gauge[0].set_value_at(g1, 50);
    gauge[0].mark_changed(ALL);
    foralldir(dir){
      onsites(ALL){
        rgauge[dir][X].represent(gauge[dir][X]);
      }
    }

    D.force(chi, psi, rmom, -1);
    rf = rmom[0].get_value_at(50);
    f = repr::project_force(rf);
    f1 = (s2-s1)/eps;
    f2 = (f*cmplx(0,1)*sun::generator(ng)).trace().re;
    diff = f2-f1;

    if(mynode()==0) {
      //hila::output << "Action 1 " << s1 << "\n";
      //hila::output << "Action 2 " << s2 << "\n";
      //hila::output << "Calculated deriv " << f2 << "\n";
      //hila::output << "Actual deriv " << (s2-s1)/eps << "\n";
      //hila::output << "Fermion dg deriv " << ng << " diff " << diff << "\n";
      assert( diff*diff < eps*10 && "Fermion dg deriv" );
    }



    foralldir(dir){
      momentum[dir][ALL] = 0;
    }

    if(mynode()==0){
      gauge[0].set_value_at(g12,50);
    }
    gauge[0].mark_changed(ALL);
    s2 = fa.action();

    if(mynode()==0)
      gauge[0].set_value_at(g1, 50);
    gauge[0].mark_changed(ALL);
    s1 = fa.action();

    fa.force_step(1.0);
    f = momentum[0].get_value_at(50);
    f1 = (s2-s1)/eps;
    f2 = (f*cmplx(0,1)*sun::generator(ng)).trace().re;
    diff = f2-f1;

    if(mynode()==0) {
      //hila::output << "Action 1 " << s1 << "\n";
      //hila::output << "Action 2 " << s2 << "\n";
      //hila::output << "Calculated force " << f2 << "\n";
      //hila::output << "Actual force " << (s2-s1)/eps << "\n";
      //hila::output << "Fermion force " << ng << " diff " << diff << "\n";
      assert( diff*diff < eps*10 && "Fermion force" );
    }
  }
}







int main(int argc, char **argv){

  /* Use a smaller lattice size since
   * the inversion takes a while */
  #if NDIM==1
  lattice->setup( 64, argc, argv );
  #elif NDIM==2
  lattice->setup( 32, 16, argc, argv );
  #elif NDIM==3
  lattice->setup( 32, 8, 8, argc, argv );
  #elif NDIM==4
  lattice->setup( 16, 8, 8, 8, argc, argv );
  #endif
  seed_random(2);

  // Test the force calculation by varying one gauge link
  // (Needs to be moved to tests)
  field<SU<N>> gauge[NDIM];
  field<SU<N>> momentum[NDIM];
  double eps = 1e-5;

  gauge_action<N> ga(gauge, momentum, 1.0);
  gauge_momentum_action<N> gma(gauge, momentum);
  foralldir(dir){
    onsites(ALL){
      gauge[dir][X].random();
    }
  }
  
  for(int ng = 0; ng < SU<N>::generator_count(); ng++){
    foralldir(dir){
      onsites(ALL){
        momentum[dir][X] = 0;
      }
    }
    SU<N> g1 = gauge[0].get_value_at(50);
    SU<N> h = SU<N>(1) + cmplx(0,eps)* SU<N>::generator(ng);
    SU<N> g12 = h*g1;

    double s1 = plaquette_sum(gauge);

    if(mynode()==0)
      gauge[0].set_value_at(g12,50);
    gauge[0].mark_changed(ALL);

    double s2 = plaquette_sum(gauge);

    if(mynode()==0)
      gauge[0].set_value_at(g1, 50);
    gauge[0].mark_changed(ALL);

    gauge_force(gauge, momentum, 1.0/N);
    SU<N> f = momentum[0].get_value_at(50);
    double diff = (f*cmplx(0,1)*SU<N>::generator(ng)).trace().re - (s2-s1)/eps;

    if(mynode()==0) {
      //hila::output << "Force 1 " << (f*SU<N>::generator(ng)).trace().re << "\n";
      //hila::output << "Force 2 " << (s2-s1)/eps << "\n";
      //hila::output << "Force " << ng << " diff " << diff << "\n";
      h = cmplx(0,1)*SU<N>::generator(ng);
      assert( diff*diff < eps*10 && "Gauge force" );
    }
  }

  using VEC=SU_vector<N, double>;
  using SUN=SU<N, double>;
  using adj=adjoint<N, double>;
  using adjvec=SU_vector<adj::size, double>;
  using sym=symmetric<N, double>;
  using symvec=SU_vector<sym::size, double>;
  using asym=antisymmetric<N, double>;
  using asymvec=SU_vector<asym::size, double>;

  output0 << "Checking staggered forces:\n";
  check_forces<dirac_staggered<VEC, SUN>, SUN>(1.5);
  output0 << "Checking evenodd preconditioned staggered forces:\n";
  check_forces<dirac_staggered_evenodd<VEC, SUN>, SUN>(1.5);
  output0 << "Checking adjoint staggered forces:\n";
  check_represented_forces<dirac_staggered_evenodd<adjvec, adj>, SUN, adj>(1.5);
  output0 << "Checking symmetric staggered forces:\n";
  check_represented_forces<dirac_staggered_evenodd<symvec, sym>, SUN, sym>(1.5);
  output0 << "Checking antisymmetric staggered forces:\n";
  check_represented_forces<dirac_staggered_evenodd<asymvec, asym>, SUN, asym>(1.5);
  output0 << "Checking Wilson forces:\n";
  check_forces<Dirac_Wilson<N, double, SUN>, SUN>(0.05);
  output0 << "Checking evenodd preconditioned Wilson forces:\n";
  check_forces<Dirac_Wilson_evenodd<N, double, SUN>, SUN>(0.05);
  output0 << "Checking adjoint Wilson forces:\n";
  check_represented_forces<Dirac_Wilson_evenodd<adj::size, double, adj>, SUN, adj>(0.05);
  output0 << "Checking symmetric Wilson forces:\n";
  check_represented_forces<Dirac_Wilson_evenodd<sym::size, double, sym>, SUN, sym>(0.05);
  output0 << "Checking antisymmetric Wilson forces:\n";
  check_represented_forces<Dirac_Wilson_evenodd<asym::size, double, asym>, SUN, asym>(0.05);

  for(int ng = 0; ng < SU<N>::generator_count(); ng++){
    // Check also the momentum action and derivative
    ga.draw_gaussian_fields();



    double s1 = gma.action();
    SU<N> h = momentum[0].get_value_at(0);
    h += eps * cmplx(0,1)*SU<N>::generator(ng);
    if(mynode()==0)
      momentum[0].set_value_at(h, 0);
    double s2 = gma.action();

    double diff = (h*cmplx(0,1)*SU<N>::generator(ng)).trace().re + (s2-s1)/eps;
    if(mynode()==0) {
      //hila::output << "Mom 1 " << (h*SU<N>::generator(ng)).trace().re << "\n";
      //hila::output << "Mom 2 " << (s2-s1)/eps << "\n";
      //hila::output << "Mom " << ng << " diff " << diff << "\n";
      h = cmplx(0,1)*SU<N>::generator(ng);
      assert( diff*diff < eps*10 && "Momentum derivative" );
    }
  }

  finishrun();
}

