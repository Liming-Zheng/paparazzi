# Hey Emacs, this is a -*- makefile -*-

ap_CFLAGS += -DUSE_INS
ap_CFLAGS += -DINS_TYPE_H=\"subsystems/ins/ins_alt_float.h\"
ap_srcs   += $(SRC_FIXEDWING)/subsystems/ins.c
ap_srcs   += $(SRC_FIXEDWING)/subsystems/ins/ins_alt_float.c
