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

// Antialiasing for calculation of the subband power
const float  Butfly    [7] = { 0.5f, 0.2776f, 0.1176f, 0.0361f, 0.0075f, 0.000948f, 0.0000598f };

// Antialiasing for calculation of the masking thresholds
const float  InvButfly [7] = { 2.f, 3.6023f, 8.5034f, 27.701f, 133.33f, 1054.852f, 16722.408f };

// w_low for long               0    1    2    3    4    5    6    7    8    9   10   11   12   13   14   15   16   17   18   19   20   21   22   23   24   25   26   27   28   29   30   31   32   33   34   35   36   37   38   39   40   41   42   43   44   45   46   47   48   49   50   51   52   53   54   55   56
const int   wl [PART_LONG] = {  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  13,  15,  17,  19,  21,  23,  25,  27,  29,  31,  33,  35,  38,  41,  44,  47,  50,  54,  58,  62,  67,  72,  78,  84,  91,  98, 106, 115, 124, 134, 145, 157, 170, 184, 199, 216, 234, 254, 276, 301, 329, 360, 396, 437, 485 };
const int   wh [PART_LONG] = {  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  12,  14,  16,  18,  20,  22,  24,  26,  28,  30,  32,  34,  37,  40,  43,  46,  49,  53,  57,  61,  66,  71,  77,  83,  90,  97, 105, 114, 123, 133, 144, 156, 169, 183, 198, 215, 233, 253, 275, 300, 328, 359, 395, 436, 484, 511 };
// Width:                       1    1    1    1    1    1    1    1    1    1    1    2    2    2    2    2    2    2    2    2    2    2    2    3    3    3    3    3    4    4    4    5    5    6    6    7    7    8    9    9   10   11   12   13   14   15   17   18   20   22   25   28   31   36   41   48   27

// inverse partition-width for long
const float iw [PART_LONG] = { 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f/2, 1.f/2, 1.f/2, 1.f/2, 1.f/2, 1.f/2, 1.f/2, 1.f/2, 1.f/2, 1.f/2, 1.f/2, 1.f/2, 1.f/3, 1.f/3, 1.f/3, 1.f/3, 1.f/3, 1.f/4, 1.f/4, 1.f/4, 1.f/5, 1.f/5, 1.f/6, 1.f/6, 1.f/7, 1.f/7, 1.f/8, 1.f/9, 1.f/9, 1.f/10, 1.f/11, 1.f/12, 1.f/13, 1.f/14, 1.f/15, 1.f/17, 1.f/18, 1.f/20, 1.f/22, 1.f/25, 1.f/28, 1.f/31, 1.f/36, 1.f/41, 1.f/48, 1.f/27 };

// w_low for short                    0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17   18
const int   wl_short [PART_SHORT] = { 0,  1,  2,  3,  4,  5,  6,  8, 10, 12, 15, 18, 23, 29, 36, 46, 59, 75,  99 };
const int   wh_short [PART_SHORT] = { 0,  1,  2,  3,  5,  6,  7,  9, 12, 14, 18, 23, 29, 36, 46, 58, 75, 99, 127 };

// inverse partition-width for short
const float iw_short [PART_SHORT] = { 1.f, 1.f, 1.f, 1.f, 1.f/2, 1.f/2, 1.f/2, 1.f/2, 1.f/3, 1.f/3, 1.f/4, 1.f/6, 1.f/7, 1.f/8, 1.f/11, 1.f/13, 1.f/17, 1.f/25, 1.f/29 };

