#
# $Id$
#  
# Copyright (C) 2008 Antoine Drouin
#
# This file is part of paparazzi.
#
# paparazzi is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or (at your option)
# any later version.
#
# paparazzi is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with paparazzi; see the file COPYING.  If not, write to
# the Free Software Foundation, 59 Temple Place - Suite 330,
# Boston, MA 02111-1307, USA. 
#
#

ARCHI=arm7

FLASH_MODE = IAP
BOARD_CFG = \"booz2_board_v1_0.h\"

BOOZ=booz
BOOZ_PRIV=booz_priv
BOOZ_PRIV_ARCH=booz_priv/arm7
BOOZ_PRIV_TEST=booz_priv/test
BOOZ_ARCH=booz/arm7

BOOZ_CFLAGS = -I$(BOOZ_PRIV) -I$(BOOZ_PRIV_ARCH)

ap.ARCHDIR = $(ARCHI)
ap.ARCH = arm7tdmi
ap.TARGET = ap
ap.TARGETDIR = ap

ap.CFLAGS += -DKILL_MOTORS

ap.CFLAGS += -DCONFIG=$(BOARD_CFG) $(BOOZ_CFLAGS)
ap.srcs += $(BOOZ_PRIV)/booz2_main.c
ap.CFLAGS += -DPERIODIC_TASK_PERIOD='SYS_TICS_OF_SEC((1./512.))'
# -DTIME_LED=1
ap.CFLAGS += -DLED
ap.srcs += sys_time.c $(SRC_ARCH)/sys_time_hw.c $(SRC_ARCH)/armVIC.c

ap.CFLAGS += -DUSE_UART1 -DUART1_BAUD=B57600 -DUART1_VIC_SLOT=6
ap.srcs += $(SRC_ARCH)/uart_hw.c

ap.CFLAGS += -DDOWNLINK -DDOWNLINK_TRANSPORT=PprzTransport -DDOWNLINK_DEVICE=Uart1 
ap.srcs += $(BOOZ_PRIV)/booz2_telemetry.c downlink.c pprz_transport.c

ap.CFLAGS += -DDATALINK=PPRZ -DPPRZ_UART=Uart1
ap.srcs += $(BOOZ_PRIV)/booz2_datalink.c

ap.srcs += commands.c

ap.CFLAGS += -DRADIO_CONTROL -DRADIO_CONTROL_TYPE=RC_FUTABA -DRC_LED=1
ap.srcs += radio_control.c $(SRC_ARCH)/ppm_hw.c

ap.CFLAGS += -DACTUATORS=\"actuators_buss_twi_blmc_hw.h\" -DUSE_BUSS_TWI_BLMC
ap.srcs += $(BOOZ_PRIV_ARCH)/actuators_buss_twi_blmc_hw.c actuators.c
ap.CFLAGS += -DUSE_I2C0 -DI2C0_SCLL=150 -DI2C0_SCLH=150 -DI2C0_VIC_SLOT=10
ap.srcs += i2c.c $(SRC_ARCH)/i2c_hw.c


#ap.CFLAGS += -DBOOZ2_IMU_TYPE=IMU_V3
#ap.srcs += $(BOOZ_PRIV)/booz2_imu_v3.c $(BOOZ_PRIV_ARCH)/booz2_imu_v3_hw.c

#ap.CFLAGS += -DBOOZ2_IMU_TYPE=IMU_CRISTA
#ap.srcs += $(BOOZ_PRIV)/booz2_imu_crista.c $(BOOZ_PRIV_ARCH)/booz2_imu_crista_hw.c

ap.CFLAGS += -DBOOZ2_IMU_TYPE=IMU_B2
ap.CFLAGS += -DSSP_VIC_SLOT=9
ap.srcs += $(BOOZ_PRIV)/booz2_imu_b2.c $(BOOZ_PRIV_ARCH)/booz2_imu_b2_hw.c
ap.CFLAGS += -DMAX1168_EOC_VIC_SLOT=8
ap.srcs += $(BOOZ_PRIV)/booz2_max1168.c $(BOOZ_PRIV_ARCH)/booz2_max1168_hw.c
ap.CFLAGS += -DFLOAT_T=float
ap.srcs += $(BOOZ_PRIV)/booz2_imu.c

ap.CFLAGS += -DADC1_VIC_SLOT=2
ap.CFLAGS += -DBOOZ2_ANALOG_BARO_LED=2 -DBOOZ2_ANALOG_BARO_PERIOD='SYS_TICS_OF_SEC((1./100.))'
ap.srcs += $(BOOZ_PRIV)/booz2_analog_baro.c


ap.CFLAGS += -DUSE_I2C1  -DI2C1_SCLL=150 -DI2C1_SCLH=150 -DI2C1_VIC_SLOT=11 -DI2C1_BUF_LEN=16
#ap.srcs += i2c.c $(SRC_ARCH)/i2c_hw.c
ap.CFLAGS += -DUSE_AMI601
ap.srcs += AMI601.c


ap.srcs += $(BOOZ_PRIV)/booz2_autopilot.c

ap.CFLAGS += -DFILTER_ALIGNER_LED=3
ap.srcs += $(BOOZ_PRIV)/booz2_filter_aligner2.c
ap.srcs += $(BOOZ_PRIV)/booz2_filter_attitude_cmpl_euler.c
ap.srcs += $(BOOZ_PRIV)/booz_trig_int.c

ap.CFLAGS += -DUSE_UART0 -DUART0_BAUD=B38400 -DUART0_VIC_SLOT=5
ap.CFLAGS += -DGPS_LINK=Uart0 -DGPS_LED=4
ap.srcs += $(BOOZ_PRIV)/booz2_gps.c
ap.srcs += $(BOOZ_PRIV)/booz2_guidance_h.c
ap.srcs += $(BOOZ_PRIV)/booz2_guidance_v.c

#ap.CFLAGS += -DNAV_ENAC
ap.srcs += $(BOOZ_PRIV)/booz2_navigation.c


ap.srcs += $(BOOZ_PRIV)/booz2_stabilization.c
ap.srcs += $(BOOZ_PRIV)/booz2_stabilization_rate.c
ap.srcs += $(BOOZ_PRIV)/booz2_stabilization_attitude.c
ap.srcs += $(BOOZ_PRIV)/booz_supervision_int.c

# Joystick
ap.CFLAGS += -DUSE_DATALINK_FMS
ap.srcs += $(BOOZ_PRIV)/booz2_fms.c