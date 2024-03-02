/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 * File: xgemm.h
 *
 * MATLAB Coder version            : 23.2
 * C/C++ source code generated on  : 02-Mar-2024 00:35:29
 */

#ifndef XGEMM_H
#define XGEMM_H

/* Include Files */
#include "rtwtypes.h"
#include <stddef.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Function Declarations */
void b_xgemm(int m, int n, int k, const double A[961], int ia0,
             const double B[496], double C[961]);

void xgemm(int m, int n, int k, const double A[225], int lda,
           const double B[961], int ib0, double C[496]);

#ifdef __cplusplus
}
#endif

#endif
/*
 * File trailer for xgemm.h
 *
 * [EOF]
 */
