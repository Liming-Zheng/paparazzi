
# Actuator drivers for the raw ardrone version are included here

ap.CFLAGS += -DACTUATORS
ap.srcs   += $(SRC_BOARD)/actuators_ardrone2_raw.c
