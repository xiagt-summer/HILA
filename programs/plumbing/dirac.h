#ifndef __DIRAC_H__
#define __DIRAC_H__

#include "../plumbing/defs.h"
#include "../datatypes/cmplx.h"
#include "../datatypes/general_matrix.h"
#include "../plumbing/field.h"


inline void init_staggered_eta(field<double> staggered_eta[NDIM]){
  // Initialize the staggered eta field
  foralldir(d){
    onsites(ALL){
      element<coordinate_vector> l = X.coordinates();
      element<int> sumcoord = 0;
      for(int d2=XUP;d2<d;d2++){
        sumcoord += l[d2];
      }
      // +1 if sumcoord divisible by 2, -1 otherwise
      // If statements not yet implemented for vectors
      staggered_eta[d][X] = (sumcoord%2)*2-1; 
    }
  }
}






template<typename mtype, typename vtype>
void dirac_staggered_apply(
  const field<mtype> *gauge,
  const double mass,
  const field<vtype> &v_in,
  field<vtype> &v_out,
  field<double> staggered_eta[NDIM],
  field<vtype> vtemp[NDIM])
{
  // Start getting neighbours
  foralldir(dir){
    v_in.start_get(dir);
  }

  // Apply the mass diagonally
  v_out[ALL] = mass * v_in[X];

  // Run neighbour fetches and multiplications
  foralldir(dir){
    direction odir = opp_dir( (direction)dir );
    // First mulltiply the by conjugate before communicating the matrix
    vtemp[dir][ALL] = v_in[X]*gauge[dir][X].conjugate();
    vtemp[dir].start_get(odir);
  }
  foralldir(dir){
    direction odir = opp_dir( (direction)dir );
    v_out[ALL] = v_out[X] + 0.5 * staggered_eta[dir][X] * (
      v_in[X+dir]*gauge[dir][X] - vtemp[dir][X+odir]
    );
  }
}


template<typename mtype, typename vtype>
void dirac_staggered_dagger(
  const field<mtype> *gauge,
  const double mass,
  const field<vtype> &v_in,
  field<vtype> &v_out,
  field<double> staggered_eta[NDIM],
  field<vtype> vtemp[NDIM])
{
  // Start getting neighbours
  foralldir(dir){
    v_in.start_get(dir);
  }

  // Apply the mass diagonally
  v_out[ALL] = mass * v_in[X];

  // Run neighbour fetches and multiplications
  foralldir(dir){
    direction odir = opp_dir( (direction)dir );
    // First mulltiply the by conjugate before communicating the matrix
    vtemp[dir][ALL] = v_in[X]*gauge[dir][X].conjugate();
    vtemp[dir].start_get(odir);
  }
  foralldir(dir){
    direction odir = opp_dir( (direction)dir );
    v_out[ALL] = v_out[X] - 0.5 * staggered_eta[dir][X] * (
      v_in[X+dir]*(gauge[dir][X]) - vtemp[dir][X+odir]
    );
  }
}






template<typename vector, typename matrix>
class dirac_staggered {
  private:
    double mass;
    field<vector> vtemp[NDIM];
    field<double> staggered_eta[NDIM];

    // Note array of fields, changes with the field
    field<matrix> *gauge;
  
  public:

    // Constructor: initialize mass, gauge and eta
    dirac_staggered(dirac_staggered &d) {
      mass = d.mass;
      gauge = d.gauge;

      // Initialize the eta field (Share this?)
      init_staggered_eta(staggered_eta);
    }
  
    // Constructor: initialize mass, gauge and eta
    dirac_staggered(double m, field<matrix> *U) {
      // Set mass and gauge field
      mass = m;
      gauge = (field<matrix>*) U;

      // Initialize the eta field
      init_staggered_eta(staggered_eta);
    }

    // Update mass
    void set_mass(double m){
      mass = m;
    }



    // Applies the operator to in
    void apply( const field<vector> & in, field<vector> & out){
      dirac_staggered_apply(gauge, mass, in, out, staggered_eta, vtemp);

    }

    // Applies the conjugate of the operator
    void dagger( const field<vector> & in, field<vector> & out){
      dirac_staggered_dagger(gauge, mass, in, out, staggered_eta, vtemp);
    }
};

// Multiplying from the left applies the standard Dirac operator
template<typename vector, typename matrix>
field<vector> operator* (dirac_staggered<field<vector>, field<matrix>> D, const field<vector> & in) {
  field<vector> out;
  D.apply(in, out);
  return out;
}

// Multiplying from the right applies the conjugate
template<typename vector, typename matrix>
field<vector> operator* (const field<vector> & in, dirac_staggered<field<vector>, field<matrix>> D) {
  field<vector> out;
  D.dagger(in, out);
  return out;
}


#endif