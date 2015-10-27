/*
 * Copyright (C) Roland
 *
 * This file is part of paparazzi
 *
 * paparazzi is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * paparazzi is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with paparazzi; see the file COPYING.  If not, see
 * <http://www.gnu.org/licenses/>.
 */
/**
 * @file "modules/readlocationfromodroid/readlocationfromodroid.c"
 * @author Roland
 * reads from the odroid and sends information of the drone to the odroid using JSON messages
 */

#include "modules/readlocationfromodroid/readlocationfromodroid.h"
#include "subsystems/abi.h"
#include "modules/computer_vision/opticflow_module.h"
#include <fcntl.h>
#include <stropts.h>
#include <termios.h>
#include <serial_port.h>
#include <stdio.h>
#include <inttypes.h>
#include <errno.h>
#include <string.h>
#include "state.h"
#include "subsystems/gps.h"
//#include "subsystems/navigation/common_nav.h"
#include "/home/roland/paparazzi/sw/ext/cJSON/cJSON.h"
//#include "cJSON.h"
#include "navdata.h"
#include "subsystems/ins/ins_int.h"
#include "subsystems/datalink/telemetry.h"
#include "modules/computer_vision/opticflow/stabilization_opticflow.h"


#include "firmwares/rotorcraft/autopilot.h"

#include "mcu_periph/uart.h"
#include "subsystems/radio_control.h"
#include "subsystems/commands.h"
#include "subsystems/actuators.h"
#include "subsystems/electrical.h"
#include "subsystems/settings.h"
#include "subsystems/datalink/telemetry.h"
#include "firmwares/rotorcraft/navigation.h"
#include "firmwares/rotorcraft/guidance.h"

#include "firmwares/rotorcraft/stabilization.h"
#include "firmwares/rotorcraft/stabilization/stabilization_none.h"
#include "firmwares/rotorcraft/stabilization/stabilization_rate.h"
#include "firmwares/rotorcraft/stabilization/stabilization_attitude.h"

#include "generated/settings.h"

#include "subsystems/gps.h"
#include "subsystems/abi.h"
#include "navdata.h"
#include "subsystems/ins/ins_int.h"
#include "state.h"
#include "generated/airframe.h"
#include "subsystems/datalink/telemetry.h"
#include "subsystems/gps/stereoprotocol.h"



#include "subsystems/gps.h"
#include "subsystems/abi.h"
#include "navdata.h"
#include "subsystems/ins/ins_int.h"
#include "state.h"
#include "generated/airframe.h"
#include "subsystems/datalink/telemetry.h"
#include "subsystems/gps/stereoprotocol.h"


int frameNumberSending=0;
static void write_serial_rot(struct transport_tx *trans, struct link_device *devasdf) {
	 	struct Int32RMat *ltp_to_body_mat = stateGetNedToBodyRMat_i();
	int32_t lengthArrayInformation = 11*sizeof(int32_t);
	uint8_t ar[lengthArrayInformation];
	int32_t *pointer = (int32_t*)ar;
	for(int indexRot = 0; indexRot < 9; indexRot++){
		pointer[indexRot]=ltp_to_body_mat->m[indexRot];
	}
	pointer[9]=(int32_t)(sonar_meas*100);
	pointer[10]=frameNumberSending++;
	stereoprot_sendArray( &((GPS_LINK).device),ar, lengthArrayInformation, 1);
}

struct SerialPort *READING_port;
speed_t usbInputSpeed = B1000000;
char *serialResponse;
int writeLocationInput=0;

void odroid_loc_init() {
	// Open the serial port
	READING_port = serial_port_new();
	int lengthBytesImage=50000;
	serialResponse=malloc(lengthBytesImage*sizeof(char));
	memset(serialResponse, '0', lengthBytesImage);
	register_periodic_telemetry(DefaultPeriodic, "SERIALRMAT", write_serial_rot);
 }

 static abi_event odroid_agl_ev;
 float lastKnownHeight = 0.0;
 int pleaseResetOdroid = 0;
 #define LENGTH_ODROID_GPS_BUFFER 1024
 char odroid_gps_response[LENGTH_ODROID_GPS_BUFFER];
 //extern int location_odroid_gps_buffer = 0;
 int gps_reset_position = 0;
 int32_t gpsXcm = 0;
 int32_t gpsYcm = 0;
 int32_t gpsVelXcm = 0;
 int32_t gpsVelYcm = 0;

 struct link_device *linkdevodroid;

 typedef struct {
   uint8_t len;
   uint8_t *data;
   uint8_t data_new;
 } uint8array;

 uint8_t msg_buf[256];         // define local data
 uint8array stereocam_data = {.len = 0, .data = msg_buf, .data_new = 0};  // buffer used to contain image without line endings
 uint16_t freq_counter = 0;
 uint16_t frequency = 0;
 uint32_t previous_time = 0;

 #ifndef STEREO_BUF_SIZE
 #define STEREO_BUF_SIZE 1024                     // size of circular buffer
 #endif
 uint8_t ser_read_buf[STEREO_BUF_SIZE];           // circular buffer for incoming data
 uint16_t insert_loc, extract_loc, msg_start;   // place holders for buffer read and write

static void gps_odroid_height(uint8_t sender_id __attribute__((unused)), float distance) {
// Update the distance if we got a valid measurement
if (distance > 0) {
	lastKnownHeight = distance;
}
}


