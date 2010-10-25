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

#ifndef AHRS_CMPL_EULER_H
#define AHRS_CMPL_EULER_H

#include <subsystems/ahrs.h>
#include "std.h"
#include <math/pprz_algebra_int.h>

extern struct Int32Rates  face_gyro_bias;
extern struct Int32Eulers face_measure;
extern struct Int32Eulers face_residual;
extern struct Int32Eulers face_uncorrected;
extern struct Int32Eulers face_corrected;

extern int32_t face_reinj_1;


#endif /* AHRS_CMPL_EULER_H */