/*
Nr.   wl  wh     fl    fh     bl         bh         bm        Nr.   wl  wh     fl    fh     bl         bh         bm
 0:    0   0      0     0   0.000000   0.000000   0.000000
 1:    1   1     43    43   0.425460   0.425460   0.425460     0:   0   0      0     0   0.000000   0.000000     0.000000
 2:    2   2     86    86   0.850241   0.850241   0.850241

 3:    3   3    129   129   1.273448   1.273448   1.273448
 4:    4   4    172   172   1.694205   1.694205   1.694205     1:   1   1    172   172   1.694205   1.694205     1.694205
 5:    5   5    215   215   2.111672   2.111672   2.111672

 6:    6   6    258   258   2.525051   2.525051   2.525051
 7:    7   7    301   301   2.933594   2.933594   2.933594     2:   2   2    345   345   3.336612   3.336612     3.336612
 8:    8   8    345   345   3.336612   3.336612   3.336612

 9:    9   9    388   388   3.733479   3.733479   3.733479
10:   10  10    431   431   4.123635   4.123635   4.123635     3:   3   3    517   517   4.881924   4.881924     4.881924
11-   11  12    474   517   4.506591   4.881924   4.695234

12:   13  14    560   603   5.249283   5.608381   5.429880
13:   15  16    646   689   5.958998   6.300971   6.131073     4:   4   5    689   861   6.300971   7.581073     6.958618
14:   17  18    732   775   6.634195   6.958618   6.797509

15:   19  20    818   861   7.274232   7.581073   7.428745
16:   21  22    904   947   7.879211   8.168753   8.025049     5:   5   6    861  1034   7.581073   8.722594     8.168753
17:   23  24    991  1034   8.449828   8.722594   8.587239

18:   25  26   1077  1120   8.987223   9.243908   9.116546
19:   27  28   1163  1206   9.492850   9.734263   9.614484     6:   6   7   1034  1206   8.722594   9.734263     9.243908
20:   29  30   1249  1292   9.968365  10.195382  10.082745

21:   31  32   1335  1378  10.415539  10.629064  10.523116
22:   33  34   1421  1464  10.836184  11.037125  10.937413     7:   8   9   1378  1550  10.629064  11.421352    11.037125
23:   35  37   1507  1593  11.232108  11.605071  11.421352

24:   38  40   1637  1723  11.783474  12.125139  11.956764
25:   41  43   1766  1852  12.288791  12.602659  12.447904     8:  10  12   1723  2067  12.125139  13.316883    12.753228
26:   44  46   1895  1981  12.753228  13.042468  12.899777

27:   47  49   2024  2110  13.181453  13.448898  13.316883
28:   50  53   2153  2283  13.577635  13.945465  13.764881     9:  12  14   2067  2412  13.316883  14.288198    13.825796
29:   54  57   2326  2455  14.062349  14.397371  14.232693

30:   58  61   2498  2627  14.504172  14.811258  14.660130
31:   62  66   2670  2842  14.909464  15.283564  15.100115    10:  15  18   2584  3101  14.711029  15.795819    15.283564
32:   67  71   2885  3058  15.372757  15.714074  15.546390

33:   72  77   3101  3316  15.795819  16.185532  15.994471
34:   78  83   3359  3575  16.259980  16.616871  16.441494    11:  18  23   3101  3962  15.795819  17.204658    16.547424
35:   84  90   3618  3876  16.685418  17.079349  16.885941

36:   91  97   3919  4177  17.142352  17.506445  17.327264
37:   98 105   4221  4522  17.564981  17.959646  17.765487    12:  23  29   3962  4996  17.204658  18.533945    17.904788
38:  106 114   4565  4910  18.014031  18.433233  18.227034

39:  115 123   4953  5297  18.483782  18.874805  18.682185
40:  124 133   5340  5728  18.922095  19.332992  19.130789    13:  29  36   4996  6202  18.533945  19.801451    19.198897
41:  134 144   5771  6202  19.377073  19.801451  19.592946

42:  145 156   6245  6718  19.842285  20.272889  20.061808
43:  157 169   6761  7278  20.310373  20.739167  20.529583    14:  36  46   6202  7924  19.801451  21.222342    20.565177
44:  170 183   7321  7881  20.773175  21.191895  20.987911

45:  184 198   7924  8527  21.222342  21.623344  21.428652
46:  199 215   8570  9259  21.650236  22.050787  21.857360    15:  46  58   7924  9991  21.222342  22.420001    21.882271
47:  216 233   9302 10034  22.074042  22.440072  22.263795

48-  234 253  10078 10896  22.459969  22.807140  22.640652
49:  254 275  10939 11843  22.823891  23.144847  22.991444    16:  59  75  10164 12920  22.499251  23.461146    23.044078
50:  276 300  11886 12920  23.158772  23.461146  23.317264

51:  301 328  12963 14126  23.472530  23.748999  23.617861
52:  329 359  14169 15461  23.758199  24.005540  23.888450    17:  75  99  12920 17054  23.461146  24.248491    23.920884
53:  360 395  15504 17011  24.012922  24.242660  24.134368

54:  396 436  17054 18777  24.248491  24.454928  24.357873
55:  437 484  18820 20844  24.459492  24.647977  24.559711    18:  99 127  17054 21878  24.248491  24.727775    24.524955
56:  485 511  20887 22007  24.651498  24.737100  24.695685
*/


