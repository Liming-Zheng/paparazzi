#ifndef BOOZ_TELEMETRY_H
#define BOOZ_TELEMETRY_H

#include "std.h"
#include "messages.h"
#include "periodic.h"
#include "uart.h"

#include "radio_control.h"
#include "actuators.h"
#include "booz_link_mcu.h"
#include "booz_estimator.h"
#include "booz_autopilot.h"
#include "booz_control.h"

#include "actuators_buss_twi_blmc_hw.h"

#include "settings.h"

#include "downlink.h"

#define PERIODIC_SEND_BOOZ_STATUS()					\
  DOWNLINK_SEND_BOOZ_STATUS(&booz_link_mcu_nb_err,			\
			    &booz_link_mcu_status,			\
			    &rc_status, \
			    &booz_autopilot_mode, \
			    &booz_autopilot_mode, \
			    &cpu_time_sec, \
			    &buss_twi_blmc_nb_err)

#define PERIODIC_SEND_BOOZ_FD()						\
  DOWNLINK_SEND_BOOZ_FD(&booz_estimator_p,				\
			&booz_estimator_q,				\
			&booz_estimator_r,				\
			&booz_estimator_phi,				\
			&booz_estimator_theta,				\
			&booz_estimator_psi); 

#define PERIODIC_SEND_BOOZ_DEBUG()					\
  DOWNLINK_SEND_BOOZ_DEBUG(&booz_control_p_sp,				\
			   &booz_control_q_sp,				\
			   &booz_control_r_sp,				\
			   &booz_control_power_sp); 

#define PERIODIC_SEND_ACTUATORS()			\
  DOWNLINK_SEND_ACTUATORS(SERVOS_NB, actuators);

#define PERIODIC_SEND_BOOZ_RATE_LOOP()					\
  DOWNLINK_SEND_BOOZ_RATE_LOOP(&booz_estimator_p, &booz_control_p_sp,	\
			       &booz_estimator_q, &booz_control_q_sp,	\
			       &booz_estimator_r, &booz_control_r_sp ); 

#define PERIODIC_SEND_BOOZ_ATT_LOOP()					\
  DOWNLINK_SEND_BOOZ_ATT_LOOP(&booz_estimator_phi, &booz_control_phi_sp, \
			      &booz_estimator_theta, &booz_control_theta_sp); 

#define PERIODIC_SEND_BOOZ_UF_RATES() \
  DOWNLINK_SEND_BOOZ_UF_RATES(&booz_estimator_uf_p, \
			      &booz_estimator_uf_q, \
			      &booz_estimator_uf_r); 

#define PERIODIC_SEND_BOOZ_CMDS() DOWNLINK_SEND_BOOZ_CMDS(&buss_twi_blmc_motor_power[SERVO_MOTOR_FRONT],\
							  &buss_twi_blmc_motor_power[SERVO_MOTOR_BACK],	\
							  &buss_twi_blmc_motor_power[SERVO_MOTOR_LEFT],	\
							  &buss_twi_blmc_motor_power[SERVO_MOTOR_RIGHT]); 

#define PERIODIC_SEND_DL_VALUE() PeriodicSendDlValue()

extern uint8_t telemetry_mode_Controller;

static inline void booz_controller_telemetry_periodic_task(void) {
  PeriodicSendController()
}


#endif /* BOOZ_TELEMETRY_H */
