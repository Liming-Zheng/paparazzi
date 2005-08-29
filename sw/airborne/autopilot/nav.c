/*
 * $Id$
 *  
 * Copyright (C) 2003  Pascal Brisset, Antoine Drouin
 *
 * This file is part of paparazzi.
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
 * along with paparazzi; see the file COPYING.  If not, write to
 * the Free Software Foundation, 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA. 
 *
 */
/** \file nav.c
 *  \brief Regroup functions to compute navigation plopplopplop ::block_time
 *
 */

#define NAV_C

#include <math.h>

#include "nav.h"
#include "estimator.h"
#include "pid.h"
#include "autopilot.h"
#include "link_fbw.h"
#include "airframe.h"
#include "cam.h"

uint8_t nav_stage, nav_block;
/** To save the current stage when an exception is raised */
uint8_t excpt_stage;
static float last_x, last_y;
static uint8_t last_wp;
float rc_pitch;
uint16_t stage_time, block_time;
float stage_time_ds;
float carrot_x, carrot_y;
static float sum_alpha, last_alpha;
/** represents in percent the stage where the uav is on a route
 *  or the qdr when uav circles.
 */
static float alpha;
/** Number of circle done */
float circle_count;
static bool_t new_circle;
bool_t in_circle = FALSE;
bool_t in_segment = FALSE;
int16_t circle_x, circle_y, circle_radius;
int16_t segment_x_1, segment_y_1, segment_x_2, segment_y_2;
uint8_t horizontal_mode;

#define RcRoll(travel) (from_fbw.channels[RADIO_ROLL]* (float)travel /(float)MAX_PPRZ)

#define RcEvent1() CheckEvent(rc_event_1)
#define RcEvent2() CheckEvent(rc_event_2)
#define Block(x) case x: nav_block=x;
#define InitBlock() { nav_stage = 0; block_time = 0; InitStage(); }
#define NextBlock() { nav_block++; InitBlock(); }
#define GotoBlock(b) { nav_block=b; InitBlock(); }

#define Stage(s) case s: nav_stage=s;
#define InitStage() { last_x = estimator_x; last_y = estimator_y; stage_time = 0; stage_time_ds = 0; sum_alpha = 0; new_circle = TRUE; in_circle = FALSE; in_segment = FALSE; return; }
#define NextStage() { nav_stage++; InitStage() }
#define NextStageFrom(wp) { last_wp = wp; NextStage() }
#define GotoStage(s) { nav_stage = s; InitStage() }

#define Label(x) label_ ## x:
#define Goto(x) { goto label_ ## x; }

#define Exception(x) { excpt_stage = nav_stage; goto label_ ## x; }
#define ReturnFromException(_) { GotoStage(excpt_stage) }

static bool_t approaching(uint8_t);
static inline void fly_to_xy(float x, float y);
static void fly_to(uint8_t wp);
static void route_to(uint8_t last_wp, uint8_t wp);
static void glide_to(uint8_t last_wp, uint8_t wp);

#define MIN_DX ((int16_t)(MAX_PPRZ * 0.05))

#define DegOfRad(x) ((x) / M_PI * 180.)
#define RadOfDeg(x) ((x)/180. * M_PI)
#define NormCourse(x) { \
  while (x < 0) x += 360; \
  while (x >= 360) x -= 360; \
}
/** Normalize angle in radian between [-Pi;Pi] */
#define Norm_Rad_Angle(x) { \
    while (x > M_PI) x -= 2 * M_PI; \
    while (x <= -M_PI) x += 2 * M_PI; \
  }

/** Degrees from 0 to 360 */
static float qdr;

#define CircleXY(x, y, radius) { \
	last_alpha = alpha; \
  alpha = atan2(estimator_y - y, \
		      estimator_x - x); \
	if (! new_circle) { \
	  float alpha_diff = alpha - last_alpha; \
		Norm_Rad_Angle(alpha_diff); \
	  sum_alpha += alpha_diff; \
	} \
	else new_circle = FALSE; \
	circle_count = fabs(sum_alpha) / (2*M_PI); \
  float alpha_carrot = alpha + CARROT / -radius * estimator_hspeed_mod; \
  horizontal_mode = HORIZONTAL_MODE_CIRCLE; \
  fly_to_xy(x+cos(alpha_carrot)*fabs(radius), \
						y+sin(alpha_carrot)*fabs(radius)); \
  qdr = DegOfRad(M_PI/2 - alpha_carrot); \
  NormCourse(qdr); \
	in_circle = TRUE; \
	circle_x = x; \
	circle_y = y; \
	circle_radius = radius; \
}

