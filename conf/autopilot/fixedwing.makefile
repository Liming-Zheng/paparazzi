#
# fixed_wings.makefile
#
#


CFG_FIXEDWING=$(PAPARAZZI_SRC)/conf/autopilot/subsystems/fixedwing


SRC_FIXEDWING=.
SRC_FIXEDWING_ARCH=$(SRC_FIXEDWING)/$(ARCH)
SRC_FIXEDWING_TEST=$(SRC_FIXEDWING)/

FIXEDWING_INC = -I$(SRC_FIXEDWING) -I$(SRC_FIXEDWING_ARCH)




# Standard Fixed Wing Code
include $(CFG_FIXEDWING)/autopilot.makefile

