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

/* overflow of subband-samples */

#include <memory.h>
#include <time.h>
#include <errno.h>
#include "mppenc.h"

/* G L O B A L  V A R I A B L E S */
float         SNR_comp_L [32];
float         SNR_comp_R [32];             // SNR-compensation after SCF-combination and ANS-gain
float         Power_L    [32] [3];
float         Power_R    [32] [3];
float         PNS = 0.;
int           Max_Band;                    // maximum bandwidth

/* MS-Coding */
unsigned int  MS_Channelmode;              // global flag for enhanced functionality
float         SampleFreq      =  0.;
float         Bandwidth       =  0.;
int           PredictionBands =  0;
int           CombPenalities  = -1;
float         KBD1            =  2;
float         KBD2            = -1.;
int           DisplayUpdateTime = 1;
int           APE_Version     = 2000;
int           LowDelay        = 0;
Bool_t        IsEndBeep       = 0;

#define MODE_OVERWRITE          0
#define MODE_NEVER_OVERWRITE    1
#define MODE_ASK_FOR_OVERWRITE  2

/* other general global variables */
unsigned int  DelInput        = 0;      // deleting the input file after encoding
unsigned int  WriteMode       = MODE_ASK_FOR_OVERWRITE;      // overwriting a possibly existing MPC file
int           MainQual;                 // Profiles
unsigned int  verbose         = 0;      // more information during output
unsigned int  NoUnicode       = 1;      // console is unicode or not (tag translation)
UintMax_t     SamplesInWAVE   = 0;      // number of samples per channel in the WAV file
unsigned int  Overflows       = 0;      // number of internal (filterbank) clippings
float         MaxOverFlow     = 0.f;    // maximum overflow
float         ScalingFactorl  = 1.f;    // Scaling the input signal
float         ScalingFactorr  = 1.f;    // Scaling the input signal
float         FadeShape       = 1.f;    // Shape of the fade
float         FadeInTime      = 0.f;    // Duration of FadeIn in secs
float         FadeOutTime     = 0.f;    // Duration of FadeOut in secs
float         SkipTime        = 0.f;    // Skip the beginning of the file (sec)
double        Duration        = 1.e+99; // Maximum encoded audio length
Bool_t        FrontendPresent = 0;      // Flag for frontend-detection
Bool_t        XLevel          = 1;      // Encode extreme levels with relative SCFs
const char    About []        = "MPC Encoder  " MPPENC_VERSION "  " MPPENC_BUILD "   (C) 1999-2006 Buschmann/Klemm/Piecha/MDT";


#if defined _WIN32  ||  defined __TURBOC__
# include <conio.h>
#else

# ifdef USE_TERMIOS
#  include <termios.h>

static struct termios  stored_settings;

static void
echo_on ( void )
{
    tcsetattr ( 0, TCSANOW, &stored_settings );
}

static void
echo_off ( void )
{
    struct termios  new_settings;

    tcgetattr ( 0, &stored_settings );
    new_settings = stored_settings;

    new_settings.c_lflag     &= ~ECHO;
    new_settings.c_lflag     &= ~ICANON;        /* Disable canonical mode, and set buffer size to 1 byte */
    new_settings.c_cc[VTIME]  = 0;
    new_settings.c_cc[VMIN]   = 1;

    tcsetattr ( 0, TCSANOW, &new_settings );
}

# else
#  define echo_off()  (void)0
#  define echo_on()   (void)0
# endif

static int
getch ( void )
{
    unsigned char  buff [1];
    int            ret;

    echo_off ();
    ret = READ1 ( STDIN, buff );
    echo_on ();
    return ret == 1  ?  buff[0]  :  -1;
}

#endif


static int
waitkey ( void )
{
    int  c;

    fflush (stdout);
    while ( (c = getch() ) <= ' ' )
        ;
    return c;
}

#include "fastmath.h"


float  bump_exp   = 1.f;
float  bump_start = 0.040790618517f;


static void
setbump ( double e )
{
    bump_exp   = e;
    bump_start = 1 - sqrt (1 - 1 / (1 - log(1.e-5) / e));
}


static double
bump ( double x )
{
    x = bump_start + x * (1. - bump_start);
    if ( x <= 0.) return 0.;
    if ( x >= 1.) return 1.;
    x *= (2. - x);
    x  = (x - 1.) / x;
    return exp (x * bump_exp);
}


static void
Fading_In ( PCMDataTyp* data, unsigned int N, const float fs )
{
    float  inv_fs = 1.f / fs;
    float  fadein_pos;
    float  scale;
    int    n;
    int    idx;

    ENTER(2);
    for ( n = 0; n < BLOCK; n++, N++ ) {
        idx           = n + CENTER;
        fadein_pos    = N * inv_fs;
        scale         = fadein_pos / FadeInTime;
        scale         = bump (scale);
        data->L[idx] *= scale;
        data->R[idx] *= scale;
        data->M[idx] *= scale;
        data->S[idx] *= scale;
    }
    LEAVE(2);
}


static void
Fading_Out ( PCMDataTyp* data, unsigned int N, const float fs )
{
    float  inv_fs = 1.f / fs;
    float  fadeout_pos;
    float  scale;
    int    n;
    int    idx;

    ENTER(3);
    for ( n = 0; n < BLOCK; n++, N++ ) {
        idx           = n + CENTER;
        fadeout_pos   = UintMAX_FP(SamplesInWAVE - N) * inv_fs;
        scale         = fadeout_pos / FadeOutTime;
        scale         = bump (scale);
        data->L[idx] *= scale;
        data->R[idx] *= scale;
        data->M[idx] *= scale;
        data->S[idx] *= scale;
    }
    LEAVE(3);
}


static const unsigned char  Penalty [256] = {
    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
      0,  2,  5,  9, 15, 23, 36, 54, 79,116,169,246,255,255,255,255,
    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
};

#define P(new,old)  Penalty [128 + (old) - (new)]

