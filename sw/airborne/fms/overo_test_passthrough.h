#ifndef OVERO_TEST_PASSTHROUGH_H
#define OVERO_TEST_PASSTHROUGH_H

#include "std.h"
#include "imu.h"

struct OveroTestPassthrough {
  /* our network connection */
  char* gs_gw;

  /* our sensors            */
  struct ImuFloat imu;
  uint8_t  rc_status;
  int32_t  baro_abs;
  int32_t  baro_diff;

  /* our actuators          */
  uint16_t servos_outputs_usecs[6]; /* FIXME */

  /* the io proc status */
  uint32_t io_proc_msg_cnt;
  uint32_t io_proc_err_cnt;
  };

extern struct OveroTestPassthrough otp;

#endif /* OVERO_TEST_PASSTHROUGH_H */
