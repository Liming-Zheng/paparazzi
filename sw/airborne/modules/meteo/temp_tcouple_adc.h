#ifndef TEMP_TCOUPLE_ADC_H
#define TEMP_TCOUPLE_ADC_H

#include "std.h"

#define TCOUPLE_NB 4

extern uint16_t up[TCOUPLE_NB], dn[TCOUPLE_NB];
extern int32_t  tcouple_cnt;

void temp_tcouple_adc_init(void);
void temp_tcouple_adc_periodic(void);

#endif