/* V A R I A B L E S */
float  MinVal   [PART_LONG];               // contains minimum tonality soffsets
float  Loudness [PART_LONG];               // weighting factors for loudness calculation
float  SPRD     [PART_LONG] [PART_LONG];   // tabulated spreading function
float  O_MAX;
float  O_MIN;
float  FAC1;
float  FAC2;                               // constants for offset calculation
float  partLtq  [PART_LONG];               // threshold in quiet (partitions)
float  invLtq   [PART_LONG];               // inverse threshold in quiet (partitions, long)
float  fftLtq   [512];                     // threshold in quiet (FFT)
float  Ltq_offset;                         // Offset for threshold in quiet
float  Ltq_max;                            // maximum level for threshold in quiet
float  TMN;
float  NMT;
float  TransDetect;
unsigned int    EarModelFlag;
int    MinValChoice;


/*
 *  Klemm 1994 and 1997. Experimental data. Sorry, data looks a little bit
 *  dodderly. Data below 30 Hz is extrapolated from other material, above 18
 *  kHz the ATH is limited due to the original purpose (too much noise at
 *  ATH is not good even if it's theoretically inaudible).
 */

static float
ATHformula_Frank ( float freq )
{
    /*
     * one value per 100 cent = 1
     * semitone = 1/4
     * third = 1/12
     * octave = 1/40 decade
     * rest is linear interpolated, values are currently in millibel rel. 20 µPa
     */
    static short tab [] = {
        /*    10.0 */  9669, 9669, 9626, 9512,
        /*    12.6 */  9353, 9113, 8882, 8676,
        /*    15.8 */  8469, 8243, 7997, 7748,
        /*    20.0 */  7492, 7239, 7000, 6762,
        /*    25.1 */  6529, 6302, 6084, 5900,
        /*    31.6 */  5717, 5534, 5351, 5167,
        /*    39.8 */  5004, 4812, 4638, 4466,
        /*    50.1 */  4310, 4173, 4050, 3922,
        /*    63.1 */  3723, 3577, 3451, 3281,
        /*    79.4 */  3132, 3036, 2902, 2760,
        /*   100.0 */  2658, 2591, 2441, 2301,
        /*   125.9 */  2212, 2125, 2018, 1900,
        /*   158.5 */  1770, 1682, 1594, 1512,
        /*   199.5 */  1430, 1341, 1260, 1198,
        /*   251.2 */  1136, 1057,  998,  943,
        /*   316.2 */   887,  846,  744,  712,
        /*   398.1 */   693,  668,  637,  606,
        /*   501.2 */   580,  555,  529,  502,
        /*   631.0 */   475,  448,  422,  398,
        /*   794.3 */   375,  351,  327,  322,
        /*  1000.0 */   312,  301,  291,  268,
        /*  1258.9 */   246,  215,  182,  146,
        /*  1584.9 */   107,   61,   13,  -35,
        /*  1995.3 */   -96, -156, -179, -235,
        /*  2511.9 */  -295, -350, -401, -421,
        /*  3162.3 */  -446, -499, -532, -535,
        /*  3981.1 */  -513, -476, -431, -313,
        /*  5011.9 */  -179,    8,  203,  403,
        /*  6309.6 */   580,  736,  881, 1022,
        /*  7943.3 */  1154, 1251, 1348, 1421,
        /* 10000.0 */  1479, 1399, 1285, 1193,
        /* 12589.3 */  1287, 1519, 1914, 2369,
#if 0
        /* 15848.9 */  3352, 4865, 5942, 6177,
        /* 19952.6 */  6385, 6604, 6833, 7009,
        /* 25118.9 */  7066, 7127, 7191, 7260,
#else
        /* 15848.9 */  3352, 4352, 5352, 6352,
        /* 19952.6 */  7352, 8352, 9352, 9999,
        /* 25118.9 */  9999, 9999, 9999, 9999,
#endif
    };
    double    freq_log;
    unsigned  index;

    if ( freq <    10. ) freq =    10.;
    if ( freq > 29853. ) freq = 29853.;

    freq_log = 40. * log10 (0.1 * freq);   /* 4 steps per third, starting at 10 Hz */
    index    = (unsigned) freq_log;
    return 0.01 * (tab [index] * (1 + index - freq_log) + tab [index+1] * (freq_log - index));
}


