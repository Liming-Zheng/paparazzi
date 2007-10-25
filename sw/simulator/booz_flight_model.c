#include "booz_flight_model.h"

#include <math.h>

#include "booz_flight_model_params.h"
#include "booz_flight_model_utils.h"
#include "airframe.h"

#include "6dof.h"


struct BoozFlightModel bfm;

//static void motor_model_derivative(VEC* x, VEC* u, VEC* xdot);

static VEC* booz_get_forces_body_frame(  VEC* X, VEC* M, MAT* dcm, VEC* omega_square);
static VEC* booz_get_moments_body_frame( VEC* X, VEC* M, VEC* omega_square );
static void booz_flight_model_get_derivatives(VEC* X, VEC* u, VEC* Xdot);

void booz_flight_model_init( void ) {
  bfm.time = 0.;
  bfm.bat_voltage = BAT_VOLTAGE;
  bfm.mot_voltage = v_get(SERVOS_NB);

  bfm.state =  v_get(BFMS_SIZE);

  /* constants */
  bfm.g_earth = v_get(AXIS_NB);
  bfm.g_earth->ve[AXIS_X] = 0.;
  bfm.g_earth->ve[AXIS_Y] = 0.;
  bfm.g_earth->ve[AXIS_Z] = G;

  bfm.thrust_factor = 0.5 * RHO * PROP_AREA * C_t * PROP_RADIUS * PROP_RADIUS;
  bfm.torque_factor = 0.5 * RHO * PROP_AREA * C_q * PROP_RADIUS * PROP_RADIUS;

  bfm.props_moment_matrix = m_get(AXIS_NB, SERVOS_NB);
  m_zero(bfm.props_moment_matrix);
  bfm.props_moment_matrix->me[AXIS_X][SERVO_MOTOR_LEFT]  =  L * bfm.thrust_factor;
  bfm.props_moment_matrix->me[AXIS_X][SERVO_MOTOR_RIGHT] = -L * bfm.thrust_factor;
  bfm.props_moment_matrix->me[AXIS_Y][SERVO_MOTOR_BACK]  = -L * bfm.thrust_factor;
  bfm.props_moment_matrix->me[AXIS_Y][SERVO_MOTOR_FRONT] =  L * bfm.thrust_factor;
  bfm.props_moment_matrix->me[AXIS_Z][SERVO_MOTOR_LEFT]  =  bfm.torque_factor;
  bfm.props_moment_matrix->me[AXIS_Z][SERVO_MOTOR_RIGHT] =  bfm.torque_factor;
  bfm.props_moment_matrix->me[AXIS_Z][SERVO_MOTOR_BACK]  =  -bfm.torque_factor;
  bfm.props_moment_matrix->me[AXIS_Z][SERVO_MOTOR_FRONT] =  -bfm.torque_factor;

  bfm.Inert = m_get(AXIS_NB, AXIS_NB);
  m_zero(bfm.Inert);
  bfm.Inert->me[AXIS_X][AXIS_X] = Ix;
  bfm.Inert->me[AXIS_Y][AXIS_Y] = Iy;
  bfm.Inert->me[AXIS_Z][AXIS_Z] = Iz;

  bfm.Inert_inv = m_get(AXIS_NB, AXIS_NB);
  m_zero(bfm.Inert_inv);
  bfm.Inert_inv->me[AXIS_X][AXIS_X] = 1./Ix;
  bfm.Inert_inv->me[AXIS_Y][AXIS_Y] = 1./Iy;
  bfm.Inert_inv->me[AXIS_Z][AXIS_Z] = 1./Iz;

}

void booz_flight_model_run( double dt, double* commands ) {
  

  int i;
  for (i=0; i<SERVOS_NB; i++)
    bfm.mot_voltage->ve[i] = bfm.bat_voltage * commands[i];
  //  rk4(motor_model_derivative, bfm.mot_omega, bfm.mot_voltage, dt); 
  rk4(booz_flight_model_get_derivatives, bfm.state, bfm.mot_voltage, dt);
  bfm.time += dt;
}



/* 
   compute the sum of external forces. 
   assumes that dcm and omega_square are already precomputed from X 
*/
static VEC* booz_get_forces_body_frame( VEC* X, VEC* F , MAT* dcm, VEC* omega_square) {
  // propeller thrust
  F->ve[AXIS_X] = 0;
  F->ve[AXIS_Y] = 0;
  F->ve[AXIS_Z] = -v_sum(omega_square) * bfm.thrust_factor;
  // gravity
  static VEC *g_body = VNULL;
  g_body = v_resize(g_body, AXIS_NB);
  g_body = mv_mlt(dcm, bfm.g_earth, g_body); 
  F = v_mltadd(F, g_body, MASS, F); 
  // body drag
  static VEC *v_body = VNULL;
  v_body = v_resize(v_body, AXIS_NB);
  v_body->ve[AXIS_X] = X->ve[BFMS_U];
  v_body->ve[AXIS_Y] = X->ve[BFMS_V];
  v_body->ve[AXIS_Z] = X->ve[BFMS_W];
  double norm_speed = v_norm2(v_body);
  F = v_mltadd(F, v_body, - norm_speed * C_d_body, F);
  return F;
}

