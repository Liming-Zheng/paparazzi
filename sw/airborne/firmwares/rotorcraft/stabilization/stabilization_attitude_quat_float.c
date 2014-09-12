/*
 * Copyright (C) 2008-2009 Antoine Drouin <poinix@gmail.com>
 *
 * This file is part of paparazzi.
 *
 * paparazzi is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * paparazzi is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with paparazzi; see the file COPYING.  If not, write to
 * the Free Software Foundation, 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/** @file firmwares/rotorcraft/stabilization/stabilization_attitude_quat_float.c
 * @brief Quaternion attitude stabilization (floating point).
 */

#include "generated/airframe.h"

#include "firmwares/rotorcraft/stabilization.h"
#include "firmwares/rotorcraft/stabilization/stabilization_attitude_rc_setpoint.h"
#include "firmwares/rotorcraft/stabilization/stabilization_attitude_quat_transformations.h"

#include "std.h"
#include "paparazzi.h"
#include "math/pprz_algebra_float.h"
#include "math/pprz_algebra_int.h"
#include "state.h"

struct FloatAttitudeGains stabilization_gains[STABILIZATION_ATTITUDE_GAIN_NB];

struct FloatQuat stabilization_att_sum_err_quat;
struct FloatEulers stabilization_att_sum_err;

struct FloatRates last_body_rate;
struct FloatRates body_rate_d;

float stabilization_att_fb_cmd[COMMANDS_NB];
float stabilization_att_ff_cmd[COMMANDS_NB];

static int gain_idx = STABILIZATION_ATTITUDE_GAIN_IDX_DEFAULT;

static const float phi_pgain[] = STABILIZATION_ATTITUDE_PHI_PGAIN;
static const float theta_pgain[] = STABILIZATION_ATTITUDE_THETA_PGAIN;
static const float psi_pgain[] = STABILIZATION_ATTITUDE_PSI_PGAIN;

static const float phi_dgain[] = STABILIZATION_ATTITUDE_PHI_DGAIN;
static const float theta_dgain[] = STABILIZATION_ATTITUDE_THETA_DGAIN;
static const float psi_dgain[] = STABILIZATION_ATTITUDE_PSI_DGAIN;

static const float phi_igain[] = STABILIZATION_ATTITUDE_PHI_IGAIN;
static const float theta_igain[] = STABILIZATION_ATTITUDE_THETA_IGAIN;
static const float psi_igain[] = STABILIZATION_ATTITUDE_PSI_IGAIN;

static const float phi_ddgain[] = STABILIZATION_ATTITUDE_PHI_DDGAIN;
static const float theta_ddgain[] = STABILIZATION_ATTITUDE_THETA_DDGAIN;
static const float psi_ddgain[] = STABILIZATION_ATTITUDE_PSI_DDGAIN;

static const float phi_dgain_d[] = STABILIZATION_ATTITUDE_PHI_DGAIN_D;
static const float theta_dgain_d[] = STABILIZATION_ATTITUDE_THETA_DGAIN_D;
static const float psi_dgain_d[] = STABILIZATION_ATTITUDE_PSI_DGAIN_D;


#if defined COMMAND_ROLL_SURFACE && defined COMMAND_PITCH_SURFACE && defined COMMAND_YAW_SURFACE
#define HAS_SURFACE_COMMANDS 1
#endif

#ifdef HAS_SURFACE_COMMANDS
static const float phi_pgain_surface[] = STABILIZATION_ATTITUDE_PHI_PGAIN_SURFACE;
static const float theta_pgain_surface[] = STABILIZATION_ATTITUDE_THETA_PGAIN_SURFACE;
static const float psi_pgain_surface[] = STABILIZATION_ATTITUDE_PSI_PGAIN_SURFACE;

