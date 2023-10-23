/*
 * Copyright (C) 2022 Gervase Lovell-Prescod <gervase.lovellprescod@gmail.com>
 */

/** @file modules/nav/nav_pivot_takeoff_landing.c
 *
 */

#include "generated/airframe.h"
#include "state.h"
#include "modules/nav/nav_pivot_takeoff_landing.h"
#include "autopilot.h"
#include "firmwares/rotorcraft/stabilization/stabilization_indi.h"
//#include "firmwares/rotorcraft/stabilization/stabilization_"
#include "modules/actuators/actuators.h"
#include "modules/nav/common_flight_plan.h"
#include "navigation.h"
#include "firmwares/rotorcraft/guidance/guidance_indi_hybrid.h"

extern struct FloatEulers guidance_euler_cmd;
struct FloatEulers *att_euler;
static struct nav_pivot nav_pivot;

#ifndef PIVOT_DURATION
#define PIVOT_DURATION 15.0
#endif

#ifndef PIVOT_GOAL
#define PIVOT_GOAL 0.0
#endif

#ifndef PIVOT_INITIAL
#define PIVOT_INITIAL -60.0
#endif

static float rad_to_pprz(float x);

void nav_pivot_ramp_init(void)
{
	att_euler = stateGetNedToBodyEulers_f();
	nav_pivot.initial = PIVOT_INITIAL;
	nav_pivot.duration = PIVOT_DURATION;
	nav_pivot.goal = PIVOT_GOAL;
	int8_t i;
//	for (i = 0; i < 2; i++) {
		actuators_pprz[1] = (int16_t) 9500.0;
		actuators_pprz[2] = (int16_t) 9500.0;
//	}

}

bool nav_pivot_ramp(void)
{
	// Start an
	if (att_euler->theta > RadOfDeg(-5.0)){
		actuators_pprz[2] = (int16_t) 6000;
		return false;
	}

//	float cmd_pprz = AbsBound(rad_to_pprz(-1*(att_euler->theta)), MAX_PPRZ);
	actuators_pprz[2] = (int16_t) 6000;

//	float pivot_time = (block_time < PIVOT_DURATION) ? block_time : PIVOT_DURATION;
////	printf("pivot time: %f\n", pivot_time);
//
//	guidance_euler_cmd.phi = 0.0;
//	guidance_euler_cmd.theta = RadOfDeg(((nav_pivot.goal - nav_pivot.initial) / nav_pivot.duration) * pivot_time + nav_pivot.initial);
//	guidance_euler_cmd.psi = 0.0;
//
//	struct FloatQuat sp_quat;
//	float_quat_of_eulers_zxy(&sp_quat, &guidance_euler_cmd);
//	float_quat_normalize(&sp_quat);
//	QUAT_BFP_OF_REAL(stab_att_sp_quat,sp_quat);
//
//	printf("guidance theta: %f\n", sp_quat.qy);
//
//	att_euler = stateGetNedToBodyEulers_f();
	return true;
}

float rad_to_pprz(float x)
{

	float m = 0.0001;
	return x/m;
}


