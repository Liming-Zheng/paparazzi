# attitude estimation for fixedwings via dcm algorithm


ifeq ($(ARCH), lpc21)
ap.CFLAGS += -DUSE_ANALOG_IMU

ap.srcs += $(SRC_SUBSYSTEMS)/ahrs/dcm/dcm.c
ap.srcs += $(SRC_SUBSYSTEMS)/ahrs/dcm/analogimu.c

endif

# since there is currently no SITL sim for the Analog IMU, we use the infrared sim

ifeq ($(TARGET), sim)

sim.CFLAGS += -DIR_ROLL_NEUTRAL_DEFAULT=0
sim.CFLAGS += -DIR_PITCH_NEUTRAL_DEFAULT=0

sim.CFLAGS += -DUSE_INFRARED
sim.srcs += subsystems/sensors/infrared.c

sim.srcs += $(SRC_ARCH)/sim_ir.c
sim.srcs += $(SRC_ARCH)/sim_analogimu.c

endif

jsbsim.srcs += $(SRC_ARCH)/jsbsim_ir.c