static void
SCF_Extraktion ( const int MaxBand, SubbandFloatTyp* x )
{
    int    Band;
    int    n;
    int    d01;
    int    d12;
    int    d02;
    int    warnL;
    int    warnR;
    int*   scfL;
    int*   scfR;
    int    comp_L [3];
    int    comp_R [3];
    float  tmp_L  [3];
    float  tmp_R  [3];
    float  facL;
    float  facR;
    float  L;
    float  R;
    float  SL;
    float  SR;

    ENTER(4);

    for ( Band = 0; Band <= MaxBand; Band++ ) {         // Suche nach Maxima
        L  = FABS (x[Band].L[ 0]);
        R  = FABS (x[Band].R[ 0]);
        SL = x[Band].L[ 0] * x[Band].L[ 0];
        SR = x[Band].R[ 0] * x[Band].R[ 0];
        for ( n = 1; n < 12; n++ ) {
            if (L < FABS (x[Band].L[n])) L = FABS (x[Band].L[n]);
            if (R < FABS (x[Band].R[n])) R = FABS (x[Band].R[n]);
            SL += x[Band].L[n] * x[Band].L[n];
            SR += x[Band].R[n] * x[Band].R[n];
        }
        Power_L [Band][0] = SL;
        Power_R [Band][0] = SR;
        tmp_L [0] = L;
        tmp_R [0] = R;

        L  = FABS (x[Band].L[12]);
        R  = FABS (x[Band].R[12]);
        SL = x[Band].L[12] * x[Band].L[12];
        SR = x[Band].R[12] * x[Band].R[12];
        for ( n = 13; n < 24; n++ ) {
            if (L < FABS (x[Band].L[n])) L = FABS (x[Band].L[n]);
            if (R < FABS (x[Band].R[n])) R = FABS (x[Band].R[n]);
            SL += x[Band].L[n] * x[Band].L[n];
            SR += x[Band].R[n] * x[Band].R[n];
        }
        Power_L [Band][1] = SL;
        Power_R [Band][1] = SR;
        tmp_L [1] = L;
        tmp_R [1] = R;

        L  = FABS (x[Band].L[24]);
        R  = FABS (x[Band].R[24]);
        SL = x[Band].L[24] * x[Band].L[24];
        SR = x[Band].R[24] * x[Band].R[24];
        for ( n = 25; n < 36; n++ ) {
            if (L < FABS (x[Band].L[n])) L = FABS (x[Band].L[n]);
            if (R < FABS (x[Band].R[n])) R = FABS (x[Band].R[n]);
            SL += x[Band].L[n] * x[Band].L[n];
            SR += x[Band].R[n] * x[Band].R[n];
        }
        Power_L [Band][2] = SL;
        Power_R [Band][2] = SR;
        tmp_L [2] = L;
        tmp_R [2] = R;

        // calculation of the scalefactor-indexes
        // -12.6f*log10(x)+57.8945021823f = -10*log10(x/32767)*1.26+1
        // normalize maximum of +/- 32767 to prevent quantizer overflow
        // It can stand a maximum of +/- 32768 ...

        // Where is scf{R,L} [0...2] initialized ???
        scfL = SCF_Index_L [Band];
        scfR = SCF_Index_R [Band];
        if (tmp_L [0] > 0.) scfL [0] = IFLOORF (-12.6f * LOG10 (tmp_L [0]) + 57.8945021823f );
        if (tmp_L [1] > 0.) scfL [1] = IFLOORF (-12.6f * LOG10 (tmp_L [1]) + 57.8945021823f );
        if (tmp_L [2] > 0.) scfL [2] = IFLOORF (-12.6f * LOG10 (tmp_L [2]) + 57.8945021823f );
        if (tmp_R [0] > 0.) scfR [0] = IFLOORF (-12.6f * LOG10 (tmp_R [0]) + 57.8945021823f );
        if (tmp_R [1] > 0.) scfR [1] = IFLOORF (-12.6f * LOG10 (tmp_R [1]) + 57.8945021823f );
        if (tmp_R [2] > 0.) scfR [2] = IFLOORF (-12.6f * LOG10 (tmp_R [2]) + 57.8945021823f );

        // restriction to SCF_Index = 0...63, make note of the internal overflow
        warnL = warnR = 0;
        if (scfL[0] & ~63) { if (scfL[0] < 0) { if (XLevel==0) scfL[0] = 0, warnL = 1; } else scfL[0] = 63; }
        if (scfL[1] & ~63) { if (scfL[1] < 0) { if (XLevel==0) scfL[1] = 0, warnL = 1; } else scfL[1] = 63; }
        if (scfL[2] & ~63) { if (scfL[2] < 0) { if (XLevel==0) scfL[2] = 0, warnL = 1; } else scfL[2] = 63; }
        if (scfR[0] & ~63) { if (scfR[0] < 0) { if (XLevel==0) scfR[0] = 0, warnR = 1; } else scfR[0] = 63; }
        if (scfR[1] & ~63) { if (scfR[1] < 0) { if (XLevel==0) scfR[1] = 0, warnR = 1; } else scfR[1] = 63; }
        if (scfR[2] & ~63) { if (scfR[2] < 0) { if (XLevel==0) scfR[2] = 0, warnR = 1; } else scfR[2] = 63; }

        // save old values for compensation calculation
        comp_L[0] = scfL[0]; comp_L[1] = scfL[1]; comp_L[2] = scfL[2];
        comp_R[0] = scfR[0]; comp_R[1] = scfR[1]; comp_R[2] = scfR[2];

        // determination and replacement of scalefactors of minor differences with the smaller one???
        // a smaller one is quantized more roughly, i.e. the noise gets amplified???

        if ( CombPenalities >= 0 ) {
            if      ( P(scfL[0],scfL[1]) + P(scfL[0],scfL[2]) <= CombPenalities ) scfL[2] = scfL[1] = scfL[0];
            else if ( P(scfL[1],scfL[0]) + P(scfL[1],scfL[2]) <= CombPenalities ) scfL[0] = scfL[2] = scfL[1];
            else if ( P(scfL[2],scfL[0]) + P(scfL[2],scfL[1]) <= CombPenalities ) scfL[0] = scfL[1] = scfL[2];
            else if ( P(scfL[0],scfL[1])                      <= CombPenalities ) scfL[1] = scfL[0];
            else if ( P(scfL[1],scfL[0])                      <= CombPenalities ) scfL[0] = scfL[1];
            else if ( P(scfL[1],scfL[2])                      <= CombPenalities ) scfL[2] = scfL[1];
            else if ( P(scfL[2],scfL[1])                      <= CombPenalities ) scfL[1] = scfL[2];

            if      ( P(scfR[0],scfR[1]) + P(scfR[0],scfR[2]) <= CombPenalities ) scfR[2] = scfR[1] = scfR[0];
            else if ( P(scfR[1],scfR[0]) + P(scfR[1],scfR[2]) <= CombPenalities ) scfR[0] = scfR[2] = scfR[1];
            else if ( P(scfR[2],scfR[0]) + P(scfR[2],scfR[1]) <= CombPenalities ) scfR[0] = scfR[1] = scfR[2];
            else if ( P(scfR[0],scfR[1])                      <= CombPenalities ) scfR[1] = scfR[0];
            else if ( P(scfR[1],scfR[0])                      <= CombPenalities ) scfR[0] = scfR[1];
            else if ( P(scfR[1],scfR[2])                      <= CombPenalities ) scfR[2] = scfR[1];
            else if ( P(scfR[2],scfR[1])                      <= CombPenalities ) scfR[1] = scfR[2];
        }
        else {

            d12  = scfL [2] - scfL [1];
            d01  = scfL [1] - scfL [0];
            d02  = scfL [2] - scfL [0];

            if      ( 0 < d12  &&  d12 < 5 ) scfL [2] = scfL [1];
            else if (-3 < d12  &&  d12 < 0 ) scfL [1] = scfL [2];
            else if ( 0 < d01  &&  d01 < 5 ) scfL [1] = scfL [0];
            else if (-3 < d01  &&  d01 < 0 ) scfL [0] = scfL [1];
            else if ( 0 < d02  &&  d02 < 4 ) scfL [2] = scfL [0];
            else if (-2 < d02  &&  d02 < 0 ) scfL [0] = scfL [2];

            d12  = scfR [2] - scfR [1];
            d01  = scfR [1] - scfR [0];
            d02  = scfR [2] - scfR [0];

            if      ( 0 < d12  &&  d12 < 5 ) scfR [2] = scfR [1];
            else if (-3 < d12  &&  d12 < 0 ) scfR [1] = scfR [2];
            else if ( 0 < d01  &&  d01 < 5 ) scfR [1] = scfR [0];
            else if (-3 < d01  &&  d01 < 0 ) scfR [0] = scfR [1];
            else if ( 0 < d02  &&  d02 < 4 ) scfR [2] = scfR [0];
            else if (-2 < d02  &&  d02 < 0 ) scfR [0] = scfR [2];
        }

        // calculate SNR-compensation
        tmp_L [0]         = invSCF [comp_L[0] - scfL[0]];
        tmp_L [1]         = invSCF [comp_L[1] - scfL[1]];
        tmp_L [2]         = invSCF [comp_L[2] - scfL[2]];
        tmp_R [0]         = invSCF [comp_R[0] - scfR[0]];
        tmp_R [1]         = invSCF [comp_R[1] - scfR[1]];
        tmp_R [2]         = invSCF [comp_R[2] - scfR[2]];
        SNR_comp_L [Band] = (tmp_L[0]*tmp_L[0] + tmp_L[1]*tmp_L[1] + tmp_L[2]*tmp_L[2]) * 0.3333333333f;
        SNR_comp_R [Band] = (tmp_R[0]*tmp_R[0] + tmp_R[1]*tmp_R[1] + tmp_R[2]*tmp_R[2]) * 0.3333333333f;

        // normalize the subband samples
        facL = invSCF[scfL[0]];
        facR = invSCF[scfR[0]];
        for ( n = 0; n < 12; n++ ) {
            x[Band].L[n] *= facL;
            x[Band].R[n] *= facR;
        }
        facL = invSCF[scfL[1]];
        facR = invSCF[scfR[1]];
        for ( n = 12; n < 24; n++ ) {
            x[Band].L[n] *= facL;
            x[Band].R[n] *= facR;
        }
        facL = invSCF[scfL[2]];
        facR = invSCF[scfR[2]];
        for ( n = 24; n < 36; n++ ) {
            x[Band].L[n] *= facL;
            x[Band].R[n] *= facR;
        }

        // limit to +/-32767 if internal clipping
        if ( warnL )
            for ( n = 0; n < 36; n++ ) {
                if      (x[Band].L[n] > +32767.f) {
                    Overflows++;
                    MaxOverFlow = maxf (MaxOverFlow,  x[Band].L[n]);
                    x[Band].L[n] = 32767.f;
                }
                else if (x[Band].L[n] < -32767.f) {
                    Overflows++;
                    MaxOverFlow = maxf (MaxOverFlow, -x[Band].L[n]);
                    x[Band].L[n] = -32767.f;
                }
            }
        if ( warnR )
            for ( n = 0; n < 36; n++ ) {
                if      (x[Band].R[n] > +32767.f) {
                    Overflows++;
                    MaxOverFlow = maxf (MaxOverFlow,  x[Band].R[n]);
                    x[Band].R[n] = 32767.f;
                }
                else if (x[Band].R[n] < -32767.f) {
                    Overflows++;
                    MaxOverFlow = maxf (MaxOverFlow, -x[Band].R[n]);
                    x[Band].R[n] = -32767.f;
                }
            }
    }

    LEAVE(4);
    return;
}