#define MAX_DIST_CARROT 250.
#define MIN_HEIGHT_CARROT 50.
#define MAX_HEIGHT_CARROT 150.

#define Goto3D(radius) { \
  static float carrot_x, carrot_y; \
  if (pprz_mode == PPRZ_MODE_AUTO2) { \
    int16_t yaw = from_fbw.channels[RADIO_YAW]; \
    if (yaw > MIN_DX || yaw < -MIN_DX) { \
      carrot_x += FLOAT_OF_PPRZ(yaw, 0, -20.); \
      carrot_x = Min(carrot_x, MAX_DIST_CARROT); \
      carrot_x = Max(carrot_x, -MAX_DIST_CARROT); \
    } \
    int16_t pitch = from_fbw.channels[RADIO_PITCH]; \
    if (pitch > MIN_DX || pitch < -MIN_DX) { \
      carrot_y += FLOAT_OF_PPRZ(pitch, 0, -20.); \
      carrot_y = Min(carrot_y, MAX_DIST_CARROT); \
      carrot_y = Max(carrot_y, -MAX_DIST_CARROT); \
    } \
    vertical_mode = VERTICAL_MODE_AUTO_ALT; \
    int16_t roll =  from_fbw.channels[RADIO_ROLL]; \
    if (roll > MIN_DX || roll < -MIN_DX) { \
      desired_altitude += FLOAT_OF_PPRZ(roll, 0, -1.0);	\
      desired_altitude = Max(desired_altitude, MIN_HEIGHT_CARROT+GROUND_ALT); \
      desired_altitude = Min(desired_altitude, MAX_HEIGHT_CARROT+GROUND_ALT); \
    } \
  } \
  CircleXY(carrot_x, carrot_y, radius); \
}
#define Circle(wp, radius) \
  CircleXY(waypoints[wp].x, waypoints[wp].y, radius);

#define And(x, y) ((x) && (y))
#define Or(x, y) ((x) || (y))
#define Min(x,y) (x < y ? x : y)
#define Max(x,y) (x > y ? x : y)
#define Qdr(x) (Min(x, 350) < qdr && qdr < x+10)

#include "flight_plan.h"


#define MIN_DIST2_WP (15.*15.)

#define DISTANCE2(p1_x, p1_y, p2) ((p1_x-p2.x)*(p1_x-p2.x)+(p1_y-p2.y)*(p1_y-p2.y))

const int32_t nav_utm_east0 = NAV_UTM_EAST0;
const int32_t nav_utm_north0 = NAV_UTM_NORTH0;
const int8_t nav_utm_zone0 = NAV_UTM_ZONE0;

float desired_altitude = GROUND_ALT + MIN_HEIGHT_CARROT;
float desired_x, desired_y;
uint16_t nav_desired_gaz;
float nav_pitch = NAV_PITCH;
float nav_desired_roll;

float dist2_to_wp, dist2_to_home;
bool_t too_far_from_home;
const uint8_t nb_waypoint = NB_WAYPOINT;

struct point waypoints[NB_WAYPOINT+1] = WAYPOINTS;

/** distance of carrot (in meter) */
static float carrot;

/** static bool_t approaching(uint8_t wp)
 *  \brief Decide if uav is approaching of current waypoint.
 */
/** Computes \a dist2_to_wp and compare it to square \a carrot.
 *  Return true if it is smaller. Else computes by scalar products if 
 *  uav has not gone past waypoint.
 *  Return true if it is the case.
 */
static bool_t approaching(uint8_t wp) {
  /** distance to waypoint in x */
  float pw_x = waypoints[wp].x - estimator_x;
  /** distance to waypoint in y */
  float pw_y = waypoints[wp].y - estimator_y;

  dist2_to_wp = pw_x*pw_x + pw_y *pw_y;
  carrot = CARROT * estimator_hspeed_mod;
  carrot = (carrot < 40 ? 40 : carrot);
  if (dist2_to_wp < carrot*carrot)
    return TRUE;

  float scal_prod = (waypoints[wp].x - last_x) * pw_x + (waypoints[wp].y - last_y) * pw_y;
  
  return (scal_prod < 0);
}

/** static inline void fly_to_xy(float x, float y)
 *  \brief Computes \a desired_x, \a desired_y and \a desired_course.
 */
static inline void fly_to_xy(float x, float y) { 
  desired_x = x;
  desired_y = y;
  desired_course = M_PI/2.-atan2(y - estimator_y, x - estimator_x);
}

/** static void fly_to(uint8_t wp)
 *  \brief Just call \a fly_to_xy with x and y of current waypoint.
 */
