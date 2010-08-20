#
# Autopilot
#
ap.CFLAGS += -DUSE_RADIO_CONTROL
ap.CFLAGS += -DRADIO_CONTROL_TYPE_H=\"radio_control/booz_radio_control_spektrum.h\"

ap.srcs += $(SRC_BOOZ)/booz_radio_control.c \
           $(SRC_BOOZ)/radio_control/booz_radio_control_spektrum.c \
	   $(SRC_BOOZ_ARCH)/radio_control/booz_radio_control_spektrum_arch.c
ifeq ($(ARCHI), arm7)
ap.CFLAGS += -DRADIO_CONTROL_LED=1
ap.CFLAGS += -DUSE_UART0 -DUART0_BAUD=B115200
ap.CFLAGS += -DRADIO_CONTROL_LINK=Uart0
else ifeq ($(ARCHI), stm32)
ap.CFLAGS += -DRADIO_CONTROL_LED=5
ap.CFLAGS += -DUSE_UART3 -DUART3_BAUD=B115200
ap.CFLAGS += -DRADIO_CONTROL_LINK=Uart3
endif

