
/** \file baro_scp_i2c.c
 *  \brief VTI SCP1000 I2C sensor interface
 *
 *   This reads the values for pressure and temperature from the VTI SCP1000 sensor through I2C.
 */


#include "baro_scp_i2c.h"

#include "sys_time.h"
#include "i2c.h"
#include "led.h"
#include "uart.h"
#include "messages.h"
#include "downlink.h"

uint8_t  baro_scp_status;
uint32_t baro_scp_pressure;
uint16_t baro_scp_temperature;

struct i2c_transaction scp_trans;

#ifndef SCP_I2C_DEV
#define SCP_I2C_DEV i2c0
#endif

#define SCP1000_SLAVE_ADDR 0x22

static void baro_scp_start_high_res_measurement(void) {
  /* write 0x0A to 0x03 */
  scp_trans.buf[0] = 0x03;
  scp_trans.buf[1] = 0x0A;
  I2CTransmit(SCP_I2C_DEV, scp_trans, SCP1000_SLAVE_ADDR, 2);
}

void baro_scp_init( void ) {
  baro_scp_status = BARO_SCP_UNINIT;
}

void baro_scp_periodic( void ) {

  if (baro_scp_status == BARO_SCP_UNINIT && cpu_time_sec > 1) {

    baro_scp_start_high_res_measurement();
    baro_scp_status = BARO_SCP_IDLE;
  } else if (baro_scp_status == BARO_SCP_IDLE) {

    /* init: start two byte temperature */
    scp_trans.buf[0] = 0x81;
    baro_scp_status = BARO_SCP_RD_TEMP;
    I2CTransceive(SCP_I2C_DEV, scp_trans, SCP1000_SLAVE_ADDR, 1, 2);
  }
}

void baro_scp_event( void ) {

  if (scp_trans.status == I2CTransSuccess) {

    if (baro_scp_status == BARO_SCP_RD_TEMP) {

      /* read two byte temperature */
      baro_scp_temperature  = scp_trans.buf[0] << 8;
      baro_scp_temperature |= scp_trans.buf[1];
      if (baro_scp_temperature & 0x2000) {
        baro_scp_temperature |= 0xC000;
      }
      baro_scp_temperature *= 5;

      /* start one byte msb pressure */
      scp_trans.buf[0] = 0x7F;
      baro_scp_status = BARO_SCP_RD_PRESS_0;
      I2CTransceive(SCP_I2C_DEV, scp_trans, SCP1000_SLAVE_ADDR, 1, 1);
    }

    else if (baro_scp_status == BARO_SCP_RD_PRESS_0) {

      /* read one byte pressure */
      baro_scp_pressure = scp_trans.buf[0] << 16;

      /* start two byte lsb pressure */
      scp_trans.buf[0] = 0x80;
      baro_scp_status = BARO_SCP_RD_PRESS_1;
      I2CTransceive(SCP_I2C_DEV, scp_trans, SCP1000_SLAVE_ADDR, 1, 2);
    }

    else if (baro_scp_status == BARO_SCP_RD_PRESS_1) {

      /* read two byte pressure */
      baro_scp_pressure |= scp_trans.buf[0] << 8;
      baro_scp_pressure |= scp_trans.buf[1];
      baro_scp_pressure *= 25;

      DOWNLINK_SEND_SCP_STATUS(DefaultChannel, &baro_scp_pressure, &baro_scp_temperature);

      baro_scp_status = BARO_SCP_IDLE;
    }

    else baro_scp_status = BARO_SCP_IDLE;
  }
}

