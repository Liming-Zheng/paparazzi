#include "booz_sensors_model.h"

#include BSM_PARAMS
#include "booz_sensors_model_utils.h"
#include "booz_flight_model.h"

bool_t booz_sensors_model_gps_available() {
  if (bsm.gps_available) {
    bsm.gps_available = FALSE;
    return TRUE;
  }
  return FALSE;
}




void booz_sensors_model_gps_init( double time ) {

  bsm.gps_speed = v_get(AXIS_NB);
  v_zero(bsm.gps_speed);

  bsm.gps_speed_noise_std_dev = v_get(AXIS_NB);
  bsm.gps_speed_noise_std_dev->ve[AXIS_X] = BSM_GPS_SPEED_NOISE_STD_DEV;
  bsm.gps_speed_noise_std_dev->ve[AXIS_Y] = BSM_GPS_SPEED_NOISE_STD_DEV;
  bsm.gps_speed_noise_std_dev->ve[AXIS_Z] = BSM_GPS_SPEED_NOISE_STD_DEV;

  bsm.gps_speed_history = NULL;

  bsm.gps_pos = v_get(AXIS_NB);
  v_zero(bsm.gps_pos);

  bsm.gps_pos_noise_std_dev = v_get(AXIS_NB);
  bsm.gps_pos_noise_std_dev->ve[AXIS_X] = BSM_GPS_POS_NOISE_STD_DEV;
  bsm.gps_pos_noise_std_dev->ve[AXIS_Y] = BSM_GPS_POS_NOISE_STD_DEV;
  bsm.gps_pos_noise_std_dev->ve[AXIS_Z] = BSM_GPS_POS_NOISE_STD_DEV;

  bsm.gps_pos_bias_initial = v_get(AXIS_NB);
  bsm.gps_pos_bias_initial->ve[AXIS_X] = BSM_GPS_POS_BIAS_INITIAL_X;
  bsm.gps_pos_bias_initial->ve[AXIS_Y] = BSM_GPS_POS_BIAS_INITIAL_Y;
  bsm.gps_pos_bias_initial->ve[AXIS_Z] = BSM_GPS_POS_BIAS_INITIAL_Z;

  bsm.gps_pos_bias_random_walk_std_dev = v_get(AXIS_NB);
  bsm.gps_pos_bias_random_walk_std_dev->ve[AXIS_X] = BSM_GPS_POS_BIAS_RANDOM_WALK_STD_DEV_X;
  bsm.gps_pos_bias_random_walk_std_dev->ve[AXIS_Y] = BSM_GPS_POS_BIAS_RANDOM_WALK_STD_DEV_Y;
  bsm.gps_pos_bias_random_walk_std_dev->ve[AXIS_Z] = BSM_GPS_POS_BIAS_RANDOM_WALK_STD_DEV_Z;

  bsm.gps_pos_bias_random_walk_value = v_get(AXIS_NB);
  bsm.gps_pos_bias_random_walk_value->ve[AXIS_X] = bsm.gps_pos_bias_initial->ve[AXIS_X];
  bsm.gps_pos_bias_random_walk_value->ve[AXIS_Y] = bsm.gps_pos_bias_initial->ve[AXIS_Y];
  bsm.gps_pos_bias_random_walk_value->ve[AXIS_Z] = bsm.gps_pos_bias_initial->ve[AXIS_Z];

  bsm.gps_pos_history = NULL;

  bsm.gps_next_update = time;
  bsm.gps_available = FALSE;

}

