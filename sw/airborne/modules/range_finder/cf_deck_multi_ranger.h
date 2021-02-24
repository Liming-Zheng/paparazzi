/*
 * Copyright (C) Gautier Hattenberger <gautier.hattenberger@enac.fr>
 *
 * This file is part of paparazzi
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

/** @file "modules/range_finder/cf_deck_multi_ranger.h"
 * @author Gautier Hattenberger <gautier.hattenberger@enac.fr>
 * Multi-ranger deck from Bitcraze for Crazyflie drones
 */

#ifndef CF_DECK_MULTI_RANGER_H
#define CF_DECK_MULTI_RANGER_H

#include "std.h"

extern void multi_ranger_init(void);
extern void multi_ranger_periodic(void);
extern void multi_ranger_report(void);
extern void multi_ranger_event(void);

#endif  // CF_DECK_MULTI_RANGER_H