static const float phi_dgain_surface[] = STABILIZATION_ATTITUDE_PHI_DGAIN_SURFACE;
static const float theta_dgain_surface[] = STABILIZATION_ATTITUDE_THETA_DGAIN_SURFACE;
static const float psi_dgain_surface[] = STABILIZATION_ATTITUDE_PSI_DGAIN_SURFACE;

static const float phi_igain_surface[] = STABILIZATION_ATTITUDE_PHI_IGAIN_SURFACE;
static const float theta_igain_surface[] = STABILIZATION_ATTITUDE_THETA_IGAIN_SURFACE;
static const float psi_igain_surface[] = STABILIZATION_ATTITUDE_PSI_IGAIN_SURFACE;

static const float phi_ddgain_surface[] = STABILIZATION_ATTITUDE_PHI_DDGAIN_SURFACE;
static const float theta_ddgain_surface[] = STABILIZATION_ATTITUDE_THETA_DDGAIN_SURFACE;
static const float psi_ddgain_surface[] = STABILIZATION_ATTITUDE_PSI_DDGAIN_SURFACE;
#endif

#define IERROR_SCALE 1024

#if PERIODIC_TELEMETRY
#include "subsystems/datalink/telemetry.h"

static void send_att(void) {
  struct FloatRates* body_rate = stateGetBodyRates_f();
  struct FloatEulers* att = stateGetNedToBodyEulers_f();
  DOWNLINK_SEND_STAB_ATTITUDE_FLOAT(DefaultChannel, DefaultDevice,
                                    &(body_rate->p), &(body_rate->q), &(body_rate->r),
                                    &(att->phi), &(att->theta), &(att->psi),
                                    &stab_att_sp_euler.phi,
                                    &stab_att_sp_euler.theta,
                                    &stab_att_sp_euler.psi,
                                    &stabilization_att_sum_err.phi,
                                    &stabilization_att_sum_err.theta,
                                    &stabilization_att_sum_err.psi,
                                    &stabilization_att_fb_cmd[COMMAND_ROLL],
                                    &stabilization_att_fb_cmd[COMMAND_PITCH],
                                    &stabilization_att_fb_cmd[COMMAND_YAW],
                                    &stabilization_att_ff_cmd[COMMAND_ROLL],
                                    &stabilization_att_ff_cmd[COMMAND_PITCH],
                                    &stabilization_att_ff_cmd[COMMAND_YAW],
                                    &stabilization_cmd[COMMAND_ROLL],
                                    &stabilization_cmd[COMMAND_PITCH],
                                    &stabilization_cmd[COMMAND_YAW],
                                    &body_rate_d.p, &body_rate_d.q, &body_rate_d.r);
}

static void send_att_ref(void) {
  DOWNLINK_SEND_STAB_ATTITUDE_REF_FLOAT(DefaultChannel, DefaultDevice,
                                        &stab_att_sp_euler.phi,
                                        &stab_att_sp_euler.theta,
                                        &stab_att_sp_euler.psi,
                                        &stab_att_ref_euler.phi,
                                        &stab_att_ref_euler.theta,
                                        &stab_att_ref_euler.psi,
                                        &stab_att_ref_rate.p,
                                        &stab_att_ref_rate.q,
                                        &stab_att_ref_rate.r,
                                        &stab_att_ref_accel.p,
                                        &stab_att_ref_accel.q,
                                        &stab_att_ref_accel.r);
}
#endif