/* We store our positions like the fdm and convert to UTM and funcky course, gspeed, climb */
void booz_sensors_model_gps_run( double time, MAT* dcm_t ) {
  if (time < bsm.gps_next_update)
    return;

  /* 
   * simulate speed sensor 
   */
  /* extract body speed from state */
  static VEC *speed_body = VNULL;
  speed_body = v_resize(speed_body, AXIS_NB);
  BoozFlighModelGetSpeed(speed_body);
  static VEC *cur_speed_reading = VNULL;
  cur_speed_reading = v_resize(cur_speed_reading, AXIS_NB);
  /* convert to earth frame */
  cur_speed_reading = mv_mlt(dcm_t, speed_body, cur_speed_reading);
  /* add a gaussian noise */
  cur_speed_reading = v_add_gaussian_noise(cur_speed_reading, bsm.gps_speed_noise_std_dev, 
					   cur_speed_reading);
  UpdateSensorLatency(time, cur_speed_reading, bsm.gps_speed_history, BSM_GPS_SPEED_LATENCY, bsm.gps_speed);

  /* course, gspeed, climb convertion */
  bsm.gps_speed_course = DegOfRad(atan2(bsm.gps_speed->ve[AXIS_Y], bsm.gps_speed->ve[AXIS_X]))*10.;
  bsm.gps_speed_course = rint(bsm.gps_speed_course);
  bsm.gps_speed_gspeed = sqrt(bsm.gps_speed->ve[AXIS_X] * bsm.gps_speed->ve[AXIS_X] +
			      bsm.gps_speed->ve[AXIS_Y] * bsm.gps_speed->ve[AXIS_Y]) * 100.;
  bsm.gps_speed_gspeed = rint(bsm.gps_speed_gspeed);
  bsm.gps_speed_climb = rint(-bsm.gps_speed->ve[AXIS_Z] * 100);
  

  /* 
   * simulate position sensor 
   */
  static VEC *cur_pos_reading = VNULL;
  cur_pos_reading = v_resize(cur_pos_reading, AXIS_NB);
  /* extract pos from state */
  BoozFlighModelGetPos(cur_pos_reading);

  /* compute position error reading */
  static VEC *pos_error = VNULL;
  pos_error = v_resize(pos_error, AXIS_NB);
  pos_error = v_zero(pos_error);
  /* add a gaussian noise */
  pos_error = v_add_gaussian_noise(pos_error, bsm.gps_pos_noise_std_dev, pos_error);
  /* update random walk bias */
  bsm.gps_pos_bias_random_walk_value = 
    v_update_random_walk(bsm.gps_pos_bias_random_walk_value, 
  			 bsm.gps_pos_bias_random_walk_std_dev, BSM_GPS_DT, 
  			 bsm.gps_pos_bias_random_walk_value);
  pos_error = v_add(pos_error, bsm.gps_pos_bias_random_walk_value, pos_error); 
  /* add error reading */
  cur_pos_reading = v_add(cur_pos_reading, pos_error, cur_pos_reading); 

  UpdateSensorLatency(time, cur_pos_reading, bsm.gps_pos_history, BSM_GPS_POS_LATENCY, bsm.gps_pos);

  /* UTM conversion */
  bsm.gps_pos_utm_north = bsm.gps_pos->ve[AXIS_X] * 100. + BSM_GPS_POS_INITIAL_UTM_EAST;
  bsm.gps_pos_utm_north = rint(bsm.gps_pos_utm_north);
  bsm.gps_pos_utm_east = bsm.gps_pos->ve[AXIS_Y] * 100. + BSM_GPS_POS_INITIAL_UTM_NORTH;
  bsm.gps_pos_utm_east = rint(bsm.gps_pos_utm_east);
  bsm.gps_pos_utm_alt = bsm.gps_pos->ve[AXIS_Z] * 100. + BSM_GPS_POS_INITIAL_UTM_ALT;
  bsm.gps_pos_utm_alt = rint(bsm.gps_pos_utm_alt);

  /* LLA conversion */

#define LAT0   40.
#define LON0   1.
#define GROUND_ALT  180.

  bsm.gps_pos_lla.lat = (bsm.gps_pos->ve[AXIS_Y] * 9e-6 + LAT0) * 1e7;
  bsm.gps_pos_lla.lat = rint(bsm.gps_pos_lla.lat);
  bsm.gps_pos_lla.lon = (bsm.gps_pos->ve[AXIS_X] * 9e-6 + LON0) * 1e7;
  bsm.gps_pos_lla.lon = rint(bsm.gps_pos_lla.lon);
  bsm.gps_pos_lla.alt = (bsm.gps_pos->ve[AXIS_Z] + GROUND_ALT)* 100.;
  bsm.gps_pos_lla.alt = rint(bsm.gps_pos_lla.alt);

  bsm.gps_next_update += BSM_GPS_DT;
  bsm.gps_available = TRUE;

}

