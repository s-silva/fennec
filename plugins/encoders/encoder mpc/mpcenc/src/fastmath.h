/*
 * Musepack audio compression
 * Copyright (C) 1999-2004 Buschmann/Klemm/Piecha/Wolf
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

typedef union mpc_floatint
{
    float   f;
    Int32_t n;
} mpc_floatint;

static __inline Int32_t mpc_nearbyintf(float fVal)
{
    mpc_floatint tmp;
    tmp.f = fVal  + 0x00FF8000;
    tmp.n = tmp.n - 0x4B7F8000;
    return tmp.n;
}
#define mpc_lrintf mpc_nearbyintf
static __inline Int32_t mpc_round32(float fVal)
{
    mpc_floatint tmp;
    tmp.f = fVal  + 0x00FD8000;
    tmp.n = tmp.n - 0x4B7D8000;
    return tmp.n;
}

#ifdef FAST_MATH

static __inline float
my_atan2 ( float x, float y )
{
    float t, ret; int i; mpc_floatint mx, my;

    mx.f = x;
    my.f = y;
    if ( (mx.n & 0x7FFFFFFF) < (my.n & 0x7FFFFFFF) ) {
        i   = mpc_round32 (t = TABSTEP * (mx.f / my.f));
        ret = tabatan2 [1*TABSTEP+i][0] + tabatan2 [1*TABSTEP+i][1] * (t-i);
        if ( my.n < 0 )
           ret = (float)(ret - M_PI);
    }
    else if ( mx.n < 0 ) {
        i   = mpc_round32 (t = TABSTEP * (my.f / mx.f));
        ret = - M_PI/2 - tabatan2 [1*TABSTEP+i][0] + tabatan2 [1*TABSTEP+i][1] * (i-t);
    }
    else if ( mx.n > 0 ) {
        i   = mpc_round32 (t = TABSTEP * (my.f / mx.f));
        ret = + M_PI/2 - tabatan2 [1*TABSTEP+i][0] + tabatan2 [1*TABSTEP+i][1] * (i-t);
    }
    else {
        ret = 0.;
    }
    return ret;
}


static __inline float
my_cos ( float x )
{
    float t, ret; int i;
    i   = mpc_round32 (t = TABSTEP * x);
    ret = tabcos [13*TABSTEP+i][0] + tabcos [13*TABSTEP+i][1] * (t-i);
    return ret;
}


static __inline int
my_ifloor ( float x )
{
    mpc_floatint mx;
    mx.f = (float) (x + (0x0C00000L + 0.500000001));
    return mx.n - 1262485505;
}


static __inline float
my_sqrt ( float x )
{
    float  ret; int i, ex; mpc_floatint mx;
    mx.f = x;
    ex   = mx.n >> 23;                     // get the exponent
    mx.n = (mx.n & 0x7FFFFF) | 0x42800000; // delete the exponent
    i    = mpc_round32 (mx.f);             // Integer-part of the mantissa  (round ????????????)
    ret  = tabsqrt_m [i-TABSTEP][0] + tabsqrt_m [i-TABSTEP][1] * (mx.f-i); // calculate value
    ret *= tabsqrt_ex [ex];
    return ret;
}

#endif
