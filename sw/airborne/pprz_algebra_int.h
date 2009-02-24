#ifndef PPRZ_ALGEBRA_INT_H
#define PPRZ_ALGEBRA_INT_H


#include "std.h"
#include "pprz_algebra.h"

// TO BE REMOVED
#include "booz_geometry_int.h"

struct Int32Vect2 {
  int32_t x;
  int32_t y;
};

struct Int32Vect3 {
  int32_t x;
  int32_t y;
  int32_t z;
};

/* Rotation quaternions                         */
#define INT32_QUAT_FRAC 15
struct Int32Quat {
  int32_t qi;
  int32_t qx;
  int32_t qy;
  int32_t qz;
};
/* Euler angles                                 */
#define INT32_ANGLE_FRAC 12
struct Int32Eulers {
  int32_t phi;
  int32_t theta;
  int32_t psi;
};

/* Rotation matrix. */
#define INT32_TRIG_FRAC 14
struct Int32Rmat {
  int32_t m[9];
};

/* 3x3 matrix                                    */
struct Int32Mat33 {
  int32_t m[3*3];
};

/* Rotational speed                              */
struct Int32Rates {
  int32_t p;
  int32_t q;
  int32_t r;
};




struct Int64Vect2 {
  int64_t x;
  int64_t y;
};

struct Int64Vect3 {
  int64_t x;
  int64_t y;
  int64_t z;
};


#define INT_BFP(_v, _frac) ((_v)*(1<<(_frac)))
#define ANGLE_BFP(_v) INT_BFP(_v, INT32_ANGLE_FRAC)
#define QUAT_BFP(_v)  INT_BFP(_v, INT32_QUAT_FRAC)

#define INT_MULT_RSHIFT(_a, _b, _r) (((_a)*(_b))>>(_r))
/*
 * Dimension 2 Vectors
 */

#define INT_VECT2_ZERO(_o) VECT2_ASSIGN(_o, 0, 0)

#define INT32_VECT2_NORM(n, v) {					\
    int32_t n2 = v.x*v.x + v.y*v.y;					\
    INT32_SQRT(n, n2);							\
  }

/*
 * Dimension 3 Vectors
 */

#define INT_VECT3_ZERO(_o) VECT3_ASSIGN(_o, 0, 0, 0)
#define INT32_VECT3_ZERO(_o) VECT3_ASSIGN(_o, 0, 0, 0)

#define INT32_VECT3_COPY(_o, _i) {		\
    _o.x = _i.x;				\
    _o.y = _i.y;				\
    _o.z = _i.z;				\
  }

#define INT32_VECT3_DIFF(_c, _a, _b) {		\
    _c.x = _a.x - _b.x;				\
    _c.y = _a.y - _b.y;				\
    _c.z = _a.z - _b.z;				\
  }

#define INT32_VECT3_SCALE_2(_a, _b, _num, _den) {	\
    (_a).x = ((_b).x * (_num)) / (_den);		\
    (_a).y = ((_b).y * (_num)) / (_den);		\
    (_a).z = ((_b).z * (_num)) / (_den);		\
  }

#define INT32_VECT3_SDIV(_a, _b, _s) {			\
    (_a).x = (_b).x / (_s);				\
    (_a).y = (_b).y / (_s);				\
    (_a).z = (_b).z / (_s);				\
  }


#define INT32_VECT3_NORM(n, v) {					\
    int32_t n2 = v.x*v.x + v.y*v.y + v.z*v.z;				\
    INT32_SQRT(n, n2);							\
  }


/*
 * 3x3 Matrices
 */

#define INT32_MAT33_VECT3_MULT(_o, _m, _v, _f) {			\
    (_o).x = ((_m)[0]*(_v).x + (_m)[1]*(_v).y + (_m)[2]*(_v).z)>>(_f);	\
    (_o).y = ((_m)[3]*(_v).x + (_m)[4]*(_v).y + (_m)[5]*(_v).z)>>(_f);	\
    (_o).z = ((_m)[6]*(_v).x + (_m)[7]*(_v).y + (_m)[8]*(_v).z)>>(_f);	\
  }

