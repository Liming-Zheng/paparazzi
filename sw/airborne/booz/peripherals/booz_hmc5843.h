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

#ifndef BOOZ_HMC5843_H
#define BOOZ_HMC5843_H

#include "std.h"

enum Hmc5843Status {
  HMC5843_UNINITIALIZED1,
  HMC5843_UNINITIALIZED2,
  HMC5843_UNINITIALIZED3,
  HMC5843_IDLE,
  HMC5843_DATA_AVAILABLE,
  HMC5843_READING
};

struct Hmc5843 {
  volatile enum Hmc5843Status status;
  volatile uint8_t i2c_done;
  union {
    uint8_t buf[7];
    int16_t value[3];
  } data;
};

extern struct Hmc5843 hmc5843;
extern struct i2c_transaction hmc5843_i2c_trans;

extern void hmc5843_init(void);
extern void hmc5843_periodic(void);

/* default I2C address */
#define HMC5843_ADDR 0x3C

/* Registers */
#define HMC5843_REG_CFGA   0x00
#define HMC5843_REG_CFGB   0x01
#define HMC5843_REG_MODE   0x02
#define HMC5843_REG_DATXM  0x03
#define HMC5843_REG_DATXL  0x04
#define HMC5843_REG_DATYM  0x05
#define HMC5843_REG_DATYL  0x06
#define HMC5843_REG_DATZM  0x07
#define HMC5843_REG_DATZL  0x08
#define HMC5843_REG_STATUS 0x09
#define HMC5843_REG_IDA    0x0A
#define HMC5843_REG_IDB    0x0B
#define HMC5843_REG_IDC    0x0C

#include <string.h>

#define MagEvent(_m_handler) {						\
    if (hmc5843.status == HMC5843_READING && hmc5843_i2c_trans.status == I2CTransSuccess) {	\
      memcpy(hmc5843.data.buf, (const void*)hmc5843_i2c_trans.buf, 6);		\
      hmc5843.status = HMC5843_DATA_AVAILABLE;					\
      _m_handler();							\
    }	else if (hmc5843_i2c_trans.status != I2CTransPending) { \
      hmc5843.status = HMC5843_IDLE;					\
    } \
  }

#endif /* BOOZ_HMC5843_H */
