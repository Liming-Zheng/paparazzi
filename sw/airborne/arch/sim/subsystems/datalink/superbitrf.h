/*
 * Copyright (C) 2013 Freek van Tienen <freek.v.tienen@gmail.com>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with paparazzi; see the file COPYING. If not, write to
 * the Free Software Foundation, 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/**
 * @file arch/sim/subsystems/datalink/superbitrf.h
 * DSM2 and DSMX datalink implementation for the cyrf6936 2.4GHz radio chip trough SPI
 *
 * Dummy for sim so you don't have to remove the superbitrf.xml settings file.
 */

#ifndef DATALINK_SUPERBITRF_H
#define DATALINK_SUPERBITRF_H

/* The superbitrf structure */
struct SuperbitRF {
  uint8_t bind_mfg_id[4];                   /**< The MFG id where the receiver is bound to */
  uint32_t bind_mfg_id32;                   /**< The MFG id where the receiver is bound to in uint32 */
};

/* The superbitrf functions and structures */
extern struct SuperbitRF superbitrf;

#endif /* DATALINK_SUPERBITRF_H */