static void
Quantisierung ( const int               MaxBand,
                const int*              resL,
                const int*              resR,
                const SubbandFloatTyp*  subx,
                SubbandQuantTyp*        subq )
{
    static float  errorL [32] [36 + MAX_NS_ORDER];
    static float  errorR [32] [36 + MAX_NS_ORDER];
    int           Band;

    ENTER(5);

    // quantize Subband- and Subframe-samples
    for ( Band = 0; Band <= MaxBand; Band++, resL++, resR++ ) {

        if ( *resL > 0 ) {
            if ( NS_Order_L [Band] > 0 ) {
                QuantizeSubbandWithNoiseShaping ( subq[Band].L, subx[Band].L, *resL, errorL [Band], FIR_L [Band] );
                memcpy ( errorL [Band], errorL[Band] + 36, MAX_NS_ORDER * sizeof (**errorL) );
            } else {
                QuantizeSubband                 ( subq[Band].L, subx[Band].L, *resL, errorL [Band] );
                memcpy ( errorL [Band], errorL[Band] + 36, MAX_NS_ORDER * sizeof (**errorL) );
            }
        } else {
        }

        if ( *resR > 0 ) {
            if ( NS_Order_R [Band] > 0 ) {
                QuantizeSubbandWithNoiseShaping ( subq[Band].R, subx[Band].R, *resR, errorR [Band], FIR_R [Band] );
                memcpy ( errorR [Band], errorR [Band] + 36, MAX_NS_ORDER * sizeof (**errorL) );
            } else {
                QuantizeSubband                 ( subq[Band].R, subx[Band].R, *resR, errorL [Band] );
                memcpy ( errorR [Band], errorR [Band] + 36, MAX_NS_ORDER * sizeof (**errorL) );
            }
        } else {
        }
    }

    LEAVE(5);
    return;
}


static int
PNS_SCF ( int* scf, float S0, float S1, float S2 )
{
//    printf ("%7.1f %7.1f %7.1f  ", sqrt(S0/12), sqrt(S1/12), sqrt(S2/12) );

#if 1
    if ( S0 < 0.5 * S1  ||  S1 < 0.5 * S2  ||  S0 < 0.5 * S2 )
        return 0;

    if ( S1 < 0.25 * S0  ||  S2 < 0.25 * S1  ||  S2 < 0.25 * S0 )
        return 0;
#endif


    if ( S0 >= 0.8 * S1 ) {
        if ( S0 >= 0.8 * S2  &&  S1 > 0.8 * S2 )
            S0 = S1 = S2 = 0.33333333333f * (S0 + S1 + S2);
        else
            S0 = S1 = 0.5f * (S0 + S1);
    }
    else {
        if ( S1 >= 0.8 * S2 )
            S1 = S2 = 0.5f * (S1 + S2);
    }

    scf [0] = scf [1] = scf [2] = 63;
    S0 = sqrt (S0/12 * 4/1.2005080577484075047860806747022);
    S1 = sqrt (S1/12 * 4/1.2005080577484075047860806747022);
    S2 = sqrt (S2/12 * 4/1.2005080577484075047860806747022);
    if (S0 > 0.) scf [0] = IFLOORF (-12.6f * LOG10 (S0) + 57.8945021823f );
    if (S1 > 0.) scf [1] = IFLOORF (-12.6f * LOG10 (S1) + 57.8945021823f );
    if (S2 > 0.) scf [2] = IFLOORF (-12.6f * LOG10 (S2) + 57.8945021823f );

    if ( scf[0] & ~63 ) scf[0] = scf[0] > 63 ? 63 : 0;
    if ( scf[1] & ~63 ) scf[1] = scf[1] > 63 ? 63 : 0;
    if ( scf[2] & ~63 ) scf[2] = scf[2] > 63 ? 63 : 0;

    return 1;
}


static void
Allocate ( const int MaxBand, int* res, float* x, int* scf, const float* comp, const float* smr, const SCFTriple* Pow, const int* Transient )
{
    int    Band;
    int    k;
    float  tmpMNR;      // to adjust the scalefactors
    float  save [36];   // to adjust the scalefactors
    float  MNR;         // Mask-to-Noise ratio

    ENTER(6);

    for ( Band = 0; Band <= MaxBand; Band++, res++, comp++, smr++, scf += 3, x += 72 ) {
        // printf ( "%2u: %u\n", Band, Transient[Band] );

        // Find out needed quantization resolution Res to fulfill the calculated MNR
        // This is done by exactly measuring the quantization residuals against the signal itself
        // Starting with Res=1  Res in increased until MNR becomes less than 1.
        if ( Band > 0  &&  res[-1] < 3  &&  *smr >= 1. &&  *smr < Band * PNS  &&
             PNS_SCF ( scf, Pow [Band][0], Pow [Band][1], Pow [Band][2] ) ) {
            *res = -1;
        } else {
            for ( MNR = *smr * 1.; MNR > 1.  &&  *res != 15; )
                MNR = *smr * (Transient[Band] ? ISNR_Schaetzer_Trans : ISNR_Schaetzer) ( x, *comp, ++*res );
        }

        // Fine adapt SCF's (MNR > 0 prevents adaption of zero samples, which is nonsense)
        // only apply to Huffman-coded samples (otherwise no savings in bitrate)
        if ( *res > 0  &&  *res <= LAST_HUFFMAN  &&  MNR < 1.  &&  MNR > 0.  &&  !Transient[Band] ) {
            while ( scf[0] > 0  &&  scf[1] > 0  &&  scf[2] > 0 ) {

                --scf[2]; --scf[1]; --scf[0];                   // adapt scalefactors and samples
                memcpy ( save, x, sizeof save );
                for (k = 0; k < 36; k++ )
                    x[k] *= SCFfac;

                tmpMNR = *smr * (Transient[Band] ? ISNR_Schaetzer_Trans : ISNR_Schaetzer) ( x, *comp, *res );// recalculate MNR

                // FK: if ( tmpMNR > MNR  &&  tmpMNR <= 1 ) {          // check for MNR
                if ( tmpMNR <= 1 ) {                            // check for MNR
                    MNR = tmpMNR;
                }
                else {
                    ++scf[0]; ++scf[1]; ++scf[2];               // restore scalefactors and samples
                    memcpy ( x, save, sizeof save );
                    break;
                }
            }
        }

    }
    LEAVE(6);
    return;
}




typedef struct {
    float            ShortThr;
    unsigned char    MinValChoice;
    unsigned int     EarModelFlag;
    signed char      Ltq_offset;
    float            TMN;
    float            NMT;
    signed char      minSMR;
    signed char      Ltq_max;
    unsigned short   BandWidth;
    unsigned char    tmpMask_used;
    unsigned char    CVD_used;
    float            varLtq;
    unsigned char    MS_Channelmode;
    unsigned char    CombPenalities;
    unsigned char    NS_Order;
    float            PNS;
    float            TransDetect;
} Profile_Setting_t;


#define PROFILE_PRE2_TELEPHONE   5      // --quality  0
#define PROFILE_PRE_TELEPHONE    6      // --quality  1
#define PROFILE_TELEPHONE        7      // --quality  2
#define PROFILE_THUMB            8      // --quality  3
#define PROFILE_RADIO            9      // --quality  4
#define PROFILE_STANDARD        10      // --quality  5
#define PROFILE_XTREME          11      // --quality  6
#define PROFILE_INSANE          12      // --quality  7
#define PROFILE_BRAINDEAD       13      // --quality  8
#define PROFILE_POST_BRAINDEAD  14      // --quality  9
#define PROFILE_POST2_BRAINDEAD 15      // --quality 10


static const Profile_Setting_t  Profiles [16] = {
    { 0 },
    { 0 },
    { 0 },
    { 0 },
    { 0 },
/*    Short   MinVal  EarModel  Ltq_                min   Ltq_  Band-  tmpMask  CVD_  varLtq    MS   Comb   NS_        Trans */
/*    Thr     Choice  Flag      offset  TMN   NMT   SMR   max   Width  _used    used         channel Penal used  PNS    Det  */
    { 1.e9f,  1,      300,       30,    3.0, -1.0,    0,  106,   4820,   1,      1,    1.,      3,     24,  6,   1.09f, 200 },  // 0: pre-Telephone
    { 1.e9f,  1,      300,       24,    6.0,  0.5,    0,  100,   7570,   1,      1,    1.,      3,     20,  6,   0.77f, 180 },  // 1: pre-Telephone
    { 1.e9f,  1,      400,       18,    9.0,  2.0,    0,   94,  10300,   1,      1,    1.,      4,     18,  6,   0.55f, 160 },  // 2: Telephone
    { 50.0f,  2,      430,       12,   12.0,  3.5,    0,   88,  13090,   1,      1,    1.,      5,     15,  6,   0.39f, 140 },  // 3: Thumb
    { 15.0f,  2,      440,        6,   15.0,  5.0,    0,   82,  15800,   1,      1,    1.,      6,     10,  6,   0.27f, 120 },  // 4: Radio
    {  5.0f,  2,      550,        0,   18.0,  6.5,    1,   76,  19980,   1,      2,    1.,     11,      9,  6,   0.00f, 100 },  // 5: Standard
    {  4.0f,  2,      560,       -6,   21.0,  8.0,    2,   70,  22000,   1,      2,    1.,     12,      7,  6,   0.00f,  80 },  // 6: Xtreme
    {  3.0f,  2,      570,      -12,   24.0,  9.5,    3,   64,  24000,   1,      2,    2.,     13,      5,  6,   0.00f,  60 },  // 7: Insane
    {  2.8f,  2,      580,      -18,   27.0, 11.0,    4,   58,  26000,   1,      2,    4.,     13,      4,  6,   0.00f,  40 },  // 8: BrainDead
    {  2.6f,  2,      590,      -24,   30.0, 12.5,    5,   52,  28000,   1,      2,    8.,     13,      4,  6,   0.00f,  20 },  // 9: post-BrainDead
    {  2.4f,  2,      599,      -30,   33.0, 14.0,    6,   46,  30000,   1,      2,   16.,     15,      2,  6,   0.00f,  10 },  //10: post-BrainDead
};


