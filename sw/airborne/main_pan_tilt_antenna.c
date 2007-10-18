#include "std.h"
#include "init_hw.h"
#include "sys_time.h"
#include "led.h"
#include "interrupt_hw.h"
#include "uart.h"

#include "messages.h"
#include "downlink.h"

#include "pt_ant_motors.h"
#include "pt_ant_sensors.h"
#include "pt_ant_estimator.h"
#include "gps.h"

#include "pt_ant_datalink.h"
//#include "traffic_info.h"

static inline void main_init( void );
static inline void main_periodic_task( void );
static inline void main_event_task( void );

int main( void ) {
  main_init();
  while(1) {
    if (sys_time_periodic())
      main_periodic_task();
    main_event_task();
  }
  return 0;
}

static inline void main_init( void ) {
  hw_init();
  sys_time_init();
  led_init();
  Uart0Init();
  //  Uart1Init();
  //  gps_init();
  pt_ant_motors_init();
  pt_ant_sensors_init_spi();
  pt_ant_sensors_init();

  int_enable();

  //  gps_configure_uart();

}

static inline void main_periodic_task( void ) {
  //  PtAntSensorsPeriodic();

#ifdef USE_GPS
  GpsPeriodic();
#endif

  //  LED_TOGGLE(1);
  //  if (cpu_time_sec > 10)
  //    pt_ant_motors_SetZPower(0.2);

}

static inline void main_event_task( void ) {

  //  PtAntSensorsEventCheckAndHandle();
  
  //  DlEventCheckAndHandle();

#ifdef USE_GPS
  if (GpsEventCheckAndHandle())
    return;
#endif  
}