/* F U N C T I O N S */
// calculation of the threshold in quiet in FFT-resolution
static void
Ruhehoerschwelle ( unsigned int  EarModelFlag,
                   int           Ltq_offset,
                   int           Ltq_max )
{
    int     n;
    int     k;
    float   f;
    float   erg;
    double  tmp;
    float   absLtq [512];

    for ( n = 0; n < 512; n++ ) {
        f = (float) ( (n+1) * (float)(SampleFreq / 2000.) / 512 );   // Frequency in kHz

        switch ( EarModelFlag / 100 ) {
        case 0:         // ISO-threshold in quiet
            tmp  = 3.64*pow (f,-0.8) -  6.5*exp (-0.6*(f-3.3)*(f-3.3)) + 0.001*pow (f, 4.0);
            break;
        default:
        case 1:         // measured threshold in quiet (Nick Berglmeir, Andree Buschmann, Kopfhörer)
            tmp  = 3.00*pow (f,-0.8) -  5.0*exp (-0.1*(f-3.0)*(f-3.0)) + 0.0000015022693846297*pow (f, 6.0) + 10.*exp (-(f-0.1)*(f-0.1));
            break;
        case 2:         // measured threshold in quiet (Filburt, Kopfhörer)
            tmp  = 9.00*pow (f,-0.5) - 15.0*exp (-0.1*(f-4.0)*(f-4.0)) + 0.0341796875*pow (f, 2.5)          + 15.*exp (-(f-0.1)*(f-0.1)) - 18;
            tmp  = mind ( tmp, Ltq_max - 18 );
            break;
        case 3:
            tmp  = ATHformula_Frank ( 1.e3 * f );
            break;
        case 4:
            tmp  = ATHformula_Frank ( 1.e3 * f );
            if ( f > 4.8 ) {
                tmp += 3.00*pow (f,-0.8) -  5.0*exp (-0.1*(f-3.0)*(f-3.0)) + 0.0000015022693846297*pow (f, 6.0) + 10.*exp (-(f-0.1)*(f-0.1));
                tmp *= 0.5 ;
            }
            break;
        case 5:
            tmp  = ATHformula_Frank ( 1.e3 * f );
            if ( f > 4.8 ) {
                tmp = 3.00*pow (f,-0.8) -  5.0*exp (-0.1*(f-3.0)*(f-3.0)) + 0.0000015022693846297*pow (f, 6.0) + 10.*exp (-(f-0.1)*(f-0.1));
            }
            break;
        }

        tmp -= f * f * (int)(EarModelFlag % 100 - 50) * 0.0015;  // 00: +30 dB, 100: -30 dB  @20 kHz

        tmp       = mind ( tmp, Ltq_max );              // Limit ATH
        tmp      += Ltq_offset - 23;                    // Add chosen Offset
        fftLtq[n] = absLtq[n] = POW10 ( 0.1 * tmp);     // conversion into power
    }

    // threshold in quiet in partitions (long)
    for ( n = 0; n < PART_LONG; n++ ) {
        erg = 1.e20f;
        for ( k = wl[n]; k <= wh[n]; k++ )
            erg = minf (erg, absLtq[k]);

        partLtq[n] = erg;               // threshold in quiet
        invLtq [n] = 1.f / partLtq[n];  // Inverse
    }
}

#ifdef _MSC_VER
static double
asinh ( double x )
{
    return x >= 0  ?  log (sqrt (x*x+1) + x)  :  -log (sqrt (x*x+1) - x);
}
#endif


static double
Freq2Bark ( double Hz )           // Klemm 2002
{
    return 9.97074*asinh (1.1268e-3 * Hz) - 6.25817*asinh (0.197193e-3 * Hz) ;
}

static double
Bark2Freq ( double Bark )           // Klemm 2002
{
    return 956.86 * sinh (0.101561*Bark) + 11.7296 * sinh (0.304992*Bark) + 6.33622e-3*sinh (0.538621*Bark);
}

static double
LongPart2Bark ( int Part )
{
    return Freq2Bark ((wl [Part] + wh [Part]) * SampleFreq / 2048.);
}

// calculating the table for loudness calculation based on absLtq = ank
static void
Loudness_Tabelle (void)
{
    int    n;
    float  midfreq;
    float  tmp;

    // ca. dB(A)
    for ( n = 0; n < PART_LONG; n++ ){
        midfreq      = (wh[n] + wl[n] + 3) * (0.25 * SampleFreq / 512);     // center frequency in kHz, why +3 ???
        tmp          = LOG10 (midfreq) - 3.5f;                                  // dB(A)
        tmp          = -10 * tmp * tmp + 3 - midfreq/3000;
        Loudness [n] = POW10 ( 0.1 * tmp );                                     // conversion into power
    }
}


