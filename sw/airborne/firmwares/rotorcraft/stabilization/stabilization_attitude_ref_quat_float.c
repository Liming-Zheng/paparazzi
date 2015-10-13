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

/**
 * @file firmwares/rotorcraft/stabilization/stabilization_attitude_ref_quat_float.c
 *
 * Rotorcraft attitude reference generation.
 * (quaternion float version)
 *
 */

#include "generated/airframe.h"

#include "firmwares/rotorcraft/stabilization/stabilization_attitude_ref_quat_float.h"
#include "firmwares/rotorcraft/stabilization/stabilization_attitude_ref_saturate.h"

/// default to fast but less precise quaternion integration
#ifndef STABILIZATION_ATTITUDE_REF_QUAT_INFINITESIMAL_STEP
#define STABILIZATION_ATTITUDE_REF_QUAT_INFINITESIMAL_STEP TRUE
#endif

/* shortcuts for saturation defines */
#define REF_ACCEL_MAX_P STABILIZATION_ATTITUDE_REF_MAX_PDOT
#define REF_ACCEL_MAX_Q STABILIZATION_ATTITUDE_REF_MAX_QDOT
#define REF_ACCEL_MAX_R STABILIZATION_ATTITUDE_REF_MAX_RDOT

#define REF_RATE_MAX_P STABILIZATION_ATTITUDE_REF_MAX_P
#define REF_RATE_MAX_Q STABILIZATION_ATTITUDE_REF_MAX_Q
#define REF_RATE_MAX_R STABILIZATION_ATTITUDE_REF_MAX_R


/* parameters used for initialization */
static const float omega_p[] = STABILIZATION_ATTITUDE_REF_OMEGA_P;
static const float zeta_p[] = STABILIZATION_ATTITUDE_REF_ZETA_P;
static const float omega_q[] = STABILIZATION_ATTITUDE_REF_OMEGA_Q;
static const float zeta_q[] = STABILIZATION_ATTITUDE_REF_ZETA_Q;
static const float omega_r[] = STABILIZATION_ATTITUDE_REF_OMEGA_R;
static const float zeta_r[] = STABILIZATION_ATTITUDE_REF_ZETA_R;

static inline void reset_psi_ref(struct AttRefQuatFloat *ref, float psi);

/*
 *
 * Implementation.
 * Should not rely on any global variables, so these functions can be used like a lib.
 *
 */
void attitude_ref_quat_float_init(struct AttRefQuatFloat *ref)
{
  FLOAT_EULERS_ZERO(ref->euler);
  float_quat_identity(&ref->quat);
  FLOAT_RATES_ZERO(ref->rate);
  FLOAT_RATES_ZERO(ref->accel);

  for (int i = 0; i < STABILIZATION_ATTITUDE_GAIN_NB; i++) {
    RATES_ASSIGN(ref->model[i].omega, omega_p[i], omega_q[i], omega_r[i]);
    RATES_ASSIGN(ref->model[i].zeta, zeta_p[i], zeta_q[i], zeta_r[i]);
    RATES_ASSIGN(ref->model[i].two_omega2, 2 * omega_p[i]*omega_p[i], 2 * omega_q[i]*omega_q[i], 2 * omega_r[i]*omega_r[i]);
  }

  ref->cur_idx = 0;
}

void attitude_ref_quat_float_enter(struct AttRefQuatFloat *ref, float psi)
{
  reset_psi_ref(ref, psi);
}


