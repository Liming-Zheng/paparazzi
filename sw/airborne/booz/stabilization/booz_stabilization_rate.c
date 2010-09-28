/*
 * $Id$
 *
 * Copyright (C) 2008-2009 Antoine Drouin <poinix@gmail.com>
 * Copyright (C) 2010 Felix Ruess <felix.ruess@gmail.com>
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

#include "booz_stabilization.h"

#include "ahrs.h"

#include <firmwares/rotorcraft/imu.h>
#include "booz_radio_control.h"
#include "airframe.h"

#define F_UPDATE_RES 9
#define REF_DOT_FRAC 11
#define REF_FRAC  16

#define MAX_SUM_ERR 4000000

#ifndef BOOZ_STABILIZATION_RATE_DDGAIN_P
#define BOOZ_STABILIZATION_RATE_DDGAIN_P 0
#endif
#ifndef BOOZ_STABILIZATION_RATE_DDGAIN_Q
#define BOOZ_STABILIZATION_RATE_DDGAIN_Q 0
#endif
#ifndef BOOZ_STABILIZATION_RATE_DDGAIN_R
#define BOOZ_STABILIZATION_RATE_DDGAIN_R 0
#endif
#ifndef BOOZ_STABILIZATION_RATE_IGAIN_P
#define BOOZ_STABILIZATION_RATE_IGAIN_P 0
#endif
#ifndef BOOZ_STABILIZATION_RATE_IGAIN_Q
#define BOOZ_STABILIZATION_RATE_IGAIN_Q 0
#endif
#ifndef BOOZ_STABILIZATION_RATE_IGAIN_R
#define BOOZ_STABILIZATION_RATE_IGAIN_R 0
#endif
#ifndef BOOZ_STABILIZATION_RATE_REF_TAU
#define BOOZ_STABILIZATION_RATE_REF_TAU 4
#endif

#define OFFSET_AND_ROUND(_a, _b) (((_a)+(1<<((_b)-1)))>>(_b))
#define OFFSET_AND_ROUND2(_a, _b) (((_a)+(1<<((_b)-1))-((_a)<0?1:0))>>(_b))

struct Int32Rates booz_stabilization_rate_sp;
struct Int32Rates booz_stabilization_rate_gain;
struct Int32Rates booz_stabilization_rate_igain;
struct Int32Rates booz_stabilization_rate_ddgain;
struct Int32Rates booz_stabilization_rate_ref;
struct Int32Rates booz_stabilization_rate_refdot;
struct Int32Rates booz_stabilization_rate_sum_err;

struct Int32Rates booz_stabilization_rate_fb_cmd;
struct Int32Rates booz_stabilization_rate_ff_cmd;


#ifndef BOOZ_STABILIZATION_RATE_DEADBAND_P
#define BOOZ_STABILIZATION_RATE_DEADBAND_P 0
#endif
#ifndef BOOZ_STABILIZATION_RATE_DEADBAND_Q
#define BOOZ_STABILIZATION_RATE_DEADBAND_Q 0
#endif
#ifndef BOOZ_STABILIZATION_RATE_DEADBAND_R
#define BOOZ_STABILIZATION_RATE_DEADBAND_R 200
#endif

#define ROLL_RATE_DEADBAND_EXCEEDED()                                         \
  (radio_control.values[RADIO_CONTROL_ROLL] >  BOOZ_STABILIZATION_RATE_DEADBAND_P || \
   radio_control.values[RADIO_CONTROL_ROLL] < -BOOZ_STABILIZATION_RATE_DEADBAND_P)

#define PITCH_RATE_DEADBAND_EXCEEDED()                                         \
  (radio_control.values[RADIO_CONTROL_PITCH] >  BOOZ_STABILIZATION_RATE_DEADBAND_Q || \
   radio_control.values[RADIO_CONTROL_PITCH] < -BOOZ_STABILIZATION_RATE_DEADBAND_Q)

#define YAW_RATE_DEADBAND_EXCEEDED()                                         \
  (radio_control.values[RADIO_CONTROL_YAW] >  BOOZ_STABILIZATION_RATE_DEADBAND_R || \
   radio_control.values[RADIO_CONTROL_YAW] < -BOOZ_STABILIZATION_RATE_DEADBAND_R)


void booz_stabilization_rate_init(void) {

  INT_RATES_ZERO(booz_stabilization_rate_sp);

  RATES_ASSIGN(booz_stabilization_rate_gain,
               BOOZ_STABILIZATION_RATE_GAIN_P,
               BOOZ_STABILIZATION_RATE_GAIN_Q,
               BOOZ_STABILIZATION_RATE_GAIN_R);
  RATES_ASSIGN(booz_stabilization_rate_igain,
               BOOZ_STABILIZATION_RATE_IGAIN_P,
               BOOZ_STABILIZATION_RATE_IGAIN_Q,
               BOOZ_STABILIZATION_RATE_IGAIN_R);
  RATES_ASSIGN(booz_stabilization_rate_ddgain,
               BOOZ_STABILIZATION_RATE_DDGAIN_P,
               BOOZ_STABILIZATION_RATE_DDGAIN_Q,
               BOOZ_STABILIZATION_RATE_DDGAIN_R);

  INT_RATES_ZERO(booz_stabilization_rate_ref);
  INT_RATES_ZERO(booz_stabilization_rate_refdot);
  INT_RATES_ZERO(booz_stabilization_rate_sum_err);
}


void booz_stabilization_rate_read_rc( void ) {

  if(ROLL_RATE_DEADBAND_EXCEEDED())
    booz_stabilization_rate_sp.p = (int32_t)-radio_control.values[RADIO_CONTROL_ROLL] * BOOZ_STABILIZATION_RATE_SP_MAX_P / MAX_PPRZ;
  else
    booz_stabilization_rate_sp.p = 0;

  if(PITCH_RATE_DEADBAND_EXCEEDED())
    booz_stabilization_rate_sp.q = (int32_t)radio_control.values[RADIO_CONTROL_PITCH] * BOOZ_STABILIZATION_RATE_SP_MAX_Q / MAX_PPRZ;
  else
    booz_stabilization_rate_sp.q = 0;

  if(YAW_RATE_DEADBAND_EXCEEDED())
    booz_stabilization_rate_sp.r = (int32_t)-radio_control.values[RADIO_CONTROL_YAW] * BOOZ_STABILIZATION_RATE_SP_MAX_R / MAX_PPRZ;
  else
    booz_stabilization_rate_sp.r = 0;
}

void booz_stabilization_rate_enter(void) {
  RATES_COPY(booz_stabilization_rate_ref, booz_stabilization_rate_sp);
  INT_RATES_ZERO(booz_stabilization_rate_sum_err);
}

void booz_stabilization_rate_run(bool_t in_flight) {

  /* reference */
  struct Int32Rates _r;
  RATES_DIFF(_r, booz_stabilization_rate_sp, booz_stabilization_rate_ref);
  RATES_SDIV(booz_stabilization_rate_refdot, _r, BOOZ_STABILIZATION_RATE_REF_TAU);
  /* integrate ref */
  const struct Int32Rates _delta_ref = {
    booz_stabilization_rate_refdot.p >> ( F_UPDATE_RES + REF_DOT_FRAC - REF_FRAC),
    booz_stabilization_rate_refdot.q >> ( F_UPDATE_RES + REF_DOT_FRAC - REF_FRAC),
    booz_stabilization_rate_refdot.r >> ( F_UPDATE_RES + REF_DOT_FRAC - REF_FRAC)};
  RATES_ADD(booz_stabilization_rate_ref, _delta_ref);

  /* compute feed-forward command */
  RATES_EWMULT_RSHIFT(booz_stabilization_rate_ff_cmd, booz_stabilization_rate_ddgain, booz_stabilization_rate_refdot, 14);


  /* compute feed-back command */
  /* error for feedback */
  const struct Int32Rates _ref_scaled = {
    OFFSET_AND_ROUND(booz_stabilization_rate_ref.p, (REF_FRAC - INT32_RATE_FRAC)),
    OFFSET_AND_ROUND(booz_stabilization_rate_ref.q, (REF_FRAC - INT32_RATE_FRAC)),
    OFFSET_AND_ROUND(booz_stabilization_rate_ref.r, (REF_FRAC - INT32_RATE_FRAC)) };
  struct Int32Rates _error;
  RATES_DIFF(_error, ahrs.body_rate, _ref_scaled);
  if (in_flight) {
    /* update integrator */
    RATES_ADD(booz_stabilization_rate_sum_err, _error);
    RATES_BOUND_CUBE(booz_stabilization_rate_sum_err, -MAX_SUM_ERR, MAX_SUM_ERR);
  }
  else {
    INT_RATES_ZERO(booz_stabilization_rate_sum_err);
  }

  /* PI */
  booz_stabilization_rate_fb_cmd.p = booz_stabilization_rate_gain.p * _error.p +
    OFFSET_AND_ROUND2((booz_stabilization_rate_igain.p  * booz_stabilization_rate_sum_err.p), 10);

  booz_stabilization_rate_fb_cmd.q = booz_stabilization_rate_gain.q * _error.q +
    OFFSET_AND_ROUND2((booz_stabilization_rate_igain.q  * booz_stabilization_rate_sum_err.q), 10);

  booz_stabilization_rate_fb_cmd.r = booz_stabilization_rate_gain.r * _error.r +
    OFFSET_AND_ROUND2((booz_stabilization_rate_igain.r  * booz_stabilization_rate_sum_err.r), 10);

  booz_stabilization_rate_fb_cmd.p = booz_stabilization_rate_fb_cmd.p >> 16;
  booz_stabilization_rate_fb_cmd.q = booz_stabilization_rate_fb_cmd.q >> 16;
  booz_stabilization_rate_fb_cmd.r = booz_stabilization_rate_fb_cmd.r >> 16;

  /* sum to final command */
  booz_stabilization_cmd[COMMAND_ROLL]  = booz_stabilization_rate_ff_cmd.p + booz_stabilization_rate_fb_cmd.p;
  booz_stabilization_cmd[COMMAND_PITCH] = booz_stabilization_rate_ff_cmd.q + booz_stabilization_rate_fb_cmd.q;
  booz_stabilization_cmd[COMMAND_YAW]   = booz_stabilization_rate_ff_cmd.r + booz_stabilization_rate_fb_cmd.r;

}
