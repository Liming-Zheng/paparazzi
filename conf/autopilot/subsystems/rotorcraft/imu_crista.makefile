#
# Rotorcraft IMU crista
#
#
# required xml:
#  <section name="IMU" prefix="IMU_">
#
#    <define name="GYRO_X_CHAN" value="1"/>
#    <define name="GYRO_Y_CHAN" value="0"/>
#    <define name="GYRO_Z_CHAN" value="2"/>
#
#    <define name="GYRO_X_SIGN" value="1"/>
#    <define name="GYRO_Y_SIGN" value="1"/>
#    <define name="GYRO_Z_SIGN" value="1"/>
#
#    <define name="GYRO_X_NEUTRAL" value="33924"/>
#    <define name="GYRO_Y_NEUTRAL" value="33417"/>
#    <define name="GYRO_Z_NEUTRAL" value="32809"/>
#
#    <define name="GYRO_X_SENS" value="1.01" integer="16"/>
#    <define name="GYRO_Y_SENS" value="1.01" integer="16"/>
#    <define name="GYRO_Z_SENS" value="1.01" integer="16"/>
#
#    <define name="ACCEL_X_CHAN" value="3"/>
#    <define name="ACCEL_Y_CHAN" value="5"/>
#    <define name="ACCEL_Z_CHAN" value="6"/>
#
#    <define name="ACCEL_X_SIGN" value="1"/>
#    <define name="ACCEL_Y_SIGN" value="1"/>
#    <define name="ACCEL_Z_SIGN" value="1"/>
#
#    <define name="ACCEL_X_NEUTRAL" value="32081"/>
#    <define name="ACCEL_Y_NEUTRAL" value="33738"/>
#    <define name="ACCEL_Z_NEUTRAL" value="32441"/>
#
#    <define name="ACCEL_X_SENS" value="2.50411474" integer="16"/>
#    <define name="ACCEL_Y_SENS" value="2.48126183" integer="16"/>
#    <define name="ACCEL_Z_SENS" value="2.51396167" integer="16"/>
#
#    <define name="MAG_X_CHAN" value="4"/>
#    <define name="MAG_Y_CHAN" value="0"/>
#    <define name="MAG_Z_CHAN" value="2"/>
#
#    <define name="MAG_X_SIGN" value="1"/>
#    <define name="MAG_Y_SIGN" value="1"/>
#    <define name="MAG_Z_SIGN" value="1"/>
#
#    <define name="MAG_X_NEUTRAL" value="2358"/>
#    <define name="MAG_Y_NEUTRAL" value="2362"/>
#    <define name="MAG_Z_NEUTRAL" value="2119"/>
#
#    <define name="MAG_X_SENS" value="3.4936416" integer="16"/>
#    <define name="MAG_Y_SENS" value="3.607713" integer="16"/>
#    <define name="MAG_Z_SENS" value="4.90788848" integer="16"/>
#    <define name="MAG_45_HACK" value="1"/>
#
#  </section>
#
#



# add imu arch to include directories
ap.CFLAGS += -I$(SRC_SUBSYSTEMS)/imu/arch/$(ARCH)

ap.CFLAGS += -DIMU_TYPE_H=\"imu/imu_crista.h\"
ap.srcs += $(SRC_SUBSYSTEMS)/imu.c            \
           $(SRC_SUBSYSTEMS)/imu/imu_crista.c \
           $(SRC_SUBSYSTEMS)/imu/arch/$(ARCH)/imu_crista_arch.c

ap.CFLAGS += -DUSE_AMI601
ap.srcs   += peripherals/ami601.c
ap.CFLAGS += -DUSE_I2C1  -DI2C1_SCLL=150 -DI2C1_SCLH=150 -DI2C1_VIC_SLOT=11 -DI2C1_BUF_LEN=16


#
# Simulator
#

# add imu arch to include directories
sim.CFLAGS += -I$(SRC_SUBSYSTEMS)/imu/arch/$(ARCH)

sim.CFLAGS += -DIMU_TYPE_H=\"imu/imu_crista.h\"
sim.srcs   += $(SRC_SUBSYSTEMS)/imu.c                 \
              $(SRC_SUBSYSTEMS)/imu/imu_crista.c     \
              $(SRC_SUBSYSTEMS)/imu/arch/$(ARCH)/imu_crista_arch.c

sim.CFLAGS += -DUSE_AMI601
sim.srcs   += peripherals/ami601.c
sim.CFLAGS += -DUSE_I2C1