void stabilization_attitude_init(void) {

  stabilization_attitude_ref_init();

  for (int i = 0; i < STABILIZATION_ATTITUDE_GAIN_NB; i++) {
    VECT3_ASSIGN(stabilization_gains[i].p, phi_pgain[i], theta_pgain[i], psi_pgain[i]);
    VECT3_ASSIGN(stabilization_gains[i].d, phi_dgain[i], theta_dgain[i], psi_dgain[i]);
    VECT3_ASSIGN(stabilization_gains[i].i, phi_igain[i], theta_igain[i], psi_igain[i]);
    VECT3_ASSIGN(stabilization_gains[i].dd, phi_ddgain[i], theta_ddgain[i], psi_ddgain[i]);
    VECT3_ASSIGN(stabilization_gains[i].rates_d, phi_dgain_d[i], theta_dgain_d[i], psi_dgain_d[i]);
#ifdef HAS_SURFACE_COMMANDS
    VECT3_ASSIGN(stabilization_gains[i].surface_p, phi_pgain_surface[i], theta_pgain_surface[i], psi_pgain_surface[i]);
    VECT3_ASSIGN(stabilization_gains[i].surface_d, phi_dgain_surface[i], theta_dgain_surface[i], psi_dgain_surface[i]);
    VECT3_ASSIGN(stabilization_gains[i].surface_i, phi_igain_surface[i], theta_igain_surface[i], psi_igain_surface[i]);
    VECT3_ASSIGN(stabilization_gains[i].surface_dd, phi_ddgain_surface[i], theta_ddgain_surface[i], psi_ddgain_surface[i]);
#endif
  }

  float_quat_identity(&stabilization_att_sum_err_quat);
  FLOAT_EULERS_ZERO( stabilization_att_sum_err );
  FLOAT_RATES_ZERO( last_body_rate );
  FLOAT_RATES_ZERO( body_rate_d );

#if PERIODIC_TELEMETRY
  register_periodic_telemetry(DefaultPeriodic, "STAB_ATTITUDE", send_att);
  register_periodic_telemetry(DefaultPeriodic, "STAB_ATTITUDE_REF", send_att_ref);
#endif
}

void stabilization_attitude_gain_schedule(uint8_t idx)
{
  if (gain_idx >= STABILIZATION_ATTITUDE_GAIN_NB) {
    // This could be bad -- Just say no.
    return;
  }
  gain_idx = idx;
  stabilization_attitude_ref_schedule(idx);
}

void stabilization_attitude_enter(void) {

  /* reset psi setpoint to current psi angle */
  stab_att_sp_euler.psi = stabilization_attitude_get_heading_f();

  stabilization_attitude_ref_enter();

  float_quat_identity(&stabilization_att_sum_err_quat);
  FLOAT_EULERS_ZERO( stabilization_att_sum_err );
}

void stabilization_attitude_set_failsafe_setpoint(void) {
  /* set failsafe to zero roll/pitch and current heading */
  float heading2 = stabilization_attitude_get_heading_f() / 2;
  stab_att_sp_quat.qi = cosf(heading2);
  stab_att_sp_quat.qx = 0.0;
  stab_att_sp_quat.qy = 0.0;
  stab_att_sp_quat.qz = sinf(heading2);
}

void stabilization_attitude_set_rpy_setpoint_i(struct Int32Eulers *rpy) {
  // copy euler setpoint for debugging
  EULERS_FLOAT_OF_BFP(stab_att_sp_euler, *rpy);

  quat_from_rpy_cmd_f(&stab_att_sp_quat, &stab_att_sp_euler);
}

void stabilization_attitude_set_earth_cmd_i(struct Int32Vect2 *cmd, int32_t heading) {
  struct FloatVect2 cmd_f;
  cmd_f.x = ANGLE_FLOAT_OF_BFP(cmd->x);
  cmd_f.y = ANGLE_FLOAT_OF_BFP(cmd->y);
  float heading_f;
  heading_f = ANGLE_FLOAT_OF_BFP(heading);

  quat_from_earth_cmd_f(&stab_att_sp_quat, &cmd_f, heading_f);
}

