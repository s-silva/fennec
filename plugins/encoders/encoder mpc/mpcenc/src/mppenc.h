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

#ifndef MPPENC_MPPENC_H
#define MPPENC_MPPENC_H

#include "mppdec.h"
#include "minimax.h"

//#define IO_BUFFERING                          // activates IO-buffer (default: off)

#define WIN32_MESSAGES      1                   // support Windows-Messaging to Frontend

// analyse_filter.c
#define X_MEM            1152

// ans.c
#define MAX_NS_ORDER        6                   // maximum order of the Adaptive Noise Shaping Filter (IIR)
#define MAX_ANS_BANDS      16
#define MAX_ANS_LINES    (32 * MAX_ANS_BANDS)   // maximum number of noiseshaped FFT-lines
///////// 16 * MAX_ANS_BANDS not sufficient? //////////////////
#define MS2SPAT1             0.5f
#define MS2SPAT2             0.25f
#define MS2SPAT3             0.125f
#define MS2SPAT4             0.0625f

// bitstream.c
#define BUFFER_ALMOST_FULL  8192
#define BUFFER_FULL         (BUFFER_ALMOST_FULL + 4352)         // 34490 bit/frame  1320.3 kbps

// cvd.c
#define MAX_CVD_LINE      300                   // maximum FFT-Index for CVD
#define CVD_UNPRED          0.040f              // unpredictability (cw) for CVD-detected bins, e33 (04)
#define MIN_ANALYZED_IDX   12                   // maximum base-frequency = 44100/MIN_ANALYZED_IDX ^^^^^^
#define MED_ANALYZED_IDX   50                   // maximum base-frequency = 44100/MED_ANALYZED_IDX ^^^^^^
#define MAX_ANALYZED_IDX  900                   // minimum base-frequency = 44100/MAX_ANALYZED_IDX  (816 for Amnesia)

// mppenc.h
#define CENTER            448                   // offset for centering current data in Main-array
#define BLOCK            1152                   // blocksize
#define ANABUFFER    (BLOCK + CENTER)           // size of PCM-data array for analysis

// psy.c
#define SHORTFFT_OFFSET   168                   // fft-offset for short FFT's
#define PREFAC_LONG        10                   // preecho-factor for long partitions

// psy_tab.h
#define PART_LONG          57                   // number of partitions for long
#define PART_SHORT     (PART_LONG / 3)          // number of partitions for short
#define MAX_SPL            20                   // maximum assumed Sound Pressure Level

// quant.h
#define SCFfac              0.832980664785f     // = SCF[n-1]/SCF[n]

// wave_in.h


// fast but maybe more inaccurate, use if you need speed
#if defined(__GNUC__) && !defined(__APPLE__)
#  define SIN(x)      sinf ((float)(x))
#  define COS(x)      cosf ((float)(x))
#  define ATAN2(x,y)  atan2f ((float)(x), (float)(y))
#  define SQRT(x)     sqrtf ((float)(x))
#  define LOG(x)      logf ((float)(x))
#  define LOG10(x)    log10f ((float)(x))
#  define POW(x,y)    expf (logf(x) * (y))
#  define POW10(x)    expf (M_LN10 * (x))
#  define FLOOR(x)    floorf ((float)(x))
#  define IFLOOR(x)   (int) floorf ((float)(x))
#  define FABS(x)     fabsf ((float)(x))
#else
# define SIN(x)      (float) sin (x)
# define COS(x)      (float) cos (x)
# define ATAN2(x,y)  (float) atan2 (x, y)
# define SQRT(x)     (float) sqrt (x)
# define LOG(x)      (float) log (x)
# define LOG10(x)    (float) log10 (x)
# define POW(x,y)    (float) pow (x,y)
# define POW10(x)    (float) pow (10., (x))
# define FLOOR(x)    (float) floor (x)
# define IFLOOR(x)   (int)   floor (x)
# define FABS(x)     (float) fabs (x)
#endif

