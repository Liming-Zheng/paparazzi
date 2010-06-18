#ifndef FMS_AUTOPILOT_H
#define FMS_AUTOPILOT_H

#include <inttypes.h>
#include "math/pprz_algebra_int.h"

/*
 * Testing
 */

struct AutopilotMessageFoo {
  uint8_t foo;
  uint8_t bar;
  uint8_t blaa;
  uint8_t bli;
};

union AutopilotMessageFoo1 {
  struct AutopilotMessageFoo up;
  struct AutopilotMessageFoo down;
};


/*
 * BETH
 */
struct AutopilotMessageBethUp {
  struct Int16Vect3 gyro;
  struct Int16Vect3 accel;
  struct Int16Vect3 bench_sensor;

};

struct AutopilotMessageBethDown {
  uint8_t motor_front;
  uint8_t motor_back;
};

union AutopilotMessageBeth {
  struct AutopilotMessageBethUp up;
  struct AutopilotMessageBethDown down;
};


/*
 *  STM Telemetry through wifi
 */
#define TW_BUF_LEN 63
struct AutopilotMessageTWUp {
  uint8_t tw_len;
  uint8_t data[TW_BUF_LEN];
};

struct AutopilotMessageTWDown {
  uint8_t tw_len;
  uint8_t data[TW_BUF_LEN];
};

union AutopilotMessageTW {
  struct AutopilotMessageTWUp up;
  struct AutopilotMessageTWDown down;
};

/*
 * Passthrough, aka biplan
 */
struct AutopilotMessagePTUp {
  struct Int16Vect3 gyro;
  struct Int16Vect3 accel;
  struct Int16Vect3 mag;
  int16_t rc_pitch;
  int16_t rc_roll;
  int16_t rc_yaw;
  int16_t rc_thrust;
  int16_t rc_mode;
  uint8_t  rc_status;
};

struct AutopilotMessagePTDown {
  int16_t command_pitch;
  int16_t command_roll;
  int16_t command_yaw;
  int16_t command_thrust;
};

union AutopilotMessagePT {
  struct AutopilotMessageBethUp up;
  struct AutopilotMessageBethDown down;
};






#endif /* FMS_AUTOPILOT_H */