/*
 * Rotation matrices
 */
#define INT32_RMAT_VMULT(_vo, _m, _vi) {				\
    (_vo).x = ( (_m).m[0]*(_vi).x + (_m).m[1]*(_vi).y + (_m).m[2]*(_vi).z)>>INT32_TRIG_FRAC; \
    (_vo).y = ( (_m).m[3]*(_vi).x + (_m).m[4]*(_vi).y + (_m).m[5]*(_vi).z)>>INT32_TRIG_FRAC; \
    (_vo).z = ( (_m).m[6]*(_vi).x + (_m).m[7]*(_vi).y + (_m).m[8]*(_vi).z)>>INT32_TRIG_FRAC; \
  }

/* 
 * http://www.mathworks.com/access/helpdesk_r13/help/toolbox/aeroblks/quaternionstodirectioncosinematrix.html 
 */
#define INT32_RMAT_OF_QUAT(_rm, _q) {					\
    const int32_t qx2  = INT_MULT_RSHIFT((_q).qx,(_q).qx, INT32_QUAT_FRAC); \
    const int32_t qy2  = INT_MULT_RSHIFT((_q).qy,(_q).qy, INT32_QUAT_FRAC); \
    const int32_t qz2  = INT_MULT_RSHIFT((_q).qz,(_q).qz, INT32_QUAT_FRAC); \
    const int32_t qiqx = INT_MULT_RSHIFT((_q).qi,(_q).qx, INT32_QUAT_FRAC); \
    const int32_t qiqy = INT_MULT_RSHIFT((_q).qi,(_q).qy, INT32_QUAT_FRAC); \
    const int32_t qiqz = INT_MULT_RSHIFT((_q).qi,(_q).qz, INT32_QUAT_FRAC); \
    const int32_t qxqy = INT_MULT_RSHIFT((_q).qx,(_q).qy, INT32_QUAT_FRAC); \
    const int32_t qxqz = INT_MULT_RSHIFT((_q).qx,(_q).qz, INT32_QUAT_FRAC); \
    const int32_t qyqz = INT_MULT_RSHIFT((_q).qy,(_q).qz, INT32_QUAT_FRAC); \
    const int32_t one = INT_BFP( 1, ITRIG_RES);				\
    const int32_t two = INT_BFP( 2, ITRIG_RES);				\
    /* dcm00 = 1.0 - 2.*(  qy2 +  qz2 ); */				\
    _rm.m[0] =  one - INT_MULT_RSHIFT( two, (qy2+qz2), ITRIG_RES+INT32_QUAT_FRAC-ITRIG_RES); \
    /* dcm01 =       2.*( qxqy + qiqz ); */				\
    _rm.m[1] = INT_MULT_RSHIFT( two, (qxqy+qiqz), ITRIG_RES+INT32_QUAT_FRAC-ITRIG_RES); \
    /* dcm02 =       2.*( qxqz - qiqy ); */				\
    _rm.m[2] = INT_MULT_RSHIFT( two, (qxqz-qiqy), ITRIG_RES+INT32_QUAT_FRAC-ITRIG_RES); \
    /* dcm10 = 2.*( qxqy - qiqz );       */				\
    _rm.m[3] = INT_MULT_RSHIFT( two, (qxqy-qiqz), ITRIG_RES+INT32_QUAT_FRAC-ITRIG_RES); \
    /* dcm11 = 1.0 - 2.*(qx2+qz2);       */				\
    _rm.m[4] = one - INT_MULT_RSHIFT( two, (qx2+qz2), ITRIG_RES+INT32_QUAT_FRAC-ITRIG_RES); \
    /* dcm12 =       2.*( qyqz + qiqx ); */				\
    _rm.m[5] = INT_MULT_RSHIFT( two, (qyqz+qiqx), ITRIG_RES+INT32_QUAT_FRAC-ITRIG_RES); \
    /* dcm20 =       2.*( qxqz + qiqy ); */				\
    _rm.m[6] = INT_MULT_RSHIFT( two, (qxqz+qiqy), ITRIG_RES+INT32_QUAT_FRAC-ITRIG_RES); \
    /* dcm21 =       2.*( qyqz - qiqx ); */				\
    _rm.m[7] = INT_MULT_RSHIFT( two, (qyqz-qiqx), ITRIG_RES+INT32_QUAT_FRAC-ITRIG_RES); \
    /* dcm22 = 1.0 - 2.*(  qx2 +  qy2 ); */				\
    _rm.m[8] = one - INT_MULT_RSHIFT( two, (qx2+qy2), ITRIG_RES+INT32_QUAT_FRAC-ITRIG_RES); \
  }

