
#include "rotorcraft/baro.h"

struct Baro baro;
struct BaroBoard baro_board;

static inline void baro_board_write_to_register(uint8_t baro_addr, uint8_t reg_addr, uint8_t val_msb, uint8_t val_lsb);
static inline void baro_board_read_from_register(uint8_t baro_addr, uint8_t reg_addr);
static inline void baro_board_set_current_register(uint8_t baro_addr, uint8_t reg_addr);
static inline void baro_board_read_from_current_register(uint8_t baro_addr);

// absolute
#define BARO_ABS_ADDR  0x90   
// differential
#define BARO_DIFF_ADDR 0x92

void baro_init(void) {
  baro.status = BS_UNINITIALIZED;
  baro.absolute     = 0;
  baro.differential = 0;
  baro_board.status = LBS_UNINITIALIZED;
}


void baro_periodic(void) {
  // check i2c_done
  switch (baro_board.status) {
  case LBS_UNINITIALIZED:
    baro_board_send_reset();
    baro_board.status = LBS_RESETED;
    break;
  case LBS_RESETED:
    baro_board_send_config_abs();
    baro_board.status = LBS_INITIALIZING_ABS;
    break;
  case LBS_INITIALIZING_ABS:
    baro_board_set_current_register(BARO_ABS_ADDR, 0x00);
    baro_board.status = LBS_INITIALIZING_ABS_1;
    break;
  case LBS_INITIALIZING_ABS_1:
    baro_board_send_config_diff();
    baro_board.status = LBS_INITIALIZING_DIFF;
    break;
  case LBS_INITIALIZING_DIFF:
    baro_board_set_current_register(BARO_DIFF_ADDR, 0x00);
    baro_board.status = LBS_INITIALIZING_DIFF_1;
    //    baro_board.status = LBS_UNINITIALIZED;
    break;
  case LBS_INITIALIZING_DIFF_1:
  case LBS_READ_DIFF:
    baro_board_read_from_current_register(BARO_ABS_ADDR);
    baro_board.status = LBS_READING_ABS;
    break;
  case LBS_READ_ABS:
    baro_board_read_from_current_register(BARO_DIFF_ADDR);
    baro_board.status = LBS_READING_DIFF;
    break;
  default:
    break;
  }

}


void baro_board_send_reset(void) {
  i2c2.buf[0] = 0x06;
  i2c2_transmit(0x00, 1, &baro_board.i2c_done);
}

void baro_board_send_config_abs(void) {
  baro_board_write_to_register(BARO_ABS_ADDR, 0x01, 0x86, 0x83);
}

void baro_board_send_config_diff(void) {
  baro_board_write_to_register(BARO_DIFF_ADDR, 0x01, 0x84, 0x83);
}

static inline void baro_board_write_to_register(uint8_t baro_addr, uint8_t reg_addr, uint8_t val_msb, uint8_t val_lsb) {
  i2c2.buf[0] = reg_addr;
  i2c2.buf[1] = val_msb;
  i2c2.buf[2] = val_lsb;
  i2c2_transmit(baro_addr, 3, &baro_board.i2c_done);
}

static inline void baro_board_read_from_register(uint8_t baro_addr, uint8_t reg_addr) {
  i2c2.buf[0] = reg_addr;
  i2c2_transceive(baro_addr, 1, 2, &baro_board.i2c_done);
}

static inline void baro_board_set_current_register(uint8_t baro_addr, uint8_t reg_addr) {
  i2c2.buf[0] = reg_addr;
  i2c2_transmit(baro_addr, 1, &baro_board.i2c_done);
}

static inline void baro_board_read_from_current_register(uint8_t baro_addr) {
  i2c2_receive(baro_addr, 2, &baro_board.i2c_done);
}