#ifndef GAIN_PRESCALER_FF
#define GAIN_PRESCALER_FF 1
#endif
static void attitude_run_ff(float ff_commands[], struct FloatAttitudeGains *gains, struct FloatRates *ref_accel)
{
  /* Compute feedforward based on reference acceleration */

  ff_commands[COMMAND_ROLL]          = GAIN_PRESCALER_FF * gains->dd.x * ref_accel->p;
  ff_commands[COMMAND_PITCH]         = GAIN_PRESCALER_FF * gains->dd.y * ref_accel->q;
  ff_commands[COMMAND_YAW]           = GAIN_PRESCALER_FF * gains->dd.z * ref_accel->r;
#ifdef HAS_SURFACE_COMMANDS
  ff_commands[COMMAND_ROLL_SURFACE]  = GAIN_PRESCALER_FF * gains->surface_dd.x * ref_accel->p;
  ff_commands[COMMAND_PITCH_SURFACE] = GAIN_PRESCALER_FF * gains->surface_dd.y * ref_accel->q;
  ff_commands[COMMAND_YAW_SURFACE]   = GAIN_PRESCALER_FF * gains->surface_dd.z * ref_accel->r;
#endif
}

#ifndef GAIN_PRESCALER_P
#define GAIN_PRESCALER_P 1
#endif
#ifndef GAIN_PRESCALER_D
#define GAIN_PRESCALER_D 1
#endif
#ifndef GAIN_PRESCALER_I
#define GAIN_PRESCALER_I 1
#endif
static void attitude_run_fb(float fb_commands[], struct FloatAttitudeGains *gains, struct FloatQuat *att_err,
    struct FloatRates *rate_err, struct FloatRates *rate_err_d, struct FloatQuat *sum_err)
{
  /*  PID feedback */
  fb_commands[COMMAND_ROLL] =
    GAIN_PRESCALER_P * gains->p.x  * att_err->qx +
    GAIN_PRESCALER_D * gains->d.x  * rate_err->p +
    GAIN_PRESCALER_D * gains->rates_d.x  * rate_err_d->p +
    GAIN_PRESCALER_I * gains->i.x  * sum_err->qx;

  fb_commands[COMMAND_PITCH] =
    GAIN_PRESCALER_P * gains->p.y  * att_err->qy +
    GAIN_PRESCALER_D * gains->d.y  * rate_err->q +
    GAIN_PRESCALER_D * gains->rates_d.y  * rate_err_d->q +
    GAIN_PRESCALER_I * gains->i.y  * sum_err->qy;

  fb_commands[COMMAND_YAW] =
    GAIN_PRESCALER_P * gains->p.z  * att_err->qz +
    GAIN_PRESCALER_D * gains->d.z  * rate_err->r +
    GAIN_PRESCALER_D * gains->rates_d.z  * rate_err_d->r +
    GAIN_PRESCALER_I * gains->i.z  * sum_err->qz;

#ifdef HAS_SURFACE_COMMANDS
  fb_commands[COMMAND_ROLL_SURFACE] =
    GAIN_PRESCALER_P * gains->surface_p.x  * att_err->qx +
    GAIN_PRESCALER_D * gains->surface_d.x  * rate_err->p +
    GAIN_PRESCALER_I * gains->surface_i.x  * sum_err->qx;

  fb_commands[COMMAND_PITCH_SURFACE] =
    GAIN_PRESCALER_P * gains->surface_p.y  * att_err->qy +
    GAIN_PRESCALER_D * gains->surface_d.y  * rate_err->q +
    GAIN_PRESCALER_I * gains->surface_i.y  * sum_err->qy;

  fb_commands[COMMAND_YAW_SURFACE] =
    GAIN_PRESCALER_P * gains->surface_p.z  * att_err->qz +
    GAIN_PRESCALER_D * gains->surface_d.z  * rate_err->r +
    GAIN_PRESCALER_I * gains->surface_i.z  * sum_err->qz;
#endif
}

