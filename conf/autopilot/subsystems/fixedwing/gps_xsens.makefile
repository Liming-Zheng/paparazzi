# XSens Mti-G


# ap.CFLAGS += -DGPS

ap.srcs   += $(SRC_FIXEDWING)/gps_xsens.c $(SRC_FIXEDWING)/gps.c

sim.srcs += $(SRC_FIXEDWING)/gps.c