#define SQRTF(x)      SQRT (x)
#ifdef FAST_MATH
# define TABSTEP      64
# define COSF(x)      my_cos ((float)(x))
# define ATAN2F(x,y)  my_atan2 ((float)(x), (float)(y))
# define IFLOORF(x)   my_ifloor ((float)(x))
#else
# undef  TABSTEP
# define COSF(x)      COS (x)
# define ATAN2F(x,y)  ATAN2 (x,y)
# define IFLOORF(x)   IFLOOR (x)
#endif

typedef struct {
    float  L [ANABUFFER];
    float  R [ANABUFFER];
    float  M [ANABUFFER];
    float  S [ANABUFFER];
} PCMDataTyp;

typedef struct {
    float  L [36];
    float  R [36];
} SubbandFloatTyp;

typedef struct {
    unsigned int  L [36];
    unsigned int  R [36];
} SubbandQuantTyp;

typedef struct {
    float  L [32];
    float  R [32];
    float  M [32];
    float  S [32];
} SMRTyp;

typedef struct {
    FILE*         fp;                   // File pointer to read data
    Ulong         PCMOffset;            // File offset of PCM data
    long double   SampleFreq;           // Sample frequency in Hz
    Uint          BitsPerSample;        // used bits per sample, 8*BytesPerSample-7 <= BitsPerSample <= BytesPerSample
    Uint          BytesPerSample;       // allocated bytes per sample
    Uint          Channels;             // Number of channels, 1...8
    UintMax_t     PCMBytes;             // PCM Samples (in 8 bit units)
    UintMax_t     PCMSamples;           // PCM Samples per Channel
    Bool_t        raw;                  // raw: headerless format
} wave_t;

// analy_filter.c
void   Analyse_Filter(const PCMDataTyp*, SubbandFloatTyp*, const int);
void   Analyse_Init ( float Left, float Right, SubbandFloatTyp* out, const int MaxBand );

void   Klemm ( void );

// ans.c
extern unsigned int  NS_Order;                          // global Flag for Noise Shaping
extern unsigned int  NS_Order_L [32];
extern unsigned int  NS_Order_R [32];                   // order of the Adaptive Noiseshaping (0: off, 1...5: on)
extern float         FIR_L     [32] [MAX_NS_ORDER];
extern float         FIR_R     [32] [MAX_NS_ORDER];     // contains FIR-Filter for NoiseShaping
extern float         ANSspec_L [MAX_ANS_LINES];
extern float         ANSspec_R [MAX_ANS_LINES];         // L/R-masking threshold for ANS
extern float         ANSspec_M [MAX_ANS_LINES];
extern float         ANSspec_S [MAX_ANS_LINES];         // M/S-masking threshold for ANS

void   Init_ANS   ( void );
void   NS_Analyse ( const int, const unsigned char* MS, const SMRTyp, const int* Transient );


// bitstream.c
typedef struct {
    Uint32_t*     ptr;
    unsigned int  bit;
} BitstreamPos;


extern Uint32_t      Buffer [BUFFER_FULL];      // buffer for bitstream file (128 KB)
extern Uint32_t      dword;                     // 32 bit-Word for Bitstream-I/O
extern unsigned int  Zaehler;                   // position counter for processed bitstream word (32 bit)
extern UintMax_t     BufferedBits;              // counter for the number of written bits in the bitstream

void  FlushBitstream    ( FILE* fp, const Uint32_t* buffer, size_t words32bit );
void  UpdateHeader      ( FILE* fp, Uint32_t Frames, Uint ValidSamples );
void  WriteBits         ( const Uint32_t input, const unsigned int bits );
void  WriteBitsAt       ( const Uint32_t input, const unsigned int bits, const BitstreamPos pos );
void  GetBitstreamPos   ( BitstreamPos* const pos );

// cvd.c
int    CVD2048 ( const float*, int* );


