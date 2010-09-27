/*
 * $Id: actuators_mkk.h 3847 2009-08-02 21:47:31Z poine $
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

#ifndef ACTUATORS_MKK_H
#define ACTUATORS_MKK_H

#include "std.h"
#include "i2c.h"

#include "airframe.h"


struct ActuatorsMkk {
  struct i2c_transaction trans[ACTUATORS_MKK_NB];
};

extern struct ActuatorsMkk actuators_mkk;



#include "actuators/supervision.h"


#endif /* ACTUATORS_MKK_H */
