/*
 * Copyright (C) 2021 Gervase Lovell-Prescod <gervase.prescod@gmail.com>
 */

/**
 * @file modules/ctrl/ctrl_effectiveness_calculator.h
 */

#ifndef CTRL_EFFECTIVENESS_CALCULATOR_H
#define CTRL_EFFECTIVENESS_CALCULATOR_H

#include "generated/airframe.h"
#include "firmwares/rotorcraft/stabilization/stabilization_indi.h"
#include "paparazzi.h"

struct MassProperties {
	float mass;
	float I_xx;
	float I_yy;
	float I_zz;
};

struct MotorCoefficients {
	float k1;
	float k2;
	float k3;
};

extern float c_delta_a;
extern float cda_offset;

extern float cde_offset;
extern float c_delta_e;
extern float mapping;

extern struct MassProperties mass_property;
extern float thrust_lower_lim;
extern float airspeed_scaling;

extern void ctrl_eff_calc_init(void);
extern void ctrl_eff_periodic(void);
extern void ctrl_eff(void);
extern void ctrl_eff_ground_contact(void);

#endif  /* CTRL_EFFECTIVENESS_CALCULATOR_H */