// fastmath.c
void   Init_FastMath ( void );
extern const float  tabatan2   [] [2];
extern const float  tabcos     [] [2];
extern const float  tabsqrt_ex [];
extern const float  tabsqrt_m  [] [2];


// fft4g.c
void   rdft                ( const int, float*, int*, float* );
void   Generate_FFT_Tables ( const int, int*, float* );


// fft_routines.c
void   Init_FFT      ( void );
void   PowSpec256    ( const float*, float* );
void   PowSpec1024   ( const float*, float* );
void   PowSpec2048   ( const float*, float* );
void   PolarSpec1024 ( const float*, float*, float* );
void   Cepstrum2048  ( float* cep, const int );


// mppenc.c
extern float         SNR_comp_L [32];
extern float         SNR_comp_R [32];   // SNR-compensation after SCF-combination and ANS-gain
extern unsigned int  MS_Channelmode;    // global flag for enhanced functionality
extern unsigned int  Overflows;
extern float         SampleFreq;
extern float         Bandwidth;
extern float         KBD1;
extern float         KBD2;

// psy.c
extern unsigned int  CVD_used;          // global flag for ClearVoiceDetection (more switches for the psychoacoustic model)
extern float         varLtq;            // variable threshold in quiet
extern unsigned int  tmpMask_used;      // global flag for temporal masking
extern float         ShortThr;          // factor for calculation masking threshold with transients
extern float         minSMR;            // minimum SMR for all subbands

void   Init_Psychoakustik       ( void );
SMRTyp Psychoakustisches_Modell ( const int, const PCMDataTyp*, int* TransientL, int* TransientR );
void   TransientenCalc          ( int* Transient, const int* TransientL, const int* TransientR );
void   RaiseSMR                 ( const int, SMRTyp* );
void   MS_LR_Entscheidung       ( const int, unsigned char* MS, SMRTyp*, SubbandFloatTyp* );


// psy_tab.c
extern int          MinValChoice;               // Flag for calculation of MinVal-values
extern unsigned int EarModelFlag;               // Flag for threshold in quiet
extern float        Ltq_offset;                 // Offset for threshold in quiet
extern float        Ltq_max;                    // maximum level for threshold in quiet
extern float        fftLtq   [512];             // threshold in quiet (FFT)
extern float        partLtq  [PART_LONG];       // threshold in quiet (Partitions)
extern float        invLtq   [PART_LONG];       // inverse threshold in quiet (Partitions, long)
extern float        Loudness [PART_LONG];       // weighting factors for calculation of loudness
extern float        MinVal   [PART_LONG];       // minimum quality that's adapted to the model, minval for long
extern float        SPRD     [PART_LONG] [PART_LONG]; // tabulated spreading function
extern float        TMN;                        // Offset for purely sinusoid components
extern float        NMT;                        // Offset for purely noisy components
extern float        TransDetect;                // minimum slewrate for transient detection
extern float        O_MAX;
extern float        O_MIN;
extern float        FAC1;
extern float        FAC2;     // constants to calculate the used offset

extern const float  Butfly    [7];              // Antialiasing to calculate the subband powers
extern const float  InvButfly [7];              // Antialiasing to calculate the masking thresholds
extern const float  iw        [PART_LONG];      // inverse partition-width for long
extern const float  iw_short  [PART_SHORT];     // inverse partition-width for short
extern const int    wl        [PART_LONG];      // w_low  for long
extern const int    wl_short  [PART_SHORT];     // w_low  for short
extern const int    wh        [PART_LONG];      // w_high for long
extern const int    wh_short  [PART_SHORT];     // w_high for short

void   Init_Psychoakustiktabellen ( void );


// quant.c
extern float __invSCF [128 + 6];        // tabulated scalefactors (inverted)
#define invSCF  (__invSCF + 6)

