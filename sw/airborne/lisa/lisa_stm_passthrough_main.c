/*
 * $Id$
 *
 * Copyright (C) 2010 The Paparazzi Team
 *
 * This file is part of Paparazzi.
 *
 * Paparazzi is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * Paparazzi is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Paparazzi; see the file COPYING.  If not, write to
 * the Free Software Foundation, 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 */

#include "init_hw.h"
#include "sys_time.h"
#include "downlink.h"
#include "booz/booz2_commands.h"
#include "booz/booz_actuators.h"
#include "booz/actuators/booz_actuators_pwm.h"
#include "booz/booz_imu.h"
#include "booz/booz_radio_control.h"
#include "lisa/lisa_overo_link.h"

#include "stm32/can.h"
#include "csc_msg_def.h"
#include "csc_protocol.h"

#include "lisa/lisa_baro.h"


static inline void main_init(void);
static inline void main_periodic(void);
static inline void main_event(void);

static inline void on_gyro_accel_event(void);
static inline void on_mag_event(void);

static inline void on_overo_link_msg_received(void);
static inline void on_overo_link_lost(void);
static inline void on_overo_link_crc_failed(void);

static inline void on_rc_message(void);
static inline void on_vane_msg(void *data);

static bool_t new_radio_msg;
static bool_t new_baro_diff;
static bool_t new_baro_abs;
static bool_t new_vane;

struct CscVaneMsg csc_vane_msg;

int main(void) {

	main_init();

	while (1) {
		if (sys_time_periodic())
			main_periodic();
		main_event();
	}

	return 0;
}

static inline void main_init(void) {

	hw_init();
	sys_time_init();
	booz_imu_init();
	baro_init();
	radio_control_init();
	booz_actuators_init();
	overo_link_init();
	cscp_init();
//	csc_bat_monitor_init(); 

	cscp_register_callback(CSC_VANE_MSG_ID, on_vane_msg, (void *)&csc_vane_msg);
	new_radio_msg = FALSE;
}

static inline void main_periodic(void) {

	booz_imu_periodic();
	OveroLinkPeriodic(on_overo_link_lost);
//	csc_bat_monitor_periodic(); 

	RunOnceEvery(10, {
			LED_PERIODIC(); 
			DOWNLINK_SEND_ALIVE(DefaultChannel, 16, MD5SUM);
			radio_control_periodic();
	});

  RunOnceEvery(2, {baro_periodic();});

}

static inline void on_rc_message(void) {
	new_radio_msg = TRUE;
}

static inline void on_overo_link_msg_received(void) {

	/* IMU up */ 
	overo_link.up.msg.valid.imu = 1;
	RATES_COPY(overo_link.up.msg.gyro, booz_imu.gyro);
	VECT3_COPY(overo_link.up.msg.accel, booz_imu.accel);
	VECT3_COPY(overo_link.up.msg.mag, booz_imu.mag);
	
	/* RC up */
	overo_link.up.msg.valid.rc  = new_radio_msg;
	new_radio_msg = FALSE;

	overo_link.up.msg.rc_pitch  = radio_control.values[RADIO_CONTROL_PITCH];
	overo_link.up.msg.rc_roll   = radio_control.values[RADIO_CONTROL_ROLL];
	overo_link.up.msg.rc_yaw    = radio_control.values[RADIO_CONTROL_YAW];
	overo_link.up.msg.rc_thrust = radio_control.values[RADIO_CONTROL_THROTTLE];
	overo_link.up.msg.rc_mode   = radio_control.values[RADIO_CONTROL_MODE];
#ifdef RADIO_CONTROL_KILL
	overo_link.up.msg.rc_kill   = radio_control.values[RADIO_CONTROL_KILL];
#endif
#ifdef RADIO_CONTROL_GEAR
	overo_link.up.msg.rc_gear   = radio_control.values[RADIO_CONTROL_GEAR];
#endif
	overo_link.up.msg.rc_aux3   = radio_control.values[RADIO_CONTROL_AUX3];
	overo_link.up.msg.rc_aux4   = radio_control.values[RADIO_CONTROL_AUX4];
	overo_link.up.msg.rc_status = radio_control.status;
	
	overo_link.up.msg.stm_msg_cnt     = overo_link.msg_cnt;
	overo_link.up.msg.stm_crc_err_cnt = overo_link.crc_err_cnt;

	/* baro up */
	overo_link.up.msg.valid.pressure_differential = new_baro_diff;
	overo_link.up.msg.valid.pressure_absolute     = new_baro_abs;
	new_baro_diff = FALSE;
	new_baro_abs  = FALSE;
	overo_link.up.msg.pressure_differential = baro.diff_raw;
	overo_link.up.msg.pressure_absolute     = baro.abs_raw;
	
	overo_link.up.msg.valid.vane =  new_vane;
	new_vane = FALSE;
	overo_link.up.msg.vane_angle1 = csc_vane_msg.vane_angle1;
	overo_link.up.msg.vane_angle2 = csc_vane_msg.vane_angle2;

	/* pwm acuators down */
	for (int i = 0; i < LISA_PWM_OUTPUT_NB; i++) { 
		booz_actuators_pwm_values[i] = overo_link.down.msg.pwm_outputs_usecs[i];
	}
	booz_actuators_pwm_commit();

}

static inline void on_overo_link_lost(void) {
}

static inline void on_overo_link_crc_failed(void) {
}

static inline void on_gyro_accel_event(void) {
	BoozImuScaleGyro(booz_imu);
	BoozImuScaleAccel(booz_imu);
}

static inline void on_mag_event(void) {
	BoozImuScaleMag(booz_imu);
}

static inline void on_vane_msg(void *data) { 
	new_vane = TRUE;
	int zero = 0;
	DOWNLINK_SEND_VANE_SENSOR(DefaultChannel, 
														&(csc_vane_msg.vane_angle1), 
														&zero, 
														&zero, 
														&zero, 
														&zero, 
														&csc_vane_msg.vane_angle2, 
														&zero, 
														&zero, 
														&zero, 
														&zero);
}

static inline void main_on_baro_diff(void) {
	new_baro_diff = TRUE;
}

static inline void main_on_baro_abs(void) {
	new_baro_abs = TRUE;
}

static inline void main_event(void) {
	
	BoozImuEvent(on_gyro_accel_event, on_mag_event);
	BaroEvent(main_on_baro_abs, main_on_baro_diff);
	OveroLinkEvent(on_overo_link_msg_received, on_overo_link_crc_failed);
	RadioControlEvent(on_rc_message);
	cscp_event();
}


