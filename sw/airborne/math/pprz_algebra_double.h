/*
 * Copyright (C) 2008-2014 The Paparazzi Team
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
 * along with paparazzi; see the file COPYING.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

/**
 * @file pprz_algebra_double.h
 * @brief Paparazzi double precision floating point algebra.
 *
 */

#ifndef PPRZ_ALGEBRA_DOUBLE_H
#define PPRZ_ALGEBRA_DOUBLE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "pprz_algebra.h"
#include "pprz_algebra_float.h"

struct DoubleVect2 {
  double x;
  double y;
};

struct DoubleVect3 {
  double x;
  double y;
  double z;
};

/**
 * @brief Roation quaternion
 */
struct DoubleQuat {
  double qi;
  double qx;
  double qy;
  double qz;
};

struct DoubleMat33 {
  double m[3 * 3];
};

/**
 * @brief rotation matrix
 */
struct DoubleRMat {
  double m[3 * 3];
};

/**
 * @brief euler angles
 * @details Units: radians */
struct DoubleEulers {
  double phi; ///< in radians
  double theta; ///< in radians
  double psi; ///< in radians
};

/**
 * @brief angular rates
 * @details Units: rad/s^2 */
struct DoubleRates {
  double p; ///< in rad/s^2
  double q; ///< in rad/s^2
  double r; ///< in rad/s^2
};

#define DOUBLE_VECT3_ROUND(_v) DOUBLE_VECT3_RINT(_v, _v)


#define DOUBLE_VECT3_RINT(_vout, _vin) {    \
    (_vout).x = rint((_vin).x);         \
    (_vout).y = rint((_vin).y);         \
    (_vout).z = rint((_vin).z);         \
  }

static inline double double_vect3_norm(struct DoubleVect3* v)
{
  return sqrt(VECT3_NORM2(*v));
}

/** normalize 3D vector in place */
static inline void double_vect3_normalize(struct DoubleVect3* v)
{
  const double n = double_vect3_norm(v);
  if (n > 0) {
    v->x /= n;
    v->y /= n;
    v->z /= n;
  }
}

static inline double double_quat_norm(struct DoubleQuat* q)
{
  return sqrt(SQUARE(q->qi) + SQUARE(q->qx) +  SQUARE(q->qy) + SQUARE(q->qz));
}


static inline void double_quat_normalize(struct DoubleQuat* q)
{
  double qnorm = double_quat_norm(q);
  if (qnorm > FLT_MIN) {
    q->qi = q->qi / qnorm;
    q->qx = q->qx / qnorm;
    q->qy = q->qy / qnorm;
    q->qz = q->qz / qnorm;
  }
}

extern void double_rmat_of_eulers_321(struct DoubleRMat* rm, struct DoubleEulers* e);
extern void double_quat_of_eulers(struct DoubleQuat* q, struct DoubleEulers* e);
extern void double_eulers_of_quat(struct DoubleEulers* e, struct DoubleQuat* q);
extern void double_quat_vmult(struct DoubleVect3* v_out, struct DoubleQuat* q, struct DoubleVect3* v_in);

static inline void double_rmat_of_eulers(struct DoubleRMat* rm, struct DoubleEulers* e)
{
  double_rmat_of_eulers_321(rm, e);
}

/* defines for backwards compatibility */
#define DOUBLE_RMAT_OF_EULERS(_rm, _e) double_rmat_of_eulers(&(_rm), &(_e))
#define DOUBLE_RMAT_OF_EULERS_321(_rm, _e) double_rmat_of_eulers(&(_rm), &(_e))
#define DOUBLE_QUAT_OF_EULERS(_q, _e) double_quat_of_eulers(&(_q), &(_e))
#define DOUBLE_EULERS_OF_QUAT(_e, _q) double_eulers_of_quat(&(_e), &(_q))
#define DOUBLE_QUAT_VMULT(v_out, q, v_in) double_quat_vmult(&(v_out), &(q), &(v_in))

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* PPRZ_ALGEBRA_DOUBLE_H */