void   Init_Skalenfaktoren             ( void );
float  ISNR_Schaetzer                  ( const float* samples, const float comp, const int res);
float  ISNR_Schaetzer_Trans            ( const float* samples, const float comp, const int res);
void   QuantizeSubband                 ( unsigned int* qu_output, const float* input, const int res, float* errors );
void   QuantizeSubbandWithNoiseShaping ( unsigned int* qu_output, const float* input, const int res, float* errors, const float* FIR );

void   NoiseInjectionComp ( void );


// encode_sv7.c
extern unsigned char  MS_Flag     [32];                  // subband-wise mid/side flag
extern int            Res_L       [32];
extern int            Res_R       [32];                  // resolution steps of the subbands
extern int            SCF_Index_L [32] [3];
extern int            SCF_Index_R [32] [3];              // Scalefactor-index for Bitstream

void         Init_SV7             ( void );
void         WriteHeader_SV7      ( const unsigned int, const unsigned int, const unsigned int, const Uint32_t TotalFrames, const unsigned int SamplesRest, const unsigned int StreamVersion, const unsigned int SampleFreq );
void         WriteBitstream_SV7   ( const int, const SubbandQuantTyp* );
void         FinishBitstream      ( void );


// huffsv7.c
extern Huffman_t         HuffHdr  [10];         // contains tables for SV7-header
extern Huffman_t         HuffSCFI [ 4];         // contains tables for SV7-scalefactor select
extern Huffman_t         HuffDSCF [16];         // contains tables for SV7-scalefactor coding
extern const Huffman_t*  HuffQ [2] [8];         // points to tables for SV7-sample coding

void    Huffman_SV7_Encoder ( void );


// keyboard.c
int    WaitKey      ( void );
int    CheckKeyKeep ( void );
int    CheckKey     ( void );


// regress.c
void    Regression       ( float* const _r, float* const _b, const float* p, const float* q );


// tags.c
void    Init_Tags        ( void );
int     FinalizeTags     ( FILE* fp, unsigned int Version );
int     addtag           ( const char* key, size_t keylen, const unsigned char* value, size_t valuelen, int converttoutf8, int flags );
int     gettag           ( const char* key, char* dst, size_t len );
int     CopyTags         ( const char* filename );


// wave_in.c
int     Open_WAV_Header  ( wave_t* type, const char* name );
size_t  Read_WAV_Samples ( wave_t* t, const size_t RequestedSamples, PCMDataTyp* data, const ptrdiff_t offset, const float scalel, const float scaler, int* Silence );
int     Read_WAV_Header  ( wave_t* type );


// winmsg.c
#ifdef _WIN32
int    SearchForFrontend   ( void );
void   SendQuitMessage     ( void );
void   SendModeMessage     ( const int );
void   SendStartupMessage  ( const char*, const int, const char* );
void   SendProgressMessage ( const int, const float, const float );
#else
# undef  WIN32_MESSAGES
# define WIN32_MESSAGES                 0
# define SearchForFrontend()            (0)
# define SendQuitMessage()              (void)0
# define SendModeMessage(x)             (void)0
# define SendStartupMessage(x,y,s)      (void)0
# define SendProgressMessage(x,y,z)     (void)0
#endif /* _WIN32 */


#define MPPENC_DENORMAL_FIX_BASE ( 32. * 1024. /* normalized sample value range */ / ( (float) (1 << 24 /* first bit below 32-bit PCM range */ ) ) )
#define MPPENC_DENORMAL_FIX_LEFT ( MPPENC_DENORMAL_FIX_BASE )
#define MPPENC_DENORMAL_FIX_RIGHT ( MPPENC_DENORMAL_FIX_BASE * 0.5f )


#endif /* MPPENC_MPPENC_H */

#if 0
# define LAST_HUFFMAN   15
# define DUMP_HIGHRES
#endif

#if 0
# define DUMP_RES15
#endif

#ifndef LAST_HUFFMAN
# define LAST_HUFFMAN    7
#endif

/* end of mppenc.h */