static int
TestProfileParams ( void )
{   //                                       0    1    2    3    4   5   6  7 8 9  10  11  12  13 14  15
    static signed char  TMNStereoAdj [] = { -6, -18, -15, -18, -12, -9, -6, 0,0,0, +1, +1, +1, +1, 0, +1 };  // Penalties for TMN
    static signed char  NMTStereoAdj [] = { -3, -18, -15, -15,  -9, -6, -3, 0,0,0,  0, +1, +1, +1, 0, +1 };  // Penalties for NMT
    int                 i;

    MainQual = PROFILE_PRE2_TELEPHONE;

    for ( i = PROFILE_PRE2_TELEPHONE; i <= PROFILE_POST2_BRAINDEAD; i++ ) {
        if ( ShortThr     > Profiles [i].ShortThr     ) continue;
        if ( MinValChoice < Profiles [i].MinValChoice ) continue;
        if ( EarModelFlag < Profiles [i].EarModelFlag ) continue;
        if ( Ltq_offset   > Profiles [i].Ltq_offset   ) continue;
        if ( Ltq_max      > Profiles [i].Ltq_max      ) continue;                     // offset should normally be considered here
        if ( TMN + TMNStereoAdj [MS_Channelmode] <
             Profiles [i].TMN + TMNStereoAdj [Profiles [i].MS_Channelmode] )
                                                        continue;
        if ( NMT + NMTStereoAdj [MS_Channelmode] <
             Profiles [i].NMT + NMTStereoAdj [Profiles [i].MS_Channelmode] )
                                                        continue;
        if ( minSMR       < Profiles [i].minSMR       ) continue;
        if ( Bandwidth    < Profiles [i].BandWidth    ) continue;
        if ( tmpMask_used < Profiles [i].tmpMask_used ) continue;
        if ( CVD_used     < Profiles [i].CVD_used     ) continue;
     // if ( varLtq       > Profiles [i].varLtq       ) continue;
     // if ( NS_Order     < Profiles [i].NS_Order     ) continue;
        if ( PNS          > Profiles [i].PNS          ) continue;
        MainQual = i;
    }
    return MainQual;
}


static void
SetQualityParams ( float qual )
{
    int    i;
    float  mix;

    if      ( qual <  0. ) {
        qual =  0.;
    }
    if      ( qual > 10. ) {
        qual = 10.;
#ifdef _WIN32
        stderr_printf ( "\nmppenc: Can't open MACDll.dll, quality set to 10.0\n" );
#else
        stderr_printf ( "\nmppenc: Can't open libMAC.so, quality set to 10.0\n" );
#endif
    }

    i   = (int) qual + PROFILE_PRE2_TELEPHONE;
    mix = qual - (int) qual;

    MainQual       = i;
    ShortThr       = Profiles [i].ShortThr   * (1-mix) + Profiles [i+1].ShortThr   * mix;
    MinValChoice   = Profiles [i].MinValChoice  ;
    EarModelFlag   = Profiles [i].EarModelFlag  ;
    Ltq_offset     = Profiles [i].Ltq_offset * (1-mix) + Profiles [i+1].Ltq_offset * mix;
    varLtq         = Profiles [i].varLtq     * (1-mix) + Profiles [i+1].varLtq     * mix;
    Ltq_max        = Profiles [i].Ltq_max    * (1-mix) + Profiles [i+1].Ltq_max    * mix;
    TMN            = Profiles [i].TMN        * (1-mix) + Profiles [i+1].TMN        * mix;
    NMT            = Profiles [i].NMT        * (1-mix) + Profiles [i+1].NMT        * mix;
    minSMR         = Profiles [i].minSMR        ;
    Bandwidth      = Profiles [i].BandWidth  * (1-mix) + Profiles [i+1].BandWidth  * mix;
    tmpMask_used   = Profiles [i].tmpMask_used  ;
    CVD_used       = Profiles [i].CVD_used      ;
    MS_Channelmode = Profiles [i].MS_Channelmode;
    CombPenalities = Profiles [i].CombPenalities;
    NS_Order       = Profiles [i].NS_Order      ;
    PNS            = Profiles [i].PNS        * (1-mix) + Profiles [i+1].PNS        * mix;
    TransDetect    = Profiles [i].TransDetect* (1-mix) + Profiles [i+1].TransDetect* mix;
}


/* Planned: return the evaluated options, without InputFile and OutputFile, argc implicit instead of explicit */