/*
 * http://www.mathworks.com/access/helpdesk_r13/help/toolbox/aeroblks/euleranglestodirectioncosinematrix.html
 */
#define INT32_RMAT_OF_EULERS(_rm, _e) {					\
    int32_t sphi;							\
    BOOZ_ISIN(sphi, (_e).phi);						\
    int32_t cphi;							\
    BOOZ_ICOS(cphi, (_e).phi);						\
    int32_t stheta;							\
    BOOZ_ISIN(stheta, (_e).theta);					\
    int32_t ctheta;							\
    BOOZ_ICOS(ctheta, (_e).theta);					\
    int32_t spsi;							\
    BOOZ_ISIN(spsi, (_e).psi);						\
    int32_t cpsi;							\
    BOOZ_ICOS(cpsi, (_e).psi);						\
									\
    int32_t ctheta_cpsi = INT_MULT_RSHIFT(ctheta, cpsi,   INT32_TRIG_FRAC); \
    int32_t ctheta_spsi = INT_MULT_RSHIFT(ctheta, spsi,   INT32_TRIG_FRAC); \
    int32_t cphi_spsi   = INT_MULT_RSHIFT(cphi,   spsi,   INT32_TRIG_FRAC); \
    int32_t cphi_cpsi   = INT_MULT_RSHIFT(cphi,   cpsi,   INT32_TRIG_FRAC); \
    int32_t cphi_ctheta = INT_MULT_RSHIFT(cphi,   ctheta, INT32_TRIG_FRAC); \
    int32_t cphi_stheta = INT_MULT_RSHIFT(cphi,   stheta, INT32_TRIG_FRAC); \
    int32_t sphi_ctheta = INT_MULT_RSHIFT(sphi,   ctheta, INT32_TRIG_FRAC); \
    int32_t sphi_stheta = INT_MULT_RSHIFT(sphi,   stheta, INT32_TRIG_FRAC); \
    int32_t sphi_spsi   = INT_MULT_RSHIFT(sphi,   spsi,   INT32_TRIG_FRAC); \
    int32_t sphi_cpsi   = INT_MULT_RSHIFT(sphi,   cpsi,   INT32_TRIG_FRAC); \
    									\
    int32_t sphi_stheta_cpsi = INT_MULT_RSHIFT(sphi_stheta, cpsi, INT32_TRIG_FRAC); \
    int32_t sphi_stheta_spsi = INT_MULT_RSHIFT(sphi_stheta, spsi, INT32_TRIG_FRAC); \
    int32_t cphi_stheta_cpsi = INT_MULT_RSHIFT(cphi_stheta, cpsi, INT32_TRIG_FRAC); \
    int32_t cphi_stheta_spsi = INT_MULT_RSHIFT(cphi_stheta, spsi, INT32_TRIG_FRAC); \
    									\
    _rm.m[0] = ctheta_cpsi;						\
    _rm.m[1] = ctheta_spsi;						\
    _rm.m[2] = -stheta;							\
    _rm.m[3] = sphi_stheta_cpsi - cphi_spsi;				\
    _rm.m[4] = sphi_stheta_spsi + cphi_cpsi;				\
    _rm.m[5] = sphi_ctheta;						\
    _rm.m[6] = cphi_stheta_cpsi + sphi_spsi;				\
    _rm.m[7] = cphi_stheta_spsi - sphi_cpsi;				\
    _rm.m[8] = cphi_ctheta;						\
    									\
  }


/*
 * Quaternions
 */

#define INT32_QUAT_ZERO(_q) {						\
    _q.qi = QUAT_BFP(1);						\
    _q.qx = 0;								\
    _q.qy = 0;								\
    _q.qz = 0;								\
  }