static void fly_to(uint8_t wp) { 
  horizontal_mode = HORIZONTAL_MODE_WAYPOINT;
  fly_to_xy(waypoints[wp].x, waypoints[wp].y);
}

/** length of the leg */
static float leg;
/** static void route_to(uint8_t _last_wp, uint8_t wp)
 *  \brief Computes where the uav is on a route and call
 *  \a fly_to_xy to follow it.
 */
static void route_to(uint8_t _last_wp, uint8_t wp) {
  float last_wp_x = waypoints[_last_wp].x;
  float last_wp_y = waypoints[_last_wp].y;
  float leg_x = waypoints[wp].x - last_wp_x;
  float leg_y = waypoints[wp].y - last_wp_y;
  float leg2 = leg_x * leg_x + leg_y * leg_y;
  alpha = ((estimator_x - last_wp_x) * leg_x + (estimator_y - last_wp_y) * leg_y) / leg2;
  alpha = Max(alpha, 0.);
  leg = sqrt(leg2);
  /** carrot is computed in approaching() */
  alpha += Max(carrot / leg, 0.);
  alpha = Min(1., alpha);
  in_segment = TRUE;
  segment_x_1 = last_wp_x;
  segment_y_1 = last_wp_y;
  segment_x_2 = waypoints[wp].x;
  segment_y_2 = waypoints[wp].y;
  horizontal_mode = HORIZONTAL_MODE_ROUTE;

  fly_to_xy(last_wp_x + alpha*leg_x, last_wp_y + alpha*leg_y);
}

/** static void glide_to(uint8_t _last_wp, uint8_t wp)
 *  \brief Computes \a desired_altitude and \a pre_climb to stay on a glide.
 */
static void glide_to(uint8_t _last_wp, uint8_t wp) {
  float last_alt = waypoints[_last_wp].a;
  desired_altitude = last_alt + alpha * (waypoints[wp].a - last_alt);
  pre_climb = NOMINAL_AIRSPEED * (waypoints[wp].a - last_alt) / leg;
}

/** static inline void compute_dist2_to_home(void)
 *  \brief Compute square distance to waypoint Home and
 *  verify uav is not too far away from Home.
 */
static inline void compute_dist2_to_home(void) {
  float ph_x = waypoints[WP_HOME].x - estimator_x;
  float ph_y = waypoints[WP_HOME].y - estimator_y;
  dist2_to_home = ph_x*ph_x + ph_y *ph_y;
  too_far_from_home = dist2_to_home > (MAX_DIST_FROM_HOME*MAX_DIST_FROM_HOME);
}

/** void nav_home(void)
 *  \brief Occurs when it switchs in Home mode.
 */
void nav_home(void) {
  /** FIXME: radius should be defined elsewhere */
#ifdef FAILSAFE_HOME_RADIUS
  Circle(WP_HOME, FAILSAFE_HOME_RADIUS);
#else
  Circle(WP_HOME, 50);
#endif
  /** Nominal speed */ 
  nav_pitch = 0.;
  vertical_mode = VERTICAL_MODE_AUTO_ALT;
  desired_altitude = GROUND_ALT+50;
  compute_dist2_to_home();
  dist2_to_wp = dist2_to_home;
}

/** void nav_update(void)
 *  \brief Update navigation
 */
void nav_update(void) {
  compute_dist2_to_home();

  auto_nav();
}


/** void nav_init(void)
 *  \brief Initialisation of navigation
 */
void nav_init(void) {
  nav_block = 0;
  nav_stage = 0;
}

/** void nav_wihtout_gps(void)
 *  \brief 
 */
/** Don't navigate anymore. It's impossible without GPS.
 *	Just set attitude and gaz to failsafe values
 *	to prevent the plane from crashing.
 */
void nav_wihtout_gps(void) {
#ifdef SECTION_FAILSAFE
	lateral_mode = LATERAL_MODE_ROLL;
	nav_desired_roll = FAILSAFE_DEFAULT_ROLL;
	auto_pitch = FALSE;
	nav_pitch = FAILSAFE_DEFAULT_PITCH;
	vertical_mode = VERTICAL_MODE_AUTO_GAZ;
	nav_desired_gaz = TRIM_UPPRZ(FAILSAFE_DEFAULT_GAZ*MAX_PPRZ);
#else
	lateral_mode = LATERAL_MODE_ROLL;
	nav_desired_roll = 0;
	auto_pitch = FALSE;
	nav_pitch = 0;
	vertical_mode = VERTICAL_MODE_AUTO_GAZ;
	nav_desired_gaz = TRIM_UPPRZ(CLIMB_LEVEL_GAZ*MAX_PPRZ);
#endif
}