static int
EvalParameters ( int argc, char** argv, char** InputFile, char** OutputFile, int onlyfilenames )
{
    int          k;
    size_t       len;
    static char  output [2048];
    static char  errmsg [] = "\n\033[33;41;1mERROR\033[0m: Missing argument for option '--%s'\n\n";
    FILE*        fp;
    char*        p;
    char         buff [32768];

    /********************************* In / Out Files *********************************/
    *InputFile  = argv [argc-1];
    *OutputFile = NULL;

    // search for output file
    if ( argc >= 3 ) {
        len = strlen (argv[argc-1]);

        if ( strcmp (argv[argc-1], "/dev/null") == 0  ||
             strcmp (argv[argc-1], "-")         == 0  ||
             (len >= 4  &&  (0 == strcasecmp (argv [argc-1] + len - 4, ".MPC")  ||
                             0 == strcasecmp (argv [argc-1] + len - 4, ".MPP")  ||
                             0 == strcasecmp (argv [argc-1] + len - 4, ".MP+"))) ) {
            *OutputFile = argv[argc-1];
            *InputFile  = argv[argc-2];
            argc -= 2;
        }
    }

    // if no Output-File is stated, set OutFile to InFile.mpc
    if ( *OutputFile == NULL  ) {
        strcpy ( *OutputFile = output, *InputFile );
        len = strlen ( output );
        if ( len > 4  &&  output[len-4] == '.' )
            len -= 4;
        strcpy (output+len, ".mpc");
        argc -= 1;
    }

    if ( onlyfilenames )
        return 0;

    /********************************* In / Out Files *********************************/


    // search for options
    for ( k = 1; k < argc; k++ ) {

        const char*  arg = argv [k];

        if ( arg[0] != '-'  ||  arg[1] != '-' )
            continue;
        arg += 2;

        if      ( 0 == strcmp ( arg, "verbose" ) ) {                                     // verbose
            verbose++;
        }
        else if ( 0 == strcmp ( arg, "telephone" ) ) {                                   // MainQual
            SetQualityParams (2.0);
        }
        else if ( 0 == strcmp ( arg, "thumb" ) ) {                                       // MainQual
            SetQualityParams (3.0);
        }
        else if ( 0 == strcmp ( arg, "radio"   ) ) {
            SetQualityParams (4.0);
        }
        else if ( 0 == strcmp ( arg, "standard")  ||  0 == strcmp ( arg, "normal") ) {
            SetQualityParams (5.0);
        }
        else if ( 0 == strcmp ( arg, "xtreme")  ||  0 == strcmp ( arg, "extreme") ) {
            SetQualityParams (6.0);
        }
        else if ( 0 == strcmp ( arg, "insane") ) {
            SetQualityParams (7.0);
        }
        else if ( 0 == strcmp ( arg, "braindead") ) {
            SetQualityParams (8.0);
        }
        else if ( 0 == strcmp ( arg, "quality") ) {                                      // Quality
            if ( ++k >= argc ) { stderr_printf ( errmsg, arg ); return -1; }
            SetQualityParams (atof (argv[k]) );
        }
        else if ( 0 == strcmp ( arg, "neveroverwrite") ) {                              // NeverOverWrite
            WriteMode = MODE_NEVER_OVERWRITE;
        }
        else if ( 0 == strcmp ( arg, "forcewrite")  ||  0 == strcmp ( arg, "overwrite") ) { // ForceWrite
            WriteMode = MODE_OVERWRITE;
        }
        else if ( 0 == strcmp ( arg, "interactive")  ) {                                // Interactive
            WriteMode = MODE_ASK_FOR_OVERWRITE;
        }
        else if ( 0 == strcmp ( arg, "delinput")  ||  0 == strcmp ( arg, "delete")  ||  0 == strcmp ( arg, "deleteinput" ) ) {                                    // DelInput
            DelInput = 0xAFFEDEAD;
        }
        else if ( 0 == strcmp ( arg, "beep")  ) {
            IsEndBeep = 1;
        }
        else if ( 0 == strcmp ( arg, "scale") ) {                                       // ScalingFactor
            if ( ++k >= argc ) { stderr_printf ( errmsg, arg ); return -1; }
            ScalingFactorl = ScalingFactorr = (float) atof (argv[k]);
            if (strchr (argv[k], ','))
                ScalingFactorr = (float) atof (strchr (argv[k], ',') + 1);
            if ( ScalingFactorl == 0.97f  ||  ScalingFactorl == 0.98f ) stderr_printf ("--scale 0.97 or --scale 0.98 is nearly useless to prevent clipping. Use replaygain tool\nto determine EXACT attenuation to avoid clipping. Factor can be between 0.696 and 1.000.\nSee \"http://www.uni-jena.de/~pfk/mpp/clipexample.html\".\n\n" );
        }
        else if ( 0 == strcmp ( arg, "kbd") ) {                                       // ScalingFactor
            if ( ++k >= argc ) { stderr_printf ( errmsg, arg ); return -1; }
            if ( 2 != sscanf ( argv[k], "%f,%f", &KBD1, &KBD2 ))
                { stderr_printf ( "%s: missing two arguments", arg ); return -1; }
            Init_FFT ();
        }
        else if ( 0 == strcmp ( arg, "fadein") ) {                                      // FadeInTime
            if ( ++k >= argc ) { stderr_printf ( errmsg, arg ); return -1; }
            FadeInTime = (float) atof (argv[k]);
            if ( FadeInTime < 0.f ) FadeInTime = 0.f;
        }
        else if ( 0 == strcmp ( arg, "fadeout") ) {                                     // FadeOutTime
            if ( ++k >= argc ) { stderr_printf ( errmsg, arg ); return -1; }
            FadeOutTime = (float) atof (argv[k]);
            if ( FadeOutTime < 0.f ) FadeOutTime = 0.f;
        }
        else if ( 0 == strcmp ( arg, "fade") ) {                                        // FadeInTime + FadeOutTime
            if ( ++k >= argc ) { stderr_printf ( errmsg, arg ); return -1; }
            FadeOutTime = (float) atof (argv[k]);
            if ( FadeOutTime < 0.f ) FadeOutTime = 0.f;
            FadeInTime = FadeOutTime;
        }
        else if ( 0 == strcmp ( arg, "fadeshape") ) {                                   // FadeOutTime
            if ( ++k >= argc ) { stderr_printf ( errmsg, arg ); return -1; }
            FadeShape = (float) atof (argv[k]);
            if ( FadeShape < 0.001f  ||  FadeShape > 1000.f ) FadeShape = 1.f;
            setbump ( FadeShape );
        }
        else if ( 0 == strcmp ( arg, "skip")  ||  0 == strcmp ( arg, "start") ) {       // SkipTime
            if ( ++k >= argc ) { stderr_printf ( errmsg, arg ); return -1; }
            SkipTime = (float) atof (argv[k]);
        }
        else if ( 0 == strcmp ( arg, "dur")  ||  0 == strcmp ( arg, "duration") ) {     // maximum Duration
            if ( ++k >= argc ) { stderr_printf ( errmsg, arg ); return -1; }
            Duration = atof (argv[k]);
        }
        else if ( 0 == strcmp ( arg, "ans") ) {                                         // AdaptiveNoiseShaping
            if ( ++k >= argc ) { stderr_printf ( errmsg, arg ); return -1; }
            NS_Order = atoi (argv[k]);
            NS_Order = mini ( NS_Order, MAX_NS_ORDER );
        }
        else if ( 0 == strcmp ( arg, "predict") ) {                                     // AdaptiveNoiseShaping
            if ( ++k >= argc ) { stderr_printf ( errmsg, arg ); return -1; }
            PredictionBands = atoi (argv[k]);
            PredictionBands = mini ( PredictionBands, 32 );
        }
        else if ( 0 == strcmp ( arg, "ltq_var")  ||  0 == strcmp ( arg, "ath_var") ) {  // ltq_var
            if ( ++k >= argc ) { stderr_printf ( errmsg, arg ); return -1; }
            varLtq = atof (argv[k]);
        }
        else if ( 0 == strcmp ( arg, "pns") ) {                                         // pns
            if ( ++k >= argc ) { stderr_printf ( errmsg, arg ); return -1; }
            PNS = atof (argv[k]);
        }
        else if ( 0 == strcmp ( arg, "minval") ) {                                      // MinValChoice
            if ( ++k >= argc ) { stderr_printf ( errmsg, arg ); return -1; }
            MinValChoice = atoi (argv[k]);
        }
        else if ( 0 == strcmp ( arg, "transdet") ) {                                    // TransDetect
            if ( ++k >= argc ) { stderr_printf ( errmsg, arg ); return -1; }
            TransDetect = (float) atof (argv[k]);
        }
        else if ( 0 == strcmp ( arg, "shortthr") ) {                                    // ShortThr
            if ( ++k >= argc ) { stderr_printf ( errmsg, arg ); return -1; }
            ShortThr = (float) atof (argv[k]);
        }
        else if ( 0 == strcmp ( arg, "noxlevel") ) {                                      // Xlevel
            XLevel = 0;
        }
        else if ( 0 == strcmp ( arg, "xlevel") ) {                                      // Xlevel
            stderr_printf ( "\nXlevel coding now enabled by default, --xlevel ignored.\n" );
        }
        else if ( 0 == strcmp ( arg, "nmt") ) {                                         // NMT
            if ( ++k >= argc ) { stderr_printf ( errmsg, arg );  return -1; }
            NMT = (float) atof (argv[k]);
        }
        else if ( 0 == strcmp ( arg, "tmn") ) {                                         // TMN
            if ( ++k >= argc ) { stderr_printf ( errmsg, arg );  return -1; }
            TMN = (float) atof (argv[k]);
        }
        else if ( 0 == strcmp ( arg, "cvd") ) {                                         // ClearVoiceDetection
            if ( ++k >= argc ) { stderr_printf ( errmsg, arg ); return -1; }
            CVD_used = atoi (argv[k]);
            if ( CVD_used == 0 )
                stderr_printf ( "\nDisabling CVD always reduces quality!\a\n" );
        }
        else if ( 0 == strcmp ( arg, "ms") ) {                                          // Mid/Side Stereo
            if ( ++k >= argc ) { stderr_printf ( errmsg, arg ); return -1; }
            MS_Channelmode = atoi (argv[k]);
        }
        else if ( 0 == strcmp ( arg, "minSMR") ) {                                      // minimum SMR
            if ( ++k >= argc ) { stderr_printf ( errmsg, arg ); return -1; }
            if ( minSMR > (float) atof (argv[k]) )
                stderr_printf ( "This option usage may reduces quality!\a\n" );
            minSMR = (float) atof (argv[k]);
        }
        else if ( 0 == strcmp ( arg, "tmpMask") ) {                                     // temporal post-masking
            if ( ++k >= argc ) { stderr_printf ( errmsg, arg ); return -1; }
            tmpMask_used = atoi (argv[k]);
        }
        else if ( 0 == strcmp ( arg, "ltq_max")  ||  0 == strcmp ( arg, "ath_max") ) {  // Maximum for threshold in quiet
            if ( ++k >= argc ) { stderr_printf ( errmsg, arg );  return -1; }
            Ltq_max = (float) atof (argv[k]);
        }
        else if ( 0 == strcmp ( arg, "ltq_gain")  ||  0 == strcmp ( arg, "ath_gain") ) {// Offset for threshold in quiet
            if ( ++k >= argc ) { stderr_printf ( errmsg, arg ); return -1; }
            Ltq_offset = (float) atof (argv[k]);
        }
        else if ( 0 == strcmp ( arg, "silent")  ||  0 == strcmp ( arg, "quiet") ) {
            SetStderrSilent (1);
        }
        else if ( 0 == strcmp ( arg, "stderr") ) {                                      // Offset for threshold in quiet
            if ( ++k >= argc ) { stderr_printf ( errmsg, arg ); return -1; }
            freopen ( argv[k], "a", stderr );
        }
        else if ( 0 == strcmp ( arg, "ltq")  ||  0 == strcmp ( arg, "ath") ) {          // threshold in quiet
            if ( ++k >= argc ) { stderr_printf ( errmsg, arg ); return -1; }
            EarModelFlag = atoi (argv[k]);
        }
        else if ( 0 == strcmp ( arg, "noco") ) {
            NoiseInjectionComp ();
        }
        else if ( 0 == strcmp ( arg, "newcomb") ) {
            if ( ++k >= argc ) { stderr_printf ( errmsg, arg ); return -1; }
            CombPenalities = atoi (argv[k]);
        }
        else if ( 0 == strcmp ( arg, "ape1") ) {                                     // Mark APE as APE 1.000
            APE_Version = 1000;
        }
        else if ( 0 == strcmp ( arg, "ape2") ) {                                     // Mark APE as APE 2.000
            APE_Version = 2000;
        }
        else if ( 0 == strcmp ( arg, "unicode") ) {                                  // no tag conversion
            NoUnicode = 0;
        }
        else if ( 0 == strcmp ( arg, "lowdelay") ) {
            LowDelay = 1;
        }
        else if ( 0 == strcmp ( arg, "bw")  ||  0 == strcmp ( arg, "lowpass") ) {       // bandwidth
            if ( ++k >= argc ) { stderr_printf ( errmsg, arg ); return -1; }
            Bandwidth = atof (argv[k]);
        }
        else if ( 0 == strcmp ( arg, "displayupdatetime") ) {
            if ( ++k >= argc ) { stderr_printf ( errmsg, arg ); return -1; }
            DisplayUpdateTime = atoi (argv[k]);
        }
        else if ( 0 == strcmp ( arg, "artist" ) ) {
            if ( ++k >= argc ) { stderr_printf ( errmsg, arg ); return -1; }
            p = argv[k];
            addtag ( "Artist", 0, p, strlen(p), NoUnicode, 0 );
        }
        else if ( 0 == strcmp ( arg, "album" ) ) {
            if ( ++k >= argc ) { stderr_printf ( errmsg, arg ); return -1; }
            p = argv[k];
            addtag ( "Album", 0, p, strlen(p), NoUnicode, 0 );
        }
        else if ( 0 == strcmp ( arg, "debutalbum" ) ) {
            if ( ++k >= argc ) { stderr_printf ( errmsg, arg ); return -1; }
            p = argv[k];
            addtag ( "Debut Album", 0, p, strlen(p), NoUnicode, 0 );
        }
        else if ( 0 == strcmp ( arg, "publisher" ) ) {
            if ( ++k >= argc ) { stderr_printf ( errmsg, arg ); return -1; }
            p = argv[k];
            addtag ( "Publisher", 0, p, strlen(p), NoUnicode, 0 );
        }
        else if ( 0 == strcmp ( arg, "conductor" ) ) {
            if ( ++k >= argc ) { stderr_printf ( errmsg, arg ); return -1; }
            p = argv[k];
            addtag ( "Conductor", 0, p, strlen(p), NoUnicode, 0 );
        }
        else if ( 0 == strcmp ( arg, "title" ) ) {
            if ( ++k >= argc ) { stderr_printf ( errmsg, arg ); return -1; }
            p = argv[k];
            addtag ( "Title", 0, p, strlen(p), NoUnicode, 0 );
        }
        else if ( 0 == strcmp ( arg, "subtitle" ) ) {
            if ( ++k >= argc ) { stderr_printf ( errmsg, arg ); return -1; }
            p = argv[k];
            addtag ( "Subtitle", 0, p, strlen(p), NoUnicode, 0 );
        }
        else if ( 0 == strcmp ( arg, "track" ) ) {
            if ( ++k >= argc ) { stderr_printf ( errmsg, arg ); return -1; }
            p = argv[k];
            addtag ( "Track", 0, p, strlen(p), NoUnicode, 0 );
        }
        else if ( 0 == strcmp ( arg, "comment" ) ) {
            if ( ++k >= argc ) { stderr_printf ( errmsg, arg ); return -1; }
            p = argv[k];
            addtag ( "Comment", 0, p, strlen(p), NoUnicode, 0 );
        }
        else if ( 0 == strcmp ( arg, "composer" ) ) {
            if ( ++k >= argc ) { stderr_printf ( errmsg, arg ); return -1; }
            p = argv[k];
            addtag ( "Composer", 0, p, strlen(p), NoUnicode, 0 );
        }
        else if ( 0 == strcmp ( arg, "copyright" ) ) {
            if ( ++k >= argc ) { stderr_printf ( errmsg, arg ); return -1; }
            p = argv[k];
            addtag ( "Copyright", 0, p, strlen(p), NoUnicode, 0 );
        }
        else if ( 0 == strcmp ( arg, "publicationright" ) ) {
            if ( ++k >= argc ) { stderr_printf ( errmsg, arg ); return -1; }
            p = argv[k];
            addtag ( "Publicationright", 0, p, strlen(p), NoUnicode, 0 );
        }
        else if ( 0 == strcmp ( arg, "filename" ) ) {
            if ( ++k >= argc ) { stderr_printf ( errmsg, arg ); return -1; }
            p = argv[k];
            addtag ( "File", 0, p, strlen(p), NoUnicode, 0 );
        }
        else if ( 0 == strcmp ( arg, "recordlocation" ) ) {
            if ( ++k >= argc ) { stderr_printf ( errmsg, arg ); return -1; }
            p = argv[k];
            addtag ( "Record Location", 0, p, strlen(p), NoUnicode, 0 );
        }
        else if ( 0 == strcmp ( arg, "recorddate" ) ) {
            if ( ++k >= argc ) { stderr_printf ( errmsg, arg ); return -1; }
            p = argv[k];
            addtag ( "Record Date", 0, p, strlen(p), NoUnicode, 0 );
        }
        else if ( 0 == strcmp ( arg, "ean/upc" ) ) {
            if ( ++k >= argc ) { stderr_printf ( errmsg, arg ); return -1; }
            p = argv[k];
            addtag ( "EAN/UPC", 0, p, strlen(p), NoUnicode, 0 );
        }
        else if ( 0 == strcmp ( arg, "year" )  ||  0 == strcmp ( arg, "releasedate") ) {
            if ( ++k >= argc ) { stderr_printf ( errmsg, arg ); return -1; }
            p = argv[k];
            addtag ( "Year", 0, p, strlen(p), NoUnicode, 0 );
        }
        else if ( 0 == strcmp ( arg, "genre" ) ) {
            if ( ++k >= argc ) { stderr_printf ( errmsg, arg ); return -1; }
            p = argv[k];
            addtag ( "Genre", 0, p, strlen(p), NoUnicode, 0 );
        }
        else if ( 0 == strcmp ( arg, "media" ) ) {
            if ( ++k >= argc ) { stderr_printf ( errmsg, arg ); return -1; }
            p = argv[k];
            addtag ( "Media", 0, p, strlen(p), NoUnicode, 0 );
        }
        else if ( 0 == strcmp ( arg, "index" ) ) {
            if ( ++k >= argc ) { stderr_printf ( errmsg, arg ); return -1; }
            p = argv[k];
            addtag ( "Index", 0, p, strlen(p), NoUnicode*3, 0 );
        }
        else if ( 0 == strcmp ( arg, "isrc" ) ) {
            if ( ++k >= argc ) { stderr_printf ( errmsg, arg ); return -1; }
            p = argv[k];
            addtag ( "ISRC", 0, p, strlen(p), NoUnicode, 0 );
        }
        else if ( 0 == strcmp ( arg, "abstract" ) ) {
            if ( ++k >= argc ) { stderr_printf ( errmsg, arg ); return -1; }
            p = argv[k];
            addtag ( "Abstract", 0, p, strlen(p), NoUnicode, 0 );
        }
        else if ( 0 == strcmp ( arg, "bibliography" ) ) {
            if ( ++k >= argc ) { stderr_printf ( errmsg, arg ); return -1; }
            p = argv[k];
            addtag ( "Bibliography", 0, p, strlen(p), NoUnicode, 0 );
        }
        else if ( 0 == strcmp ( arg, "introplay" ) ) {
            if ( ++k >= argc ) { stderr_printf ( errmsg, arg ); return -1; }
            p = argv[k];
            addtag ( "Introplay", 0, p, strlen(p), NoUnicode*3, 0 );
        }
        else if ( 0 == strcmp ( arg, "media" ) ) {
            if ( ++k >= argc ) { stderr_printf ( errmsg, arg ); return -1; }
            p = argv[k];
            addtag ( "Media", 0, p, strlen(p), NoUnicode, 0 );
        }
        else if ( 0 == strcmp ( arg, "tag" ) ) {
            if ( ++k >= argc ) { stderr_printf ( errmsg, arg ); return -1; }
            p = strchr ( argv[k], '=' );
            if ( p == NULL )
                addtag ( argv[k], strlen(argv[k]), "", 0, NoUnicode, 0 );
            else
                addtag ( argv[k], p-argv[k], p+1, strlen(p+1), NoUnicode, 0 );
        }
        else if ( 0 == strcmp ( arg, "tagfile" ) ) {
            if ( ++k >= argc ) { stderr_printf ( errmsg, arg ); return -1; }
            p = strchr ( argv[k], '=' );
            if ( p == NULL ) {
                stderr_printf (" Enter value for tag key '%s': ", argv[k] );
                fgets ( buff, sizeof buff, stdin );
                len = strlen (buff);
                while ( len > 0  &&  (buff [len-1] == '\r'  ||  buff [len-1] == '\n') )
                    len--;
                addtag ( arg, strlen(arg), buff, len, NoUnicode*6, 0 );
            }
            else {
                fp = fopen ( p+1, "rb" );
                if ( fp == NULL ) {
                    fprintf ( stderr, "Can't open file '%s'.\n", p+1 );
                }
                else {
                    addtag ( argv[k], p-argv[k], buff, fread (buff,1,sizeof buff,fp), NoUnicode*2, 3 );
                    fclose (fp);
                }
            }
        }
        else {
            char c;
            stderr_printf ( "\n\033[33;41;1mERROR\033[0m: unknown option '--%s' !\n", arg );

            stderr_printf ( "\nNevertheless continue with encoding (Y/n)? \a" );
            c = waitkey ();
            if ( c != 'Y' && c != 'y' ) {
                stderr_printf ( "\n\n*** Abort ***\n" );
                return -1;
            }
            stderr_printf ( "\n" );
        }
    }

    TestProfileParams ();
    return 0;
}