#define INT32_QUAT_NORM(n, q) {			                        \
    int32_t n2 = q.qi*q.qi + q.qx*q.qx + q.qy*q.qy + q.qz*q.qz;		\
    INT32_SQRT(n, n2);							\
  }

#define INT32_QUAT_NORMALISE(q) {		                        \
    int32_t n;								\
    INT32_QUAT_NORM(n, q);						\
    q.qi = q.qi * QUAT_BFP(1) / n;			\
    q.qx = q.qx * QUAT_BFP(1) / n;					\
    q.qy = q.qy * QUAT_BFP(1) / n;					\
    q.qz = q.qz * QUAT_BFP(1) / n;					\
  }


#define INT32_QUAT_MULT(c, a, b) {					\
    c.qi = (a.qi*b.qi - a.qx*b.qx - a.qy*b.qy - a.qz*b.qz)>>INT32_QUAT_FRAC; \
    c.qx = (a.qi*b.qx + a.qx*b.qi + a.qy*b.qz - a.qz*b.qy)>>INT32_QUAT_FRAC; \
    c.qy = (a.qi*b.qy - a.qx*b.qz + a.qy*b.qi + a.qz*b.qx)>>INT32_QUAT_FRAC; \
    c.qz = (a.qi*b.qz + a.qx*b.qy - a.qy*b.qx + a.qz*b.qi)>>INT32_QUAT_FRAC; \
  }

#define INT32_QUAT_INVERT(_qo, _qi) {					\
    (_qo).qi =  (_qi).qi;						\
    (_qo).qx = -(_qi).qx;						\
    (_qo).qy = -(_qi).qy;						\
    (_qo).qz = -(_qi).qz;						\
  }

#define INT32_QUAT_VMULT(v_out, q, v_in) {				\
    const int32_t qi2  = (q.qi*q.qi)>>INT32_QUAT_FRAC;			\
    const int32_t qx2  = (q.qx*q.qx)>>INT32_QUAT_FRAC;			\
    const int32_t qy2  = (q.qy*q.qy)>>INT32_QUAT_FRAC;			\
    const int32_t qz2  = (q.qz*q.qz)>>INT32_QUAT_FRAC;			\
    const int32_t qiqx = (q.qi*q.qx)>>INT32_QUAT_FRAC;			\
    const int32_t qiqy = (q.qi*q.qy)>>INT32_QUAT_FRAC;			\
    const int32_t qiqz = (q.qi*q.qz)>>INT32_QUAT_FRAC;			\
    const int32_t qxqy = (q.qx*q.qy)>>INT32_QUAT_FRAC;			\
    const int32_t qxqz = (q.qx*q.qz)>>INT32_QUAT_FRAC;			\
    const int32_t qyqz = (q.qy*q.qz)>>INT32_QUAT_FRAC;			\
    const int32_t m00 = qi2 + qx2 - qy2 - qz2;				\
    const int32_t m01 = 2 * (qxqy + qiqz );				\
    const int32_t m02 = 2 * (qxqz - qiqy );				\
    const int32_t m10 = 2 * (qxqy - qiqz );				\
    const int32_t m11 = qi2 - qx2 + qy2 - qz2;				\
    const int32_t m12 = 2 * (qyqz + qiqx );				\
    const int32_t m20 = 2 * (qxqz + qiqy );				\
    const int32_t m21 = 2 * (qyqz - qiqx );				\
    const int32_t m22 = qi2 - qx2 - qy2 + qz2;				\
    v_out.x = (m00 * v_in.x + m01 * v_in.y + m02 * v_in.z)>>INT32_QUAT_FRAC; \
    v_out.y = (m10 * v_in.x + m11 * v_in.y + m12 * v_in.z)>>INT32_QUAT_FRAC; \
    v_out.z = (m20 * v_in.x + m21 * v_in.y + m22 * v_in.z)>>INT32_QUAT_FRAC; \
  }




/*
 * http://www.mathworks.com/access/helpdesk_r13/help/toolbox/aeroblks/euleranglestoquaternions.html
 */