/* 
   compute the sum of external moments. 
   assumes that omega_square is already precomputed from X 
*/
static VEC* booz_get_moments_body_frame( VEC* X, VEC* M, VEC* omega_square ) {
  M =  mv_mlt(bfm.props_moment_matrix, omega_square, M);
  return M;
}

static void booz_flight_model_get_derivatives(VEC* X, VEC* u, VEC* Xdot) {

  /* square of prop rotational speeds */
  static VEC *omega_square = VNULL;
  omega_square = v_resize(omega_square,SERVOS_NB);
  omega_square->ve[SERVO_MOTOR_BACK]  = X->ve[BFMS_OM_B];
  omega_square->ve[SERVO_MOTOR_FRONT] = X->ve[BFMS_OM_F];
  omega_square->ve[SERVO_MOTOR_RIGHT] = X->ve[BFMS_OM_R];
  omega_square->ve[SERVO_MOTOR_LEFT]  = X->ve[BFMS_OM_L];
  omega_square = v_star(omega_square, omega_square, omega_square);
  /* extract eulers angles from state */
  static VEC *eulers = VNULL;
  eulers = v_resize(eulers, AXIS_NB);
  eulers->ve[EULER_PHI]   =  X->ve[BFMS_PHI];
  eulers->ve[EULER_THETA] =  X->ve[BFMS_THETA];
  eulers->ve[EULER_PSI]   =  X->ve[BFMS_PSI];
  /* direct cosine matrix ( inertial to body )*/
  static MAT *dcm = MNULL;
  dcm = m_resize(dcm,AXIS_NB, AXIS_NB);
  dcm = dcm_of_eulers(eulers, dcm);
  /* transpose of dcm ( body to inertial ) */
  static MAT *dcm_t = MNULL;
  dcm_t = m_resize(dcm_t,AXIS_NB, AXIS_NB);
  dcm_t = m_transp(dcm, dcm_t);

  /* compute external forces */
  static VEC *f_body = VNULL;
  f_body = v_resize(f_body, AXIS_NB);
  f_body = booz_get_forces_body_frame( X, f_body , dcm, omega_square);

  /* compute external moments */
  static VEC *m_body = VNULL;
  m_body = v_resize(m_body, AXIS_NB);
  m_body = booz_get_moments_body_frame( X, m_body, omega_square);

  /* derivatives of position */
  static VEC *speed_body = VNULL;
  speed_body = v_resize(speed_body, AXIS_NB);
  speed_body->ve[AXIS_X] = X->ve[BFMS_U];
  speed_body->ve[AXIS_Y] = X->ve[BFMS_V];
  speed_body->ve[AXIS_Z] = X->ve[BFMS_W];
  static VEC *speed_earth = VNULL;
  speed_earth = v_resize(speed_earth, AXIS_NB);
  speed_earth = mv_mlt(dcm_t, speed_body,speed_earth);
  Xdot->ve[BFMS_X] = speed_earth->ve[AXIS_X];
  Xdot->ve[BFMS_Y] = speed_earth->ve[AXIS_Y];
  Xdot->ve[BFMS_Z] = speed_earth->ve[AXIS_Z];

  /* derivatives of speed           */
  /* extracts body rates from state */
  static VEC *rate_body = VNULL;
  rate_body = v_resize(rate_body, AXIS_NB);
  rate_body->ve[AXIS_P] = X->ve[BFMS_P];
  rate_body->ve[AXIS_Q] = X->ve[BFMS_Q];
  rate_body->ve[AXIS_R] = X->ve[BFMS_R];
  /* Newton in body frame    */
  static VEC *fict_f = VNULL;
  fict_f = v_resize(fict_f, AXIS_NB);
  fict_f = out_prod(speed_body, rate_body, fict_f);
  Xdot->ve[BFMS_U] = 1./MASS * f_body->ve[AXIS_X] + fict_f->ve[AXIS_X];
  Xdot->ve[BFMS_V] = 1./MASS * f_body->ve[AXIS_Y] + fict_f->ve[AXIS_Y];
  Xdot->ve[BFMS_W] = 1./MASS * f_body->ve[AXIS_Z] + fict_f->ve[AXIS_Z];

  /* derivatives of eulers   */
  double sinPHI   = sin(X->ve[BFMS_PHI]);
  double cosPHI   = cos(X->ve[BFMS_PHI]);
  double cosTHETA = cos(X->ve[BFMS_THETA]);
  double tanTHETA = tan(X->ve[BFMS_THETA]);
  static MAT *euler_dot_of_pqr = MNULL;
  euler_dot_of_pqr = m_resize(euler_dot_of_pqr,AXIS_NB, AXIS_NB);
  euler_dot_of_pqr->me[EULER_PHI][AXIS_P] = 1.;
  euler_dot_of_pqr->me[EULER_PHI][AXIS_Q] = sinPHI*tanTHETA;
  euler_dot_of_pqr->me[EULER_PHI][AXIS_R] = cosPHI*tanTHETA;
  euler_dot_of_pqr->me[EULER_THETA][AXIS_P] = 0.;
  euler_dot_of_pqr->me[EULER_THETA][AXIS_Q] = cosPHI;
  euler_dot_of_pqr->me[EULER_THETA][AXIS_R] = -sinPHI;
  euler_dot_of_pqr->me[EULER_PSI][AXIS_P] = 0.;
  euler_dot_of_pqr->me[EULER_PSI][AXIS_Q] = sinPHI/cosTHETA;
  euler_dot_of_pqr->me[EULER_PSI][AXIS_R] = cosPHI/cosTHETA;
  static VEC *euler_dot = VNULL;
  euler_dot = v_resize(euler_dot, AXIS_NB);
  euler_dot = mv_mlt(euler_dot_of_pqr, rate_body, euler_dot); 
  Xdot->ve[BFMS_PHI] = euler_dot->ve[EULER_PHI];
  Xdot->ve[BFMS_THETA] = euler_dot->ve[EULER_THETA];
  Xdot->ve[BFMS_PSI] = euler_dot->ve[EULER_PSI];

  /* derivatives of rates    */
  /* Newton in body frame    */
  static VEC *i_omega = VNULL;
  i_omega = v_resize(i_omega, AXIS_NB);
  i_omega = mv_mlt(bfm.Inert, rate_body, i_omega);
  static VEC *omega_i_omega = VNULL;
  omega_i_omega = v_resize(omega_i_omega, AXIS_NB);
  omega_i_omega = out_prod(rate_body, i_omega, omega_i_omega);
  static VEC *m_tot = VNULL;
  m_tot = v_resize(m_tot, AXIS_NB);
  m_tot = v_sub(m_body, omega_i_omega, m_tot);
  static VEC *I_inv_m_tot = VNULL;
  I_inv_m_tot = v_resize(I_inv_m_tot, AXIS_NB);
  I_inv_m_tot = mv_mlt(bfm.Inert_inv, m_tot, I_inv_m_tot);
  Xdot->ve[BFMS_P] = I_inv_m_tot->ve[AXIS_P];
  Xdot->ve[BFMS_Q] = I_inv_m_tot->ve[AXIS_Q];
  Xdot->ve[BFMS_R] = I_inv_m_tot->ve[AXIS_R];

  /* derivatives of motors rpm */
  /* omega_dot = -1/THAU*omega - Kq*omega^2 + Kv/THAU * V */
  Xdot->ve[BFMS_OM_B] = -1./THAU * X->ve[BFMS_OM_B] - Kq * omega_square->ve[SERVO_MOTOR_BACK] + Kv/THAU * u->ve[SERVO_MOTOR_BACK];
  Xdot->ve[BFMS_OM_F] = -1./THAU * X->ve[BFMS_OM_F] - Kq * omega_square->ve[SERVO_MOTOR_FRONT] + Kv/THAU * u->ve[SERVO_MOTOR_FRONT];
  Xdot->ve[BFMS_OM_R] = -1./THAU * X->ve[BFMS_OM_R] - Kq * omega_square->ve[SERVO_MOTOR_RIGHT] + Kv/THAU * u->ve[SERVO_MOTOR_RIGHT];
  Xdot->ve[BFMS_OM_L] = -1./THAU * X->ve[BFMS_OM_L] - Kq * omega_square->ve[SERVO_MOTOR_LEFT] + Kv/THAU * u->ve[SERVO_MOTOR_LEFT];

}






#if 0
static void motor_model_derivative(VEC* x, VEC* u, VEC* xdot) {
  static VEC *temp1 = VNULL;
  static VEC *temp2 = VNULL;
  temp1 = v_resize(temp1,SERVOS_NB);
  temp2 = v_resize(temp2,SERVOS_NB);

  // omega_dot = -1/THAU*omega - Kq*omega^2 + Kv/THAU * V;
  temp1 = sv_mlt(-1./THAU, x, temp1);        /* temp1 = -1/THAU * x       */
  temp2 = v_star(x, x, temp2);               /* temp2 = x^2               */
  xdot = v_mltadd(temp1, temp2, -Kq, xdot);  /* xdot = temp1 - Kq*temp2   */ 
  xdot = v_mltadd(xdot, u, Kv/THAU, xdot);   /* xdot = xdot + Kv/THAU * u */ 
}
#endif
