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

#include "mppenc.h"

#define CX0     -1.
#define CX1      0.5

#define SX1     -1.
#define SX2      (2./9/  1)
#define SX3      (2./9/  4)
#define SX4      (2./9/ 10)
#define SX5      (2./9/ 20)
#define SX6      (2./9/ 35)
#define SX7      (2./9/ 56)
#define SX8      (2./9/ 84)
#define SX9      (2./9/120)
#define SX10     (2./9/165)


#ifdef EXTRA_DECONV
# define DECONV \
    {  \
    tmp      = (CX0*aix[0] + CX1*aix[2]) * (1./(CX0*CX0+CX1*CX1)); \
    aix[ 0] -= CX0*tmp; \
    aix[ 2] -= CX1*tmp; \
    tmp      = (SX1*aix[3] + SX2*aix[5] + SX3*aix[7] + SX4*aix[9] + SX5*aix[11]) * (1./(SX1*SX1+SX2*SX2+SX3*SX3+SX4*SX4+SX5*SX5)); \
    aix[ 3] -= SX1*tmp; \
    aix[ 5] -= SX2*tmp; \
    aix[ 7] -= SX3*tmp; \
    aix[ 9] -= SX4*tmp; \
    aix[11] -= SX5*tmp; \
    }
#elif 0
# define DECONV \
    {  \
    float A[20]; \
    int   i; \
    memcpy (A, aix, 20*sizeof(aix)); \
    tmp      = (CX0*aix[0] + CX1*aix[2]) * (1./(CX0*CX0+CX1*CX1)); \
    aix[ 0] -= CX0*tmp; \
    aix[ 2] -= CX1*tmp; \
    tmp      = (SX1*aix[3] + SX2*aix[5] + SX3*aix[7] + SX4*aix[9] + SX5*aix[11]) * (1./(SX1*SX1+SX2*SX2+SX3*SX3+SX4*SX4+SX5*SX5)); \
    aix[ 3] -= SX1*tmp; \
    aix[ 5] -= SX2*tmp; \
    aix[ 7] -= SX3*tmp; \
    aix[ 9] -= SX4*tmp; \
    aix[11] -= SX5*tmp; \
    for ( i=0; i<10; i++) \
        printf ("%u%9.0f%7.0f%9.0f%7.0f\n",i, A[i+i], A[i+i+1], aix[i+i], aix[i+i+1] ); \
    }
#else
# define DECONV
#endif


/* V A R I A B L E S */
static int    ip [4096];   // bitinverse for maximum 2048 FFT
static float  w  [4096];   // butterfly-coefficient for maximum 2048 FFT
static float  a  [4096];   // holds real input for FFT
static float  Hann_256  [ 256];
static float  Hann_1024 [1024];
static float  Hann_1600 [1600];


//////////////////////////////
//
// BesselI0 -- Regular Modified Cylindrical Bessel Function (Bessel I).
//

static double
Bessel_I_0 ( double x )
{
    double  denominator;
    double  numerator;
    double  z;

    if (x == 0.)
        return 1.;

    z = x * x;
    numerator = z* (z* (z* (z* (z* (z* (z* (z* (z* (z* (z* (z* (z* (z*
                   0.210580722890567e-22  + 0.380715242345326e-19 ) +
                   0.479440257548300e-16) + 0.435125971262668e-13 ) +
                   0.300931127112960e-10) + 0.160224679395361e-07 ) +
                   0.654858370096785e-05) + 0.202591084143397e-02 ) +
                   0.463076284721000e+00) + 0.754337328948189e+02 ) +
                   0.830792541809429e+04) + 0.571661130563785e+06 ) +
                   0.216415572361227e+08) + 0.356644482244025e+09 ) +
                   0.144048298227235e+10;

    denominator = z* (z* (z - 0.307646912682801e+04) + 0.347626332405882e+07) - 0.144048298227235e+10;

    return - numerator / denominator;
}

static double
residual ( double x )
{
    return sqrt ( 1. - x*x );
}

//////////////////////////////
//
// KBDWindow -- Kaiser Bessel Derived Window
//      fills the input window array with size samples of the
//      KBD window with the given tuning parameter alpha.
//


static void
KBDWindow ( float* window, unsigned int size, float alpha )
{
    double  sumvalue = 0.;
    double  scale;
    int     i;

    scale = 0.25 / sqrt (size);
    for ( i = 0; i < (int)size/2; i++ )
        window [i] = sumvalue += Bessel_I_0 ( M_PI * alpha * residual (4.*i/size - 1.) );

    // need to add one more value to the nomalization factor at size/2:
    sumvalue += Bessel_I_0 ( M_PI * alpha * residual (4.*(size/2)/size-1.) );

    // normalize the window and fill in the righthand side of the window:
    for ( i = 0; i < (int)size/2; i++ )
        window [size-1-i] = window [i] = /*sqrt*/ ( window [i] / sumvalue ) * scale;
}