void stabilization_attitude_run(bool_t enable_integrator) {

  /*
   * Update reference
   */
  stabilization_attitude_ref_update();

  /*
   * Compute errors for feedback
   */

  /* attitude error                          */
  struct FloatQuat att_err;
  struct FloatQuat* att_quat = stateGetNedToBodyQuat_f();
  FLOAT_QUAT_INV_COMP(att_err, *att_quat, stab_att_ref_quat);
  /* wrap it in the shortest direction       */
  FLOAT_QUAT_WRAP_SHORTEST(att_err);

  /*  rate error                */
  struct FloatRates rate_err;
  struct FloatRates* body_rate = stateGetBodyRates_f();
  RATES_DIFF(rate_err, stab_att_ref_rate, *body_rate);
  /* rate_d error               */
  RATES_DIFF(body_rate_d, *body_rate, last_body_rate);
  RATES_COPY(last_body_rate, *body_rate);

  /* integrated error */
  if (enable_integrator) {
    struct FloatQuat new_sum_err, scaled_att_err;
    /* update accumulator */
    scaled_att_err.qi = att_err.qi;
    scaled_att_err.qx = att_err.qx / IERROR_SCALE;
    scaled_att_err.qy = att_err.qy / IERROR_SCALE;
    scaled_att_err.qz = att_err.qz / IERROR_SCALE;
    FLOAT_QUAT_COMP(new_sum_err, stabilization_att_sum_err_quat, scaled_att_err);
    FLOAT_QUAT_NORMALIZE(new_sum_err);
    QUAT_COPY(stabilization_att_sum_err_quat, new_sum_err);
    FLOAT_EULERS_OF_QUAT(stabilization_att_sum_err, stabilization_att_sum_err_quat);
  } else {
    /* reset accumulator */
    float_quat_identity(&stabilization_att_sum_err_quat);
    FLOAT_EULERS_ZERO( stabilization_att_sum_err );
  }

  attitude_run_ff(stabilization_att_ff_cmd, &stabilization_gains[gain_idx], &stab_att_ref_accel);

  attitude_run_fb(stabilization_att_fb_cmd, &stabilization_gains[gain_idx], &att_err, &rate_err, &body_rate_d, &stabilization_att_sum_err_quat);

  stabilization_cmd[COMMAND_ROLL] = stabilization_att_fb_cmd[COMMAND_ROLL] + stabilization_att_ff_cmd[COMMAND_ROLL];
  stabilization_cmd[COMMAND_PITCH] = stabilization_att_fb_cmd[COMMAND_PITCH] + stabilization_att_ff_cmd[COMMAND_PITCH];
  stabilization_cmd[COMMAND_YAW] = stabilization_att_fb_cmd[COMMAND_YAW] + stabilization_att_ff_cmd[COMMAND_YAW];

#ifdef HAS_SURFACE_COMMANDS
  stabilization_cmd[COMMAND_ROLL_SURFACE] = stabilization_att_fb_cmd[COMMAND_ROLL_SURFACE] + stabilization_att_ff_cmd[COMMAND_ROLL_SURFACE];
  stabilization_cmd[COMMAND_PITCH_SURFACE] = stabilization_att_fb_cmd[COMMAND_PITCH_SURFACE] + stabilization_att_ff_cmd[COMMAND_PITCH_SURFACE];
  stabilization_cmd[COMMAND_YAW_SURFACE] = stabilization_att_fb_cmd[COMMAND_YAW_SURFACE] + stabilization_att_ff_cmd[COMMAND_YAW_SURFACE];
#endif

  /* bound the result */
  BoundAbs(stabilization_cmd[COMMAND_ROLL], MAX_PPRZ);
  BoundAbs(stabilization_cmd[COMMAND_PITCH], MAX_PPRZ);
  BoundAbs(stabilization_cmd[COMMAND_YAW], MAX_PPRZ);
}

void stabilization_attitude_read_rc(bool_t in_flight, bool_t in_carefree, bool_t coordinated_turn) {

  stabilization_attitude_read_rc_setpoint_quat_f(&stab_att_sp_quat, in_flight, in_carefree, coordinated_turn);
  //FLOAT_QUAT_WRAP_SHORTEST(stab_att_sp_quat);
}
