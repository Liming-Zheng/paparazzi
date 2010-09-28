/*
 * $Id$
 *
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

#include "hf_float-old.h"
#include <firmwares/rotorcraft/ins.h>

#include <firmwares/rotorcraft/imu.h>
#include <firmwares/rotorcraft/ahrs.h>
#include "math/pprz_algebra_int.h"


struct Int32Vect3 b2ins_accel_bias;
struct Int32Vect3 b2ins_accel_ltp;
struct Int32Vect3 b2ins_speed_ltp;
struct Int64Vect3 b2ins_pos_ltp;

struct Int32Eulers b2ins_body_to_imu_eulers;
struct Int32Quat   b2ins_body_to_imu_quat;
struct Int32Quat   b2ins_imu_to_body_quat;

struct Int32Vect3  b2ins_meas_gps_pos_ned;
struct Int32Vect3  b2ins_meas_gps_speed_ned;


#include "downlink.h"

void b2ins_init(void) {
  INT32_VECT3_ZERO(b2ins_accel_bias);
}

void b2ins_propagate(void) {

  struct Int32Vect3 scaled_biases;
  VECT3_SDIV(scaled_biases, b2ins_accel_bias, (1<<(B2INS_ACCEL_BIAS_FRAC-B2INS_ACCEL_LTP_FRAC)));
  struct Int32Vect3 accel_imu;
  /* unbias accelerometers */
  VECT3_DIFF(accel_imu, imu.accel, scaled_biases);
  /* convert to LTP */
  //  BOOZ_IQUAT_VDIV(b2ins_accel_ltp, ahrs.ltp_to_imu_quat, accel_imu);
  INT32_RMAT_TRANSP_VMULT(b2ins_accel_ltp,  ahrs.ltp_to_imu_rmat, accel_imu);
  /* correct for gravity */
  b2ins_accel_ltp.z += ACCEL_BFP_OF_REAL(9.81);
  /* propagate position */
  VECT3_ADD(b2ins_pos_ltp, b2ins_speed_ltp);
  /* propagate speed */
  VECT3_ADD(b2ins_speed_ltp, b2ins_accel_ltp);

}

#define K_POS   3
/* make sure >=9 */
#define K_SPEED 9

#define UPDATE_FROM_POS   1
#define UPDATE_FROM_SPEED 1

void b2ins_update_gps(void) {

#ifdef UPDATE_FROM_POS
  struct Int64Vect2 scaled_pos_meas;
  /* INT32 pos in cm -> INT64 pos in cm*/
  VECT2_COPY(scaled_pos_meas, ins_gps_pos_cm_ned);
  /* to BFP but still in cm */
  VECT2_SMUL(scaled_pos_meas, scaled_pos_meas, (1<<B2INS_POS_LTP_FRAC));
  /* INT64 BFP pos in cm -> INT64 BFP pos in m */
  VECT2_SDIV(scaled_pos_meas, scaled_pos_meas, 100);
  struct Int64Vect2 pos_residual;
  VECT2_DIFF(pos_residual, scaled_pos_meas, b2ins_pos_ltp);
  struct Int32Vect2 pos_cor_1;
  VECT2_SDIV(pos_cor_1, pos_residual, (1<<K_POS));
  VECT2_ADD(b2ins_pos_ltp, pos_cor_1);
  struct Int32Vect2 speed_cor_1;
  VECT2_SDIV(speed_cor_1, pos_residual, (1<<(K_POS+9)));
  VECT2_ADD(b2ins_speed_ltp, speed_cor_1);
#endif /* UPDATE_FROM_POS */

#ifdef UPDATE_FROM_SPEED
  INT32_VECT3_SCALE_2(b2ins_meas_gps_speed_ned, ins_gps_speed_cm_s_ned, INT32_SPEED_OF_CM_S_NUM, INT32_SPEED_OF_CM_S_DEN);
  struct Int32Vect2 scaled_speed_meas;
  VECT2_SMUL(scaled_speed_meas, b2ins_meas_gps_speed_ned, (1<<(B2INS_SPEED_LTP_FRAC-INT32_SPEED_FRAC)));
  struct Int32Vect2 speed_residual;
  VECT2_DIFF(speed_residual, scaled_speed_meas, b2ins_speed_ltp);
  struct Int32Vect2 pos_cor_s;
  VECT2_SDIV(pos_cor_s, speed_residual, (1<<(K_SPEED-9)));
  VECT2_ADD(b2ins_pos_ltp, pos_cor_s);
  struct Int32Vect2 speed_cor_s;
  VECT2_SDIV(speed_cor_s, speed_residual, (1<<K_SPEED));
  VECT2_ADD(b2ins_speed_ltp, speed_cor_s);

  struct Int32Vect3 speed_residual3;
  VECT2_SDIV(speed_residual3, speed_residual, (1<<9));
  speed_residual3.z = 0;
  struct Int32Vect3 bias_cor_s;
  INT32_RMAT_VMULT( bias_cor_s, ahrs.ltp_to_imu_rmat, speed_residual3);
  VECT3_ADD(b2ins_accel_bias, bias_cor_s);
#endif /* UPDATE_FROM_SPEED */

}
