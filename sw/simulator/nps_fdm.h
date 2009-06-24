#ifndef NPS_FDM
#define NPS_FDM


#include "std.h"
#include "pprz_geodetic_double.h"
#include "pprz_algebra_double.h"

/*
  Notations for fdm variables
  ---------------------------
  coordinate system [ frame ] name

  ecef_inertial_vel is the time derivative of position
  with respect to inertial frame expressed in ECEF ( Earth Centered Earth Fixed)
  coordinate system.
*/

struct NpsFdm {
  
  bool_t on_ground;

  struct EcefCoor_d  ecef_pos;
  struct EcefCoor_d  ecef_inertial_vel;
  struct EcefCoor_d  ecef_inertial_accel;
  struct DoubleVect3 body_inertial_vel;   /* aka UVW */
  struct DoubleVect3 body_inertial_accel; 

  struct NedCoor_d ltp_pos;
  struct NedCoor_d ltp_inertial_vel;

  struct DoubleQuat   ltp_to_body_quat;
  struct DoubleEulers ltp_to_body_eulers;
  struct DoubleRates body_rate;
  struct DoubleRates body_rate_dot;

  struct DoubleVect3 ltp_g;
  struct DoubleVect3 ltp_h;

};

extern struct NpsFdm fdm;

extern void nps_fdm_init(double dt);
extern void nps_fdm_run_step(double* commands);

#endif /* NPS_FDM */
