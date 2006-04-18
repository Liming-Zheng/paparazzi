/*
 * Paparazzi $Id$
 *  
 * Copyright (C) 2006 Pascal Brisset, Antoine Drouin, Michel Gorraz
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

/** \file uart.c
 *  \brief arch independant uart headers
 *
 */

#ifndef UART_H
#define UART_H

#include <inttypes.h>
#include "std.h"
#include "uart_hw.h"

void uart0_init_tx( void );

/** uart0_init_rx() is optional but must be called after uart0_init_tx */
void uart0_init_rx( void );

void uart0_transmit( unsigned char data );
bool_t uart0_check_free_space( uint8_t len);

/** Not necessarily defined */
void uart1_init_tx( void );

/** uart1_init_rx() is optional but must be called after uart1_init_tx */
void uart1_init_rx( void );

void uart1_transmit( unsigned char data );
bool_t uart1_check_free_space( uint8_t len);

#define Uart0Init() { uart0_init_tx(); uart0_init_rx(); }
#define Uart1Init() { uart1_init_tx(); uart1_init_rx(); }

#endif /* UART_H */