static void



static int
myfeof ( FILE* fp )
{
    int  ch;

    if ( fp != (FILE*)-1 )
        return feof (fp);

    ch = CheckKeyKeep ();
    if ( ch == 'q'  ||  ch == 'Q' )
        return 1;
    return 0;
}

static void fill_float(float * buffer,float val,unsigned count)
{
	unsigned n;
	for(n=0;n<count;n++) buffer[n] = val;
}


static int mainloop ( int argc, char** argv )
{
    SMRTyp           SMR;                       // contains SMRs for the given frame
    PCMDataTyp       Main;                      // contains PCM data for 1600 samples
    SubbandFloatTyp  X [32];                    // Subbandsamples as float()
    SubbandQuantTyp  Q [32];                    // Subband samples after quantization
    wave_t           Wave;                      // contains WAV-files arguments
    UintMax_t        AllSamplesRead   =    0;   // overall read Samples per channel
    unsigned int     CurrentRead      =    0;   // current read Samples per channel
    unsigned int     N;                         // counter for processed frames
    unsigned int     LastValidSamples =    0;   // number of valid samples for the last frame
    unsigned int     LastValidFrame   =    0;   // overall number of frames
    char*            InputName        = NULL;   // Name of WAVE file
    char*            OutputName       = NULL;   // Name of bitstream file
    FILE*            OutputFile       = NULL;   // Filepointer to output file
    int              Silence          =    0;
    int              OldSilence       =    0;
    time_t           T;
    UintMax_t        OldBufferedBits;
    BitstreamPos     bitstreampos;
    int              TransientL [PART_SHORT];   // Flag of transient detection
    int              TransientR [PART_SHORT];   // Flag of transient detection
    int              Transient  [32];           // Flag of transient detection


    ENTER(2);

    // initialize PCM-data
    memset ( &Main, 0, sizeof Main );




    SetQualityParams (5.0);

    if ( EvalParameters ( argc, argv, &InputName, &OutputName, 0 ) < 0 )
        return 1;

    if ( UintMAX_FP(SamplesInWAVE) >= Wave.SampleFreq * (SkipTime + Duration) ) {
        SamplesInWAVE = Wave.SampleFreq * (SkipTime + Duration);
    }

    Init_Psychoakustiktabellen ();              // must be done AFTER decoding command line parameters

    // check fade-length
    if ( FadeInTime + FadeOutTime > UintMAX_FP(SamplesInWAVE) / Wave.SampleFreq ) {
        stderr_printf ( "WARNING: Duration of fade in + out exceeds file length!\n");
        FadeInTime = FadeOutTime = 0.5 * UintMAX_FP(SamplesInWAVE) / Wave.SampleFreq;
    }

    switch ( WriteMode ) {
    default:
        stderr_printf ( "\033[33;41;1mERROR\033[0m: Invalid Write mode, internal error\n" );
        return 1;
    case MODE_NEVER_OVERWRITE:
        OutputFile = fopen ( OutputName, "rb" );
        if ( OutputFile != NULL ) {
            fclose ( OutputFile );
            stderr_printf ( "\033[33;41;1mERROR\033[0m: Output file '%s' already exists\n", OutputName );
            return 1;
        }
        OutputFile = fopen ( OutputName, "w+b" );
        break;
    case MODE_OVERWRITE:
        OutputFile = fopen ( OutputName, "w+b" );
        break;
    case MODE_ASK_FOR_OVERWRITE:
        OutputFile = fopen ( OutputName, "rb" );
        if ( OutputFile != NULL ) {
            char c;
            fclose ( OutputFile );
            stderr_printf ( "\nmppenc: Output file '%s' already exists, overwrite (Y/n)? ", OutputName );
            c = waitkey ();
            if ( c != 'Y'  &&  c != 'y' ) {
                stderr_printf ( "No!!!\n\n*** Canceled overwrite ***\n" );
                return 1;
            }
                            stderr_printf ( " YES\n" );
        }
        OutputFile = fopen ( OutputName, "w+b" );
        break;
    }

    if ( OutputFile == NULL ) {
        stderr_printf ( "\033[33;41;1mERROR\033[0m: Could not create output file '%s'\n", OutputName );
        return 1;
    }

#ifndef IO_BUFFERING
    setvbuf ( OutputFile, NULL, _IONBF, 0 );
#endif

    ShowParameters ( InputName, OutputName );
    if ( WIN32_MESSAGES  &&  FrontendPresent )
        SendModeMessage (MainQual);

    BufferedBits     = 0;
    LastValidFrame   = (SamplesInWAVE + BLOCK - 1) / BLOCK;
    LastValidSamples = (SamplesInWAVE + BLOCK - 1) - BLOCK * LastValidFrame + 1;
    
	WriteHeader_SV7 ( Max_Band, MainQual, MS_Channelmode > 0, LastValidFrame, LastValidSamples, PNS > 0 ? 0x17 : 0x07, SampleFreq );

    // read samples
    CurrentRead     = Read_WAV_Samples ( &Wave, (int)minf(BLOCK, SamplesInWAVE - AllSamplesRead), &Main, CENTER, ScalingFactorl, ScalingFactorr, &Silence );
    AllSamplesRead += CurrentRead;

	if (CurrentRead > 0)
	{
		fill_float( Main.L, Main.L[CENTER], CENTER );
		fill_float( Main.R, Main.R[CENTER], CENTER );
		fill_float( Main.M, Main.M[CENTER], CENTER );
		fill_float( Main.S, Main.S[CENTER], CENTER );
	}

	Analyse_Init ( Main.L[CENTER], Main.R[CENTER], X, Max_Band );

    // adapt SamplesInWAVE to the real number of contained samples
    if ( myfeof (Wave.fp) ) {
        stderr_printf ( "WAVE file has incorrect header: header: %.3f s, contents: %.3f s    \n",
                        UintMAX_FP(AllSamplesRead) / SampleFreq, UintMAX_FP(SamplesInWAVE) / SampleFreq );
        SamplesInWAVE = AllSamplesRead;

        // in the case of a broken wav-header, recalculate the overall frames
        // and the valid samples for the last frame
        LastValidFrame   = (SamplesInWAVE + BLOCK - 1) / BLOCK;
        LastValidSamples = (SamplesInWAVE + BLOCK - 1) - BLOCK * LastValidFrame + 1;
        // fprintf ( stderr, "\nKorrupt WAV file in Frame %d: NEU!: Frames: %u, last valid: %u\n", -1, LastValidFrame, LastValidSamples );
    }

    for ( N = 0; (UintMax_t)N * BLOCK < SamplesInWAVE + DECODER_DELAY; N++ ) {

        // setting residual data-fields to zero
        if ( CurrentRead < BLOCK  &&  N > 0 ) {
            fill_float( Main.L + (CENTER + CurrentRead), Main.L[CENTER + CurrentRead - 1], BLOCK - CurrentRead );
            fill_float( Main.R + (CENTER + CurrentRead), Main.R[CENTER + CurrentRead - 1], BLOCK - CurrentRead );
            fill_float( Main.M + (CENTER + CurrentRead), Main.M[CENTER + CurrentRead - 1], BLOCK - CurrentRead );
            fill_float( Main.S + (CENTER + CurrentRead), Main.S[CENTER + CurrentRead - 1], BLOCK - CurrentRead );
        }

        /********************************************************************/
        /*                         Encoder-Core                             */
        /********************************************************************/
        // you only get null samples at the output of the filterbank when the last frame contains zeroes

        memset ( Res_L, 0, sizeof Res_L );
        memset ( Res_R, 0, sizeof Res_R );

        if ( !Silence  ||  !OldSilence ) {
            Analyse_Filter ( &Main, X, Max_Band );                      // Analysis-Filterbank (Main -> X)
            SMR = Psychoakustisches_Modell ( Max_Band*0+31, &Main, TransientL, TransientR );    // Psychoacoustics return SMRs for input data 'Main'
            if ( minSMR > 0 )
                RaiseSMR ( Max_Band, &SMR );                            // Minimum-operation on SBRs (full bandwidth)
            if ( MS_Channelmode > 0 )
                MS_LR_Entscheidung ( Max_Band, MS_Flag, &SMR, X );      // Selection of M/S- or L/R-Coding
            SCF_Extraktion ( Max_Band, X );                             // Extraction of the scalefactors and normalization of the subband samples
            TransientenCalc ( Transient, TransientL, TransientR );
            if ( NS_Order > 0 ) {
                NS_Analyse ( Max_Band, MS_Flag, SMR, Transient );                  // calculate possible ANS-Filter and the expected gain
            }

            Allocate ( Max_Band, Res_L, X[0].L, SCF_Index_L[0], SNR_comp_L, SMR.L, Power_L, Transient );   // allocate bits for left + right channel
            Allocate ( Max_Band, Res_R, X[0].R, SCF_Index_R[0], SNR_comp_R, SMR.R, Power_R, Transient );

            Quantisierung ( Max_Band, Res_L, Res_R, X, Q );             // quantize samples
        }

        if ( Zaehler >= BUFFER_ALMOST_FULL  ||  LowDelay ) {
            FlushBitstream ( OutputFile, Buffer, Zaehler );
            Zaehler = 0;
         }

        OldSilence      = Silence;
        OldBufferedBits = BufferedBits;
        GetBitstreamPos    ( &bitstreampos );
        WriteBits          ( 0, 20 );                                                      // Reserve 20 bits for jump-information
        WriteBitstream_SV7 ( Max_Band, Q );                                                // write SV7-Bitstream
        WriteBitsAt        ( (Uint32_t)(BufferedBits - OldBufferedBits - 20), 20, bitstreampos );      // Patch 20 bits for jump-information to the right value

        if ( (Int)(time (NULL) - T) >= 0 ) {                            // output
            T += labs (DisplayUpdateTime);
            ShowProgress ( (UintMax_t)(N+1) * BLOCK, SamplesInWAVE, BufferedBits );
        }

        // for backwards-compatibility with older decoders write the 11 bit for
        // reconstruction of exact filelength before the very last frame

        memmove ( Main.L, Main.L + BLOCK, CENTER * sizeof(float) );
        memmove ( Main.R, Main.R + BLOCK, CENTER * sizeof(float) );
        memmove ( Main.M, Main.M + BLOCK, CENTER * sizeof(float) );
        memmove ( Main.S, Main.S + BLOCK, CENTER * sizeof(float) );

		//if ( AllSamplesRead + BLOCK > SamplesInWAVE )
		//{
		//	int n = 0;
		//}

        // read samples
        CurrentRead     = 0;//Read_WAV_Samples ( &Wave, (int)minf(BLOCK, SamplesInWAVE - AllSamplesRead), &Main, CENTER, ScalingFactorl, ScalingFactorr, &Silence );
        AllSamplesRead += CurrentRead;

        // adapt SamplesInWAV to the real number of contained samples
        if ( myfeof (Wave.fp) ) {
            stderr_printf ( "WAVE file has incorrect header: header: %.3f s, contents: %.3f s    \n",
                            UintMAX_FP(AllSamplesRead) / SampleFreq, UintMAX_FP(SamplesInWAVE) / SampleFreq );
            SamplesInWAVE = AllSamplesRead;

            // in the case of broken wav-header, recalculate the overall frames
            // and the valid samples for the last frame
            LastValidFrame   = (SamplesInWAVE + BLOCK - 1) / BLOCK;
            LastValidSamples = (SamplesInWAVE + BLOCK - 1) - BLOCK * LastValidFrame + 1;
            // fprintf ( stderr, "\nKorrupt WAV file in Frame %d: NEU!: Frames: %u, last valid: %u\n", N, LastValidFrame, LastValidSamples );
        }

        if ( N == LastValidFrame - 1 ) {
            WriteBits ( LastValidSamples, 11 );
            // fprintf ( stderr, "\nGültige Samples im letzten Frame: %4u   \n", LastValidSamples );
        }
        if ( N >= LastValidFrame ) {
            // fprintf ( stderr, "Zusätzlicher Frame %u (von %u) angehängt.   \n", N, LastValidFrame );
        }

    }
    LEAVE(2);

    // write the last incomplete word to buffer, so it's written during the next flush
    FinishBitstream();
    ShowProgress ( SamplesInWAVE, SamplesInWAVE, BufferedBits );

    FlushBitstream ( OutputFile, Buffer, Zaehler );
    Zaehler = 0;

    UpdateHeader ( OutputFile, LastValidFrame, LastValidSamples );

    FinalizeTags ( OutputFile, APE_Version );
    fclose ( OutputFile );
    fclose ( Wave.fp );

    if ( DelInput == 0xAFFEDEAD  &&  remove (InputName) == -1 )         // delete input file if DelInput is active
        stderr_printf ( "\n\n\033[33;41;1mERROR\033[0m: Could not delete input file '%s'\n", InputName );

    if ( WIN32_MESSAGES  &&  FrontendPresent )
        SendQuitMessage ();

    stderr_printf ( "\n" );
    return 0;
}


