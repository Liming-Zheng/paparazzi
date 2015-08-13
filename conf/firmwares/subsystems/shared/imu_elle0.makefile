# Hey Emacs, this is a -*- makefile -*-
#
# MPU9250 IMU via SPI
# Basically the same as conf/firmwares/subsystems/shared/imu_mpu9250_spi.makefile
# Only changed axes assignment for Elle0
#

include $(CFG_SHARED)/imu_mpu9250_spi.makefile

# define the axes for Elle0
# add it for all targets except sim, fbw and nps
ifeq (,$(findstring $(TARGET),sim fbw nps))
$(TARGET).CFLAGS += -DIMU_MPU9250_CHAN_X=1 -DIMU_MPU9250_CHAN_Y=0 -DIMU_MPU9250_CHAN_Z=2
$(TARGET).CFLAGS += -DIMU_GYRO_P_SIGN=-1 -DIMU_GYRO_Q_SIGN=-1 -DIMU_GYRO_R_SIGN=-1
$(TARGET).CFLAGS += -DIMU_ACCEL_X_SIGN=-1 -DIMU_ACCEL_Y_SIGN=-1 -DIMU_ACCEL_Z_SIGN=-1
$(TARGET).CFLAGS += -DIMU_MAG_X_SIGN=-1 -DIMU_MAG_Y_SIGN=-1 -DIMU_MAG_Z_SIGN=-1
endif
