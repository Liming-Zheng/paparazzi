#
# tiny_1.1.makefile
#
# http://paparazzi.enac.fr/wiki/Tiny_v1.1
#


include $(PAPARAZZI_SRC)/conf/boards/tiny_2.11.makefile


BOARD=tiny
BOARD_VERSION=1.1

#BOARD_CFG=\"boards/$(BOARD)_$(BOARD_VERSION).h\"

# TODO: update syntax
BOARD_CFG = \"tiny_1_1.h\"

GPS_UART_NR	= 1
GPS_LED = none
MODEM_UART_NR = 0
