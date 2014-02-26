/*
 * Copyright (C) 2008-2012 The Paparazzi Team
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
 * @file subsystems/ins.h
 * Integrated Navigation System interface.
 */

#ifndef INS_H
#define INS_H

#include "std.h"
#include "math/pprz_geodetic_int.h"
#include "math/pprz_algebra_float.h"
#include "state.h"

#define INS_UNINIT  0
#define INS_RUNNING 1

/* underlying includes (needed for parameters) */
#ifdef INS_TYPE_H
#include INS_TYPE_H
#endif

/** Inertial Navigation System state */
struct Ins {
  uint8_t status;     ///< status of the INS
  bool_t hf_realign;  ///< flag to request to realign horizontal filter to current position (local origin unchanged)
  bool_t vf_realign;  ///< flag to request to realign vertical filter to ground level (local origin unchanged)
};

/** global INS state */
extern struct Ins ins;

/** INS initialization. Called at startup.
 *  Needs to be implemented by each INS algorithm.
 */
extern void ins_init( void );

/** INS periodic call.
 *  Needs to be implemented by each INS algorithm.
 */
extern void ins_periodic( void );

/** INS local origin reset.
 *  Reset horizontal and vertical reference to the current position.
 *  Needs to be implemented by each INS algorithm.
 */
extern void ins_reset_local_origin( void );

/** INS altitude reference reset.
 *  Reset only vertical reference to the current altitude.
 *  Needs to be implemented by each INS algorithm.
 */
extern void ins_reset_altitude_ref( void );

/** INS utm zone reset.
 *  Reset UTM zone according te the actual position.
 *  Needs to be implemented by each INS algorithm but is only
 *  used with fixedwing firmware.
 *  @param utm initial utm zone, returns the corrected utm position
 */
extern void ins_reset_utm_zone(struct UtmCoor_f * utm);

/** INS horizontal realign.
 *  This only reset the filter to a given value, but doesn't change the local reference.
 *  @param pos new horizontal position to set
 *  @param speed new horizontal speed to set
 *  Needs to be implemented by each INS algorithm.
 */
extern void ins_realign_h(struct FloatVect2 pos, struct FloatVect2 speed);

/** INS vertical realign.
 *  This only reset the filter to a given value, but doesn't change the local reference.
 *  @param z new altitude to set
 *  Needs to be implemented by each INS algorithm.
 */
extern void ins_realign_v(float z);

/** Propagation. Usually integrates the gyro rates to angles.
 *  Reads the global #imu data struct.
 *  Needs to be implemented by each INS algorithm.
 */
extern void ins_propagate( void );

/** Update INS state with barometer measurements.
 *  Reads the global #baro data struct.
 *  Needs to be implemented by each INS algorithm.
 */
extern void ins_update_baro( void );

/** Update INS state with GPS measurements.
 *  Reads the global #gps data struct.
 *  Needs to be implemented by each INS algorithm.
 */
extern void ins_update_gps( void );

/** Update INS state with sonar measurements.
 *  Reads the global #sonar data struct.
 *  Needs to be implemented by each INS algorithm.
 */
extern void ins_update_sonar( void );


#endif /* INS_H */
