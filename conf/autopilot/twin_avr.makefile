ARCHI=avr

ap.ARCHDIR = $(ARCHI)
ap.ARCH = atmega128
ap.TARGET = autopilot
ap.TARGETDIR = autopilot
ap.LOW_FUSE  = a0
ap.HIGH_FUSE = 99
ap.EXT_FUSE  = ff
ap.LOCK_FUSE = ff
ap.CFLAGS += -DAP -DMODEM -DDOWNLINK -DMCU_SPI_LINK
ap.srcs = gps_ubx.c gps.c main_ap.c $(SRC_ARCH)/modem_hw.c inter_mcu.c link_mcu.c $(SRC_ARCH)/link_mcu_ap.c $(SRC_ARCH)/spi_ap.c $(SRC_ARCH)/adc_hw.c infrared.c pid.c nav.c $(SRC_ARCH)/uart_hw.c estimator.c mainloop.c cam.c sys_time.c main.c

fbw.ARCHDIR = $(ARCHI)
fbw.ARCH = atmega8
fbw.TARGET = fbw
fbw.TARGETDIR = fbw
fbw.LOW_FUSE  = 2e
fbw.HIGH_FUSE = cb
fbw.EXT_FUSE  = ff
fbw.LOCK_FUSE = ff
fbw.CFLAGS += -DFBW -DRADIO_CONTROL -DINTER_MCU -DMCU_SPI_LINK -DACTUATORS=\"servos_4017.h\"
fbw.srcs = $(SRC_ARCH)/ppm_hw.c commands.c radio_control.c inter_mcu.c $(SRC_ARCH)/servos_4017.c $(SRC_ARCH)/spi_fbw.c $(SRC_ARCH)/uart_hw.c $(SRC_ARCH)/adc_hw.c sys_time.c main_fbw.c main.c