/************ The main() function *****************************/
int main (int argc, char** argv )
{
    int  ret;

    START();
    ENTER(1);

    // initialize tables which must be initialized once and only once

#ifdef FAST_MATH
    Init_FastMath ();                           // check if something has to be done for each file !!
#endif

    Init_SV7 ();
    Init_Psychoakustiktabellen ();
    Init_Skalenfaktoren ();
    Init_Psychoakustik ();
    Init_FPU ();
    Init_ANS ();
    Klemm    ();

    ret = mainloop(argc, argv);              // analyze command line and do the requested work

    /* OverdriveReport ();                         // output a report if clipping was necessary

    if(IsEndBeep)
        stderr_printf("\a\a\a");
	*/

    LEAVE(1);
    REPORT();

    return ret;
}

/* end of mppenc.c */










/******************************************************************************
   --------------- F O R  F E N N E C  P L A Y E R  P L U G - I N ----------
******************************************************************************/

typedef struct _mpc_data
{
    SMRTyp           SMR;                       // contains SMRs for the given frame
    PCMDataTyp       Main;                      // contains PCM data for 1600 samples
    SubbandFloatTyp  X [32];                    // Subbandsamples as float()
    SubbandQuantTyp  Q [32];                    // Subband samples after quantization
    wave_t           Wave;                      // contains WAV-files arguments
    UintMax_t        AllSamplesRead;            // overall read Samples per channel
    unsigned int     CurrentRead;               // current read Samples per channel
    unsigned int     N;                         // counter for processed frames
    unsigned int     LastValidSamples;          // number of valid samples for the last frame
    unsigned int     LastValidFrame;            // overall number of frames
    string           OutputName;                // Name of bitstream file
    FILE*            OutputFile;                // Filepointer to output file
    int              Silence;
    int              OldSilence;
    time_t           T;
    UintMax_t        OldBufferedBits;
    BitstreamPos     bitstreampos;
    int              TransientL [PART_SHORT];   // Flag of transient detection
    int              TransientR [PART_SHORT];   // Flag of transient detection
    int              Transient  [32];           // Flag of transient detection

}mpc_data;




