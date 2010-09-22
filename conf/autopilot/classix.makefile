# Makefile for the Classix board (2 arm7tdmi)

ARCH=arm7

ap.ARCHDIR = $(ARCH)


fbw.ARCHDIR = $(ARCH)

test.ARCHDIR = $(ARCH)

LPC21ISP_BAUD = 38400
LPC21ISP_XTAL = 12000

# A test program to monitor the ADC values
test_adcs.ARCHDIR = $(ARCH)

test_adcs.CFLAGS += -DAP -DBOARD_CONFIG=\"classix.h\" -DLED -DTIME_LED=2 -DADC -DUSE_AD1 -DUSE_AD1_2 -DUSE_AD1_3 -DUSE_AD1_4 -DUSE_AD1_5  -DUSE_AD1_6  -DUSE_AD1_7
test_adcs.CFLAGS += -DDOWNLINK -DUSE_UART0 -DDOWNLINK_TRANSPORT=PprzTransport -DDOWNLINK_DEVICE=Uart0 -DPPRZ_UART=Uart0 -DDATALINK=PPRZ -DUART0_BAUD=B57600
test_adcs.srcs += downlink.c $(SRC_ARCH)/uart_hw.c

test_adcs.srcs += sys_time.c $(SRC_ARCH)/adc_hw.c $(SRC_ARCH)/sys_time_hw.c $(SRC_ARCH)/armVIC.c pprz_transport.c test_adcs.c

# a test program to setup actuators
# Message to send on Ivy bus, 1500 on servo 3 for aircraft 16
# xxx RAW_DATALINK 16 SET_ACTUATOR;1500;3
setup_actuators.ARCHDIR = $(ARCH)

setup_actuators.CFLAGS += -DFBW -DBOARD_CONFIG=\"classix.h\" -DLED -DTIME_LED=2 -DACTUATORS=\"servos_4017_hw.h\" -DSERVOS_4017 -DSERVOS_4017_CLOCK_FALLING -DUSE_UART0 -DUART0_BAUD=B57600 -DDATALINK=PPRZ -DPPRZ_UART=Uart0
setup_actuators.srcs += sys_time.c $(SRC_ARCH)/sys_time_hw.c $(SRC_ARCH)/armVIC.c pprz_transport.c setup_actuators.c $(SRC_ARCH)/uart_hw.c $(SRC_ARCH)/servos_4017_hw.c main.c
