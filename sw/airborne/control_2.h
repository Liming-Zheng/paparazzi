#ifndef CONTROL_H
#define CONTROL_H

#include "paparazzi.h"
#include "airframe.h"
#include "radio_control.h"

extern pprz_t control_commands[];

// twinjet1
static inline void control_process_radio_control_manual( void ) {
  control_commands[COMMAND_THROTTLE] = rc_values[RADIO_THROTTLE];
  control_commands[COMMAND_ROLL] = rc_values[RADIO_ROLL];
  control_commands[COMMAND_PITCH] = rc_values[RADIO_PITCH];
}

// twinstar_test
#ifdef 0
static inline void control_process_radio_control_manual( void ) {
  control_commands[COMMAND_THROTTLE] = rc_values[RADIO_THROTTLE];
  control_commands[COMMAND_ROLL] = rc_values[RADIO_ROLL];
  control_commands[COMMAND_PITCH] = rc_values[RADIO_PITCH];
  control_commands[COMMAND_YAW] = rc_values[RADIO_YAW];
}
#endif

#ifdef 0
// gorazoptere_test
static inline void control_process_radio_control_manual( void ) {
  control_commands[COMMAND_THROTTLE] = rc_values[RADIO_THROTTLE];
  control_setpoint_rolldot = rc_values[RADIO_ROLL]*0.015;
  control_setpoint_pitchdot = rc_values[RADIO_ROLL]*0.015;
  control_setpoint_yawdot = rc_values[RADIO_ROLL]*0.015;
}

static inline void control_process_radio_control_auto1( void ) {
  control_commands[COMMAND_THROTTLE] = rc_values[RADIO_THROTTLE];
  control_setpoint_roll = rc_values[RADIO_ROLL]*0.015;
  control_setpoint_pitch = rc_values[RADIO_ROLL]*0.015;
  control_setpoint_yaw = rc_values[RADIO_ROLL]*0.015;
}

static inline void control_run_manual( void ) {
  


}
#endif
#endif /* CONTROL_H */