void attitude_ref_quat_float_update(struct AttRefQuatFloat *ref, struct FloatQuat *sp_quat, float dt)
{

  /* integrate reference attitude            */
#if STABILIZATION_ATTITUDE_REF_QUAT_INFINITESIMAL_STEP
  struct FloatQuat qdot;
  float_quat_derivative(&qdot, &ref->rate, &ref->quat);
  QUAT_SMUL(qdot, qdot, dt);
  QUAT_ADD(ref->quat, qdot);
#else // use finite step (involves trig)
  struct FloatQuat delta_q;
  float_quat_differential(&delta_q, &ref->rate, dt);
  /* compose new ref_quat by quaternion multiplication of delta rotation and current ref_quat */
  struct FloatQuat new_ref_quat;
  float_quat_comp(&new_ref_quat, &ref->quat, &delta_q);
  QUAT_COPY(ref->quat, new_ref_quat);
#endif
  float_quat_normalize(&ref->quat);

  /* integrate reference rotational speeds   */
  struct FloatRates delta_rate;
  RATES_SMUL(delta_rate, ref->accel, dt);
  RATES_ADD(ref->rate, delta_rate);

  /* compute reference angular accelerations */
  struct FloatQuat err;
  /* compute reference attitude error        */
  float_quat_inv_comp(&err, sp_quat, &ref->quat);
  /* wrap it in the shortest direction       */
  float_quat_wrap_shortest(&err);
  /* propagate the 2nd order linear model: xdotdot = -2*zeta*omega*xdot - omega^2*x  */
  /* since error quaternion contains the half-angles we get 2*omega^2*err */
  ref->accel.p = -2.*ref->model[ref->cur_idx].zeta.p * ref->model[ref->cur_idx].omega.p * ref->rate.p -
    ref->model[ref->cur_idx].two_omega2.p * err.qx;
  ref->accel.q = -2.*ref->model[ref->cur_idx].zeta.q * ref->model[ref->cur_idx].omega.q * ref->rate.q -
    ref->model[ref->cur_idx].two_omega2.q * err.qy;
  ref->accel.r = -2.*ref->model[ref->cur_idx].zeta.r * ref->model[ref->cur_idx].omega.r * ref->rate.r -
    ref->model[ref->cur_idx].two_omega2.r * err.qz;

  /*  saturate acceleration */
  const struct FloatRates MIN_ACCEL = { -REF_ACCEL_MAX_P, -REF_ACCEL_MAX_Q, -REF_ACCEL_MAX_R };
  const struct FloatRates MAX_ACCEL = {  REF_ACCEL_MAX_P,  REF_ACCEL_MAX_Q,  REF_ACCEL_MAX_R };
  RATES_BOUND_BOX(ref->accel, MIN_ACCEL, MAX_ACCEL);

  /* saturate angular speed and trim accel accordingly */
  SATURATE_SPEED_TRIM_ACCEL(*ref);

  /* compute ref_euler */
  float_eulers_of_quat(&ref->euler, &ref->quat);
}



/*
 *
 * Local helper functions.
 *
 */
static inline void reset_psi_ref(struct AttRefQuatFloat *ref, float psi)
{
  ref->euler.psi = psi;
  ref->rate.r = 0;
  ref->accel.r = 0;

  // recalc quat from eulers
  float_quat_of_eulers(&ref->quat, &ref->euler);
  float_quat_wrap_shortest(&ref->quat);
}


/*
 *
 * Setting of the reference model parameters
 *
 */
void attitude_ref_quat_float_idx_set_omega_p(struct AttRefQuatFloat *ref, uint8_t idx, float omega)
{
  ref->model[idx].omega.p = omega;
  ref->model[idx].two_omega2.p = 2 * omega * omega;
}

void attitude_ref_quat_float_idx_set_omega_q(struct AttRefQuatFloat *ref, uint8_t idx, float omega)
{
  ref->model[idx].omega.q = omega;
  ref->model[idx].two_omega2.q = 2 * omega * omega;
}

void attitude_ref_quat_float_idx_set_omega_r(struct AttRefQuatFloat *ref, uint8_t idx, float omega)
{
  ref->model[idx].omega.r = omega;
  ref->model[idx].two_omega2.r = 2 * omega * omega;
}

void attitude_ref_quat_float_set_omega_p(struct AttRefQuatFloat *ref, float omega)
{
  attitude_ref_quat_float_idx_set_omega_p(ref, ref->cur_idx, omega);
}

void attitude_ref_quat_float_set_omega_q(struct AttRefQuatFloat *ref, float omega)
{
  attitude_ref_quat_float_idx_set_omega_q(ref, ref->cur_idx, omega);
}

void attitude_ref_quat_float_set_omega_r(struct AttRefQuatFloat *ref, float omega)
{
  attitude_ref_quat_float_idx_set_omega_r(ref, ref->cur_idx, omega);
}


/*
 * schedule a different model
 */
void attitude_ref_quat_float_schedule(struct AttRefQuatFloat *ref, uint8_t idx)
{
  ref->cur_idx = idx;
}