static double
Bass ( float f, float TMN, float NMT, float bass )
{
    static unsigned char  lfe [11] = { 120, 100, 80, 60, 50, 40, 30, 20, 15, 10, 5 };
    int                   tmp      = (int) ( 1024/44100. * f + 0.5 );

    switch ( tmp ) {
    case  0:
    case  1:
    case  2:
    case  3:
    case  4:
    case  5:
    case  6:
    case  7:
    case  8:
    case  9:
    case 10:
        return TMN + bass * lfe [tmp];
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
    case 16:
    case 17:
    case 18:
        return TMN;
    case 19:
    case 20:
    case 21:
    case 22:
        return TMN*0.75 + NMT*0.25;
    case 23:
    case 24:
        return TMN*0.50 + NMT*0.50;
    case 25:
    case 26:
        return TMN*0.25 + NMT*0.75;
    default:
        return NMT;
    }
}


// calculating the coefficient for utilization of the tonality offset, depending on TMN und NMT
static void
Tonalitaetskoeffizienten ( void )
{
    double                tmp;
    int                   n;
    float                 bass;

    bass = 0.1/8 * NMT;
    if ( MinValChoice <= 2  &&  bass > 0.1 )
        bass = 0.1f;
    if ( MinValChoice <= 1 )
        bass = 0.0f;

    // alternative: calculation of the minval-values dependent on TMN and TMN
    for ( n = 0; n < PART_LONG; n++ ) {
        tmp        = Bass ( (wl [n] + wh [n]) / 2048. * SampleFreq, TMN, NMT, bass );
        MinVal [n] = POW10 ( -0.1 * tmp );                      // conversion into power
    }

    // calculation of the constants for "tonality offset"
    O_MAX = POW10 ( -0.1 * TMN );
    O_MIN = POW10 ( -0.1 * NMT );
    FAC1  = POW10 ( -0.1 * (NMT - (TMN - NMT) * 0.229) ) ;
    FAC2  = (TMN - NMT) * (0.99011159 * 0.1);
}


// calculation of the spreading function
static void
Spread ( void )
{
    int    i;
    int    j;
    float  tmpx;
    float  tmpy;
    float  tmpz;
    float  x;

    // calculation of the spreading-function for all occuring values
    for ( i = 0; i < PART_LONG; i++ ) {                 // i is masking Partition, Source
        for ( j = 0; j < PART_LONG; j++ ) {             // j is masking Partition, Target
            tmpx = LongPart2Bark (j) - LongPart2Bark (i);// Difference of the partitions in Bark
            tmpy = tmpz = 0.;                           // tmpz = 0: no dip

            if      ( tmpx < 0 ) {                      // downwards (S1)
                tmpy  = -32.f * tmpx;                   // 32 dB per Bark, e33 (10)
            }
            else if ( tmpx > 0 ) {                      // upwards (S2)
#if 0
                x = (wl[i]+wh[i])/2 * (float)(SampleFreq / 2000)/512;   // center frequency in kHz ???????
                if (i==0) x = 0.5f  * (float)(SampleFreq / 2000)/512;   // if first spectral line
#else
                x  = i  ?  wl[i]+wh[i]  :  1;
                x *= SampleFreq / 1000. / 2048;         // center frequency in kHz
#endif
                // dB/Bark
                tmpy = (22.f + 0.23f / x) * tmpx;       // e33 (10)

                // dip (up to 6 dB)
                tmpz = 8 * minf ( (tmpx-0.5f) * (tmpx-0.5f) - 2 * (tmpx-0.5f), 0.f );
            }

            // calculate coefficient
            SPRD[i][j] = POW10 ( -0.1 * (tmpy+tmpz) );  // [Source] [Target]
        }
    }

    // Normierung e33 (10)
    for ( i = 0; i < PART_LONG; i++ ) {                 // i is masked Partition
        float  norm = 0.f;
        for ( j = 0; j < PART_LONG; j++ )               // j is masking Partition
            norm += SPRD [j] [i];
        for ( j = 0; j < PART_LONG; j++ )               // j is masking Partition
            SPRD [j] [i] /= norm;
    }
}

// call all initialisation procedures
void
Init_Psychoakustiktabellen ( void )
{
    Max_Band = (int) ( Bandwidth * 64. / SampleFreq );
    if ( Max_Band <  1 ) Max_Band =  1;
    if ( Max_Band > 31 ) Max_Band = 31;

    Tonalitaetskoeffizienten ();
    Ruhehoerschwelle ( EarModelFlag, Ltq_offset, Ltq_max );
    Loudness_Tabelle ();
    Spread ();
}

/* end of psy_tab.c */