#define INT32_QUAT_OF_EULERS(_q, _e) {					\
    const int32_t phi2   = (_e).phi   / 2;				\
    const int32_t theta2 = (_e).theta / 2;				\
    const int32_t psi2   = (_e).psi   / 2;				\
    									\
    int32_t s_phi2;							\
    BOOZ_ISIN(s_phi2, phi2);						\
    int32_t c_phi2;							\
    BOOZ_ICOS(c_phi2, phi2);						\
    int32_t s_theta2;							\
    BOOZ_ISIN(s_theta2, theta2);					\
    int32_t c_theta2;							\
    BOOZ_ICOS(c_theta2, theta2);					\
    int32_t s_psi2;							\
    BOOZ_ISIN(s_psi2, psi2);						\
    int32_t c_psi2;							\
    BOOZ_ICOS(c_psi2, psi2);						\
									\
    int32_t c_th_c_ps = INT_MULT_RSHIFT(c_theta2, c_psi2, ITRIG_RES);	\
    int32_t c_th_s_ps = INT_MULT_RSHIFT(c_theta2, s_psi2, ITRIG_RES);	\
    int32_t s_th_s_ps = INT_MULT_RSHIFT(s_theta2, s_psi2, ITRIG_RES);	\
    int32_t s_th_c_ps = INT_MULT_RSHIFT(s_theta2, c_psi2, ITRIG_RES);	\
    									\
    (_q).qi = INT_MULT_RSHIFT( c_phi2, c_th_c_ps, ITRIG_RES + ITRIG_RES - INT32_QUAT_FRAC) + \
              INT_MULT_RSHIFT( s_phi2, s_th_s_ps, ITRIG_RES + ITRIG_RES - INT32_QUAT_FRAC);  \
    (_q).qx = INT_MULT_RSHIFT(-c_phi2, s_th_s_ps, ITRIG_RES + ITRIG_RES - INT32_QUAT_FRAC) + \
              INT_MULT_RSHIFT( s_phi2, c_th_c_ps, ITRIG_RES + ITRIG_RES - INT32_QUAT_FRAC);  \
    (_q).qy = INT_MULT_RSHIFT( c_phi2, s_th_c_ps, ITRIG_RES + ITRIG_RES - INT32_QUAT_FRAC) + \
              INT_MULT_RSHIFT( s_phi2, c_th_s_ps, ITRIG_RES + ITRIG_RES - INT32_QUAT_FRAC);  \
    (_q).qz = INT_MULT_RSHIFT( c_phi2, c_th_s_ps, ITRIG_RES + ITRIG_RES - INT32_QUAT_FRAC) + \
              INT_MULT_RSHIFT(-s_phi2, s_th_c_ps, ITRIG_RES + ITRIG_RES - INT32_QUAT_FRAC);  \
  }



#define INT32_EULERS_OF_RMAT(_e, _rm) {	\
    					\
  }

#define INT32_QUAT_AND_RMAT_OF_EULERS(_q, _rm, _e) {	\
							\
  }




/*
 * Euler angles
 */

#define INT_EULERS_ZERO(_e) {			\
    (_e).phi = 0;				\
    (_e).theta = 0;				\
    (_e).psi = 0;				\
  }


/*
 * Rotational speeds
 */

#define INT_RATES_ZERO(_e) {		\
    (_e).p = 0;				\
    (_e).q = 0;				\
    (_e).r = 0;				\
  }



/*
 *
 */
#define INT32_SQRT_MAX_ITER 40
#define INT32_SQRT(_out,_in) {			                        \
    if (_in == 0)							\
      _out = 0;								\
    else {								\
      uint32_t s1, s2;							\
      uint8_t iter = 0;							\
      s2 = _in;								\
      do {								\
	s1 = s2;							\
	s2 = _in / s1;							\
	s2 += s1;							\
	s2 /= 2;							\
	iter++;								\
      }									\
      while( ( (s1-s2) > 1) && (iter < INT32_SQRT_MAX_ITER));		\
      _out = s2;							\
    }									\
  }



#endif /* PPRZ_ALGEBRA_INT_H */
