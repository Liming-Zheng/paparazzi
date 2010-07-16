/*
 * $Id:$
 *
 * Copyright (C) 2010 Piotr Esden-Tempski <piotr@esden.net>
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
 *
 */

#include <stdint.h>

#include "can.h"
#include "can_hw.h"

void can_init(void)
{
	can_hw_init();
}

static inline uint32_t can_id(uint8_t client_id, uint8_t msg_id)
{
	return ((client_id & 0xF) << 7) | (msg_id & 0x7F);
}

int can_transmit(uint8_t client_id, uint8_t msg_id, const uint8_t *buf, uint8_t len)
{
	uint32_t id = can_id(client_id, msg_id);
	return can_hw_transmit(id, buf, len);
}
