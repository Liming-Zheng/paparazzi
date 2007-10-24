#include "booz_control.h"

#include "booz_estimator.h"
#include "radio_control.h"

float booz_control_p_sp;
float booz_control_q_sp;
float booz_control_r_sp;
float booz_control_power_sp;

float booz_control_rate_pq_pgain;
float booz_control_rate_pq_dgain;
float booz_control_rate_r_pgain;
float booz_control_rate_r_dgain;
float booz_control_rate_last_err_p;
float booz_control_rate_last_err_q;
float booz_control_rate_last_err_r;

pprz_t booz_control_commands[COMMANDS_NB];

#define BOOZ_CONTROL_RATE_PQ_PGAIN -1000.
#define BOOZ_CONTROL_RATE_PQ_DGAIN 0.

#define BOOZ_CONTROL_RATE_R_PGAIN -100.
#define BOOZ_CONTROL_RATE_R_DGAIN 0.

/* setpoints for max stick throw in degres per second */
#define BOOZ_CONTROL_RATE_PQ_MAX_SP 200.
#define BOOZ_CONTROL_RATE_R_MAX_SP  100.


float booz_control_phi_sp;
float booz_control_theta_sp;
float booz_control_attitude_phi_theta_pgain;
float booz_control_attitude_phi_theta_dgain;

#define BOOZ_CONTROL_ATTITUDE_PHI_THETA_PGAIN -15000.
#define BOOZ_CONTROL_ATTITUDE_PHI_THETA_DGAIN -15000.

/* setpoints for max stick throw in degres */
#define BOOZ_CONTROL_ATTITUDE_PHI_THETA_MAX_SP 30.



void booz_control_init(void) {

  booz_control_p_sp = 0.;
  booz_control_q_sp = 0.;
  booz_control_r_sp = 0.;
  booz_control_power_sp = 0.;

  booz_control_rate_last_err_p = 0.;
  booz_control_rate_last_err_q = 0.;
  booz_control_rate_last_err_r = 0.;

  booz_control_rate_pq_pgain = BOOZ_CONTROL_RATE_PQ_PGAIN;
  booz_control_rate_pq_dgain = BOOZ_CONTROL_RATE_PQ_DGAIN;
  booz_control_rate_r_pgain = BOOZ_CONTROL_RATE_R_PGAIN;
  booz_control_rate_r_dgain = BOOZ_CONTROL_RATE_R_DGAIN;


  booz_control_phi_sp = 0.;
  booz_control_theta_sp =0.;
  booz_control_attitude_phi_theta_pgain = BOOZ_CONTROL_ATTITUDE_PHI_THETA_PGAIN;
  booz_control_attitude_phi_theta_dgain = BOOZ_CONTROL_ATTITUDE_PHI_THETA_DGAIN;

}


void booz_control_rate_compute_setpoints(void) {

  booz_control_p_sp = -rc_values[RADIO_ROLL]  * RadOfDeg(BOOZ_CONTROL_RATE_PQ_MAX_SP)/MAX_PPRZ;
  booz_control_q_sp =  rc_values[RADIO_PITCH] * RadOfDeg(BOOZ_CONTROL_RATE_PQ_MAX_SP)/MAX_PPRZ;
  booz_control_r_sp = -rc_values[RADIO_YAW]   * RadOfDeg(BOOZ_CONTROL_RATE_R_MAX_SP)/MAX_PPRZ;
  booz_control_power_sp = rc_values[RADIO_THROTTLE] / (float)MAX_PPRZ;

}


void booz_control_rate_run(void) {

  const float rate_err_p = booz_estimator_p - booz_control_p_sp;
  const float rate_d_err_p = rate_err_p - booz_control_rate_last_err_p;
  booz_control_rate_last_err_p = rate_err_p;
  const float cmd_p = booz_control_rate_pq_pgain * ( rate_err_p + booz_control_rate_pq_dgain * rate_d_err_p );

  const float rate_err_q = booz_estimator_q - booz_control_q_sp;
  const float rate_d_err_q = rate_err_q - booz_control_rate_last_err_q;
  booz_control_rate_last_err_q = rate_err_q;
  const float cmd_q = booz_control_rate_pq_pgain * ( rate_err_q + booz_control_rate_pq_dgain * rate_d_err_q );

  const float rate_err_r = booz_estimator_r - booz_control_r_sp;
  const float rate_d_err_r = rate_err_r - booz_control_rate_last_err_r;
  booz_control_rate_last_err_r = rate_err_r;
  const float cmd_r = booz_control_rate_r_pgain * ( rate_err_r + booz_control_rate_r_dgain * rate_d_err_r );

  booz_control_commands[COMMAND_P] = TRIM_PPRZ((int16_t)cmd_p);
  booz_control_commands[COMMAND_Q] = TRIM_PPRZ((int16_t)cmd_q);
  booz_control_commands[COMMAND_R] = TRIM_PPRZ((int16_t)cmd_r);
  booz_control_commands[COMMAND_THROTTLE] = TRIM_PPRZ((int16_t) (booz_control_power_sp * MAX_PPRZ));

}

void booz_control_attitude_compute_setpoints(void) {

  booz_control_phi_sp = -rc_values[RADIO_ROLL]  * RadOfDeg(BOOZ_CONTROL_ATTITUDE_PHI_THETA_MAX_SP)/MAX_PPRZ;
  booz_control_theta_sp =  rc_values[RADIO_PITCH] * RadOfDeg(BOOZ_CONTROL_ATTITUDE_PHI_THETA_MAX_SP)/MAX_PPRZ;
  booz_control_r_sp = -rc_values[RADIO_YAW]   * RadOfDeg(BOOZ_CONTROL_RATE_R_MAX_SP)/MAX_PPRZ;
  booz_control_power_sp = rc_values[RADIO_THROTTLE] / (float)MAX_PPRZ;

}

void booz_control_attitude_run(void) {

  const float att_err_phi = booz_estimator_phi - booz_control_phi_sp;
  const float cmd_p = booz_control_attitude_phi_theta_pgain *  att_err_phi + 
                      booz_control_attitude_phi_theta_dgain * booz_estimator_p ;

  const float att_err_theta = booz_estimator_theta - booz_control_theta_sp;
  const float cmd_q = booz_control_attitude_phi_theta_pgain * att_err_theta + 
                      booz_control_attitude_phi_theta_dgain * booz_estimator_q;

  const float rate_err_r = booz_estimator_r - booz_control_r_sp;
  const float rate_d_err_r = rate_err_r - booz_control_rate_last_err_r;
  booz_control_rate_last_err_r = rate_err_r;
  const float cmd_r = booz_control_rate_r_pgain * ( rate_err_r + booz_control_rate_r_dgain * rate_d_err_r );

  booz_control_commands[COMMAND_P] = TRIM_PPRZ((int16_t)cmd_p);
  booz_control_commands[COMMAND_Q] = TRIM_PPRZ((int16_t)cmd_q);
  booz_control_commands[COMMAND_R] = TRIM_PPRZ((int16_t)cmd_r);
  booz_control_commands[COMMAND_THROTTLE] = TRIM_PPRZ((int16_t) (booz_control_power_sp * MAX_PPRZ));


}