static void
CosWindow ( float* window, unsigned int size )
{
    double  x;
    double  scale;
    int     i;

    scale = 0.25 / sqrt (size);
    for ( i = 0; i < (int)size/2; i++ ) {
        x = cos ( (i+0.5) * (M_PI / size) );
        window [size/2-1-i] = window [size/2+i] = scale * x * x;
    }
}

static void
Window ( float* window, unsigned int size, float alpha )
{
    if ( alpha < 0. )
        CosWindow ( window, size ) ;
    else
        KBDWindow ( window, size, alpha );
}


/* F U N C T I O N S */
// generates FFT lookup-tables
void
Init_FFT ( void )
{
    int     n;
    double  x;
    double  scale;

    // normalized hann functions
    Window ( Hann_256 ,  256, KBD1 );
    Window ( Hann_1024, 1024, KBD2 );
    scale = 0.25 / sqrt (2048.);
    for ( n = 0; n < 800; n++ )
        x = cos ((n+0.5) * (M_PI/1600)), Hann_1600 [799-n] = Hann_1600 [800+n] = (float)(x * x * scale);

    Generate_FFT_Tables ( 2048, ip, w );
}

// input : Signal *x
// output: energy spectrum *erg
void
PowSpec256 ( const float* x, float* erg )
{
    const float*  win = Hann_256;
    float*        aix = a;
    int           i;

    ENTER(40);
    // windowing
    i = 256;
    while (i--)
        *aix++ = *x++ * *win++;

    // perform FFT
    rdft ( 256, a, ip, w );

    // calculate power
    aix = a;    // reset pointer
    i   = 128;
    while (i--) {
        *erg++ = aix[0]*aix[0] + aix[1]*aix[1];
        aix += 2;
    }
    LEAVE(40);
}

// input : Signal *x
// output: energy spectrum *erg
void
PowSpec1024 ( const float* x, float* erg )
{
    const float*  win = Hann_1024;
    float*        aix = a;
    int           i;

    ENTER(41);
    i = 1024;                   // windowing
    while (i--)
        *aix++ = *x++ * *win++;

//    for (i=0; i<1024; i++)
//        a[i] = Hann_1024[i] * ((i==0 ? 0 : i-512) + 1000);

    rdft ( 1024, a, ip, w );    // perform FFT

    aix = a;                    // calculate power
    i   = 512;


    DECONV;
//    for (i = 0; i <= 512; i++ )
//        printf ("%3u %12.6f %12.6f\n", i, a[i+i], a[i+i+1]);
//    exit(1);
    while (i--) {
        *erg++ = aix[0]*aix[0] + aix[1]*aix[1];
        aix += 2;
    }
    LEAVE(41);
}

// input : Signal *x
// output: energy spectrum *erg
void
PowSpec2048 ( const float* x, float* erg )
{
    const float*  win = Hann_1600;
    float*        aix = a;
    int           i;

    ENTER(42);
    // windowing (only 1600 samples available -> centered in 2048!)
    memset ( a     , 0, 224*sizeof(*a) );
    aix = a + 224;
    i   = 1600;
    while (i--)
        *aix++ = *x++ * *win++;
    memset ( a+1824, 0, 224*sizeof(*a) );

    rdft ( 2048, a, ip, w );    // perform FFT

    aix = a;                    // calculate power
    i   = 1024;
    while (i--) {
        *erg++ = aix[0]*aix[0] + aix[1]*aix[1];
        aix += 2;
    }
    LEAVE(42);
}

#include "fastmath.h"

// input : Signal *x
// output: energy spectrum *erg and phase spectrum *phs
void
PolarSpec1024 ( const float* x, float* erg, float* phs )
{
    const float*  win = Hann_1024;
    float*        aix = a;
    int           i;

    ENTER(43);
    i = 1024;                   // windowing
    while (i--)
        *aix++ = *x++ * *win++;

    rdft ( 1024, a, ip, w );    // perform FFT

    // calculate power and phase
    aix = a;    // reset pointer
    i   = 512;
    while (i--) {
        *erg++ = aix[0]*aix[0] + aix[1]*aix[1];
        *phs++ = ATAN2F (aix[1], aix[0]);
        aix += 2;
    }
    LEAVE(43);
}

// input : logarithmized energy spectrum *cep
// output: Cepstrum *cep (in-place)
void
Cepstrum2048 ( float* cep, const int MaxLine )
{
    float*  aix = cep;
    float*  bix = cep + 2048;
    int     i;

    ENTER(44);
    // generate real, even spectrum (symmetric around 1024, cep[2048-i] = cep[i])
    for ( i = 0; i < 1024; i++ )
        *bix-- = *aix++;

    // perform IFFT
    rdft ( 2048, cep, ip, w );

    // only real part as outcome (all even indexes of cep[])
    aix = cep;
    bix = cep;
    i   = MaxLine + 1;
    while (i--) {
        *aix = *bix * (float) (0.9888 / 2048.);
//      *aix = *bix * 0.0004828125f;
        aix ++;
        bix += 2;
    }
    LEAVE(44);
}