int  musepack_init(int samplerate, int channels, int quality, mpc_data *md)
{
	START();
    ENTER(1);

#ifdef FAST_MATH
    Init_FastMath ();
#endif

    Init_SV7 ();
    Init_Psychoakustiktabellen ();
    Init_Skalenfaktoren ();
    Init_Psychoakustik ();
    Init_FPU ();
    Init_ANS ();
    Klemm    ();
	
	ENTER(2);

	memset(md, 0, sizeof(mpc_data));
	
    SampleFreq    = samplerate;

	md->Wave.Channels   = channels;
	md->Wave.SampleFreq = SampleFreq;

    if(SampleFreq != 44100.0  &&  SampleFreq != 48000.0  &&  SampleFreq != 37800.0  &&  SampleFreq != 32000.0)return 1;

	SetQualityParams (5.0);

	Init_Psychoakustiktabellen();

	md->OutputFile = _wfopen (OutputName, uni("w+b"));
	if(!md->OutputFile == NULL )return 1;

	WriteHeader_SV7 ( Max_Band, MainQual, MS_Channelmode > 0, 1000 /* FIX */, 1000 /* FIX */, PNS > 0 ? 0x17 : 0x07, SampleFreq);

	return 0;
}

long  musepack_encode(float *samples_left, float *samples_right, int scount, mpc_data *md)
{
	long out_scount = scount;

	CurrentRead     = Read_WAV_Samples (&md->Wave, (int)minf(BLOCK, scount), &Main, CENTER, ScalingFactorl, ScalingFactorr, &Silence );
    AllSamplesRead += CurrentRead;

	if (CurrentRead > 0)
	{
		fill_float( Main.L, Main.L[CENTER], CENTER );
		fill_float( Main.R, Main.R[CENTER], CENTER );
		fill_float( Main.M, Main.M[CENTER], CENTER );
		fill_float( Main.S, Main.S[CENTER], CENTER );
	}

	Analyse_Init ( Main.L[CENTER], Main.R[CENTER], X, Max_Band );





	return out_scount;
}


int  musepack_finish(mpc_data *md)
{
	LEAVE(1);
    REPORT();
	return 0;
}











