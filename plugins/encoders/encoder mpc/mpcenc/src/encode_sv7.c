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

/*
 *  SV1:   DATE 13.12.1998
 *  SV2:   DATE 12.06.1999
 *  SV3:   DATE 19.10.1999
 *  SV4:   DATE 20.10.1999
 *  SV5:   DATE 18.06.2000
 *  SV6:   DATE 10.08.2000
 *  SV7:   DATE 23.08.2000
 *  SV7.f: DATE 20.07.2002
 */

unsigned char         MS_Flag     [32];         // Flag to save if Subband was MS- or LR-coded
int                   SCF_Last_L  [32];
int                   SCF_Last_R  [32];         // Last coded SCF value
static unsigned char  DSCF_RLL_L  [32];
static unsigned char  DSCF_RLL_R  [32];         // Duration of the differential SCF-coding for RLL (run length limitation)
int                   Res_L       [32];
int                   Res_R       [32];         // Quantization precision of the subbands
int                   SCF_Index_L [32] [3];
int                   SCF_Index_R [32] [3];     // Scalefactor index for quantized subband values


// initialize SV7
void
Init_SV7 ( void )
{
    Init_Huffman_Encoder_SV7 ();
}


// writes SV7-header
void
WriteHeader_SV7 ( const unsigned int  MaxBand,
                  const unsigned int  Profile,
                  const unsigned int  MS_on,
                  const Uint32_t      TotalFrames,
                  const unsigned int  SamplesRest,
                  const unsigned int  StreamVersion,
                  const unsigned int  SampleFreq )
{
    WriteBits ( StreamVersion,  8 );    // StreamVersion
    WriteBits ( 0x2B504D     , 24 );    // Magic Number "MP+"

    WriteBits ( TotalFrames  , 32 );    // # of frames

    WriteBits ( 0            ,  1 );    // former IS-Flag (not supported anymore)
    WriteBits ( MS_on        ,  1 );    // MS-Coding Flag
    WriteBits ( MaxBand      ,  6 );    // Bandwidth

#if 0
    if ( MPPENC_VERSION [3] & 1 )
        WriteBits ( 1        ,  4 );    // 1: Experimental profile
    else
#endif

        WriteBits ( Profile  ,  4 );    // 5...15: below Telephone...above BrainDead
    WriteBits ( 0            ,  2 );    // for future use
    switch ( SampleFreq ) {
        case 44100: WriteBits ( 0, 2 ); break;
        case 48000: WriteBits ( 1, 2 ); break;
        case 37800: WriteBits ( 2, 2 ); break;
        case 32000: WriteBits ( 3, 2 ); break;
        default   : stderr_printf ( "Internal error\n");
                    exit (1);
    }
    WriteBits ( 0            , 16 );    // maximum input sample value, currently filled by replaygain

    WriteBits ( 0            , 32 );    // title based gain controls, currently filled by replaygain

    WriteBits ( 0            , 32 );    // album based gain controls, currently filled by replaygain

    WriteBits ( 1            ,  1 );    // true gapless: used?
    WriteBits ( SamplesRest  , 11 );    // true gapless: valid samples in last frame
    WriteBits ( 1            , 1 );     // we now support fast seeking
    WriteBits ( 0            , 19 );

    WriteBits ( (MPPENC_VERSION[0]&15)*100 + (MPPENC_VERSION[2]&15)*10 + (MPPENC_VERSION[3]&15),
                                8 );    // for future use
}


void
FinishBitstream ( void )
{
    Buffer [Zaehler++] = dword;         // Assigning the "last" word
}


#define ENCODE_SCF1( new, old, rll )                         \
        d = new - old + 7;                                   \
        if ( d <= 14u  && rll < 32) {                        \
            WriteBits ( Table[d].Code, Table[d].Length );    \
        }                                                    \
        else {                                               \
            if ( new < 0 ) new = 0, Overflows++;             \
            WriteBits ( Table[15].Code, Table[15].Length );  \
            WriteBits ( (unsigned int)new, 6 );              \
            rll = 0;                                         \
        }

#define ENCODE_SCFn( new, old, rll )                         \
        d = new - old + 7;                                   \
        if ( d <= 14u ) {                                    \
            WriteBits ( Table[d].Code, Table[d].Length );    \
        }                                                    \
        else {                                               \
            if ( new < 0 ) new = 0, Overflows++;             \
            WriteBits ( Table[15].Code, Table[15].Length );  \
            WriteBits ( (unsigned int)new, 6 );              \
            rll = 0;                                         \
        }


static void
test ( const int* const Res, const unsigned int* q )
{
#if 0
    int  i;

    switch ( *Res ) {
    case 1:
        for ( i = 0; i < 36; i ++ )
            if ( q[i] != 1 )
                return;
        fprintf ( stderr, "Alles Nullsamples, aber Auflösung = %u\n", *Res );
        *Res = 0;
        break;
    case 2:
        for ( i = 0; i < 36; i ++ )
            if ( q[i] != 2 )
                return;
        fprintf ( stderr, "Alles Nullsamples, aber Auflösung = %u\n", *Res );
        *Res = 0;
        break;
    }
#endif
}


// formatting and writing SV7-bitstream for one frame
void
WriteBitstream_SV7 ( const int               MaxBand,
                     const SubbandQuantTyp*  Q )
{
    int                  n;
    int                  k;
    unsigned int         d;
    unsigned int         idx;
    unsigned int         book;
    const Huffman_t*     Table;
    const Huffman_t*     Table0;
    const Huffman_t*     Table1;
    int                  sum;
    const unsigned int*  q;
    unsigned char        SCFI_L [32];
    unsigned char        SCFI_R [32];

    ENTER(110);

    /************************************ Resolution *********************************/
    WriteBits ( (unsigned int)Res_L[0], 4 );                            // subband 0
    WriteBits ( (unsigned int)Res_R[0], 4 );
    if ( MS_Channelmode > 0  &&  !(Res_L[0]==0  &&  Res_R[0]==0) )
         WriteBits ( MS_Flag[0] , 1 );

    Table = HuffHdr;                                                    // subband 1...MaxBand
    for ( n = 1; n <= MaxBand; n++ ) {
        test ( Res_L+n, Q[n].L );

        d = Res_L[n] - Res_L[n-1] + 5;
        if ( d <= 8u ) {
            WriteBits ( Table[d].Code, Table[d].Length );
        }
        else {
            WriteBits ( Table[9].Code, Table[9].Length );
            WriteBits ( Res_L[n]     , 4               );
        }

        test ( Res_R+n, Q[n].R );
        d = Res_R[n] - Res_R[n-1] + 5;
        if ( d <= 8u ) {
            WriteBits ( Table[d].Code, Table[d].Length );
        }
        else {
            WriteBits ( Table[9].Code, Table[9].Length );
            WriteBits ( Res_R[n]     , 4               );
        }
        if ( MS_Channelmode > 0  &&  !(Res_L[n]==0 && Res_R[n]==0) )
            WriteBits ( MS_Flag[n], 1 );
    }

    /************************************ SCF encoding type ***********************************/
    Table = HuffSCFI;
    for ( n = 0; n <= MaxBand; n++ ) {
        if ( Res_L[n] ) {
            SCFI_L[n] = 2 * (SCF_Index_L[n][0] == SCF_Index_L[n][1]) + (SCF_Index_L[n][1] == SCF_Index_L[n][2]);
            WriteBits ( Table[SCFI_L[n]].Code, Table[SCFI_L[n]].Length );
        }
        if ( Res_R[n] ) {
            SCFI_R[n] = 2 * (SCF_Index_R[n][0] == SCF_Index_R[n][1]) + (SCF_Index_R[n][1] == SCF_Index_R[n][2]);
            WriteBits ( Table[SCFI_R[n]].Code, Table[SCFI_R[n]].Length );
        }
    }

    /************************************* SCF **********************************/
    Table = HuffDSCF;
    for ( n = 0; n <= MaxBand; n++ ) {

        if ( Res_L[n] ) {
            switch ( SCFI_L[n] ) {
            default:
                ENCODE_SCF1 ( SCF_Index_L[n][0], SCF_Last_L [n]   , DSCF_RLL_L[n] );
                ENCODE_SCFn ( SCF_Index_L[n][1], SCF_Index_L[n][0], DSCF_RLL_L[n] );
                ENCODE_SCFn ( SCF_Index_L[n][2], SCF_Index_L[n][1], DSCF_RLL_L[n] );
                SCF_Last_L[n] = SCF_Index_L[n][2];
                break;
            case 1:
                ENCODE_SCF1 ( SCF_Index_L[n][0], SCF_Last_L [n]   , DSCF_RLL_L[n] );
                ENCODE_SCFn ( SCF_Index_L[n][1], SCF_Index_L[n][0], DSCF_RLL_L[n] );
                SCF_Last_L[n] = SCF_Index_L[n][1];
                break;
            case 2:
                ENCODE_SCF1 ( SCF_Index_L[n][0], SCF_Last_L[n]    , DSCF_RLL_L[n] );
                ENCODE_SCFn ( SCF_Index_L[n][2], SCF_Index_L[n][0], DSCF_RLL_L[n] );
                SCF_Last_L[n] = SCF_Index_L[n][2];
                break;
            case 3:
                ENCODE_SCF1 ( SCF_Index_L[n][0], SCF_Last_L[n]    , DSCF_RLL_L[n] );
                SCF_Last_L[n] = SCF_Index_L[n][0];
                break;
            }
        }
        if (DSCF_RLL_L[n] <= 32)
            DSCF_RLL_L[n]++;        // Increased counters for SCF that haven't been initialized again

        if ( Res_R[n] ) {
            switch ( SCFI_R[n] ) {
            default:
                ENCODE_SCF1 ( SCF_Index_R[n][0], SCF_Last_R[n]    , DSCF_RLL_R[n] );
                ENCODE_SCFn ( SCF_Index_R[n][1], SCF_Index_R[n][0], DSCF_RLL_R[n] );
                ENCODE_SCFn ( SCF_Index_R[n][2], SCF_Index_R[n][1], DSCF_RLL_R[n] );
                SCF_Last_R[n] = SCF_Index_R[n][2];
                break;
            case 1:
                ENCODE_SCF1 ( SCF_Index_R[n][0], SCF_Last_R[n]    , DSCF_RLL_R[n] );
                ENCODE_SCFn ( SCF_Index_R[n][1], SCF_Index_R[n][0], DSCF_RLL_R[n] );
                SCF_Last_R[n] = SCF_Index_R[n][1];
                break;
            case 2:
                ENCODE_SCF1 ( SCF_Index_R[n][0], SCF_Last_R[n]    , DSCF_RLL_R[n] );
                ENCODE_SCFn ( SCF_Index_R[n][2], SCF_Index_R[n][0], DSCF_RLL_R[n] );
                SCF_Last_R[n] = SCF_Index_R[n][2];
                break;
            case 3:
                ENCODE_SCF1 ( SCF_Index_R[n][0], SCF_Last_R[n]    , DSCF_RLL_R[n] );
                SCF_Last_R[n] = SCF_Index_R[n][0];
                break;
            }
        }
        if (DSCF_RLL_R[n] <= 32)
            DSCF_RLL_R[n]++;          // Increased counters for SCF that haven't been freshly initialized
    }

    /*********************************** Samples *********************************/
    for ( n = 0; n <= MaxBand; n++ ) {

        sum = 0;
        q   = Q[n].L;

        switch ( Res_L[n] ) {
        case -1:
        case  0:
            break;
        case  1:
            Table0 = HuffQ [0][1];
            Table1 = HuffQ [1][1];
            for ( k = 0; k < 36; k += 3 ) {
                idx  = q[k+0] + 3*q[k+1] + 9*q[k+2];
                sum += Table0 [idx].Length;
                sum -= Table1 [idx].Length;
            }
            book = sum >= 0;
            WriteBits ( book, 1 );
            Table = HuffQ [book][1];
            for ( k = 0; k < 36; k += 3 ) {
                idx = q[k+0] + 3*q[k+1] + 9*q[k+2];
                WriteBits ( Table[idx].Code, Table[idx].Length );
            }
            break;
        case  2:
            Table0 = HuffQ [0][2];
            Table1 = HuffQ [1][2];
            for ( k = 0; k < 36; k += 2 ) {
                idx  = q[k+0] + 5*q[k+1];
                sum += Table0 [idx].Length;
                sum -= Table1 [idx].Length;
            }
            book = sum >= 0;
            WriteBits ( book, 1 );
            Table = HuffQ [book][2];
            for ( k = 0; k < 36; k += 2 ) {
                idx = q[k+0] + 5*q[k+1];
                WriteBits ( Table[idx].Code, Table[idx].Length );
            }
            break;
        case  3:
        case  4:
        case  5:
        case  6:
        case  7:
            Table0 = HuffQ [0][Res_L[n]];
            Table1 = HuffQ [1][Res_L[n]];
            for ( k = 0; k < 36; k++ ) {
                sum += Table0 [q[k]].Length;
                sum -= Table1 [q[k]].Length;
            }
            book = sum >= 0;
            WriteBits ( book, 1 );
            Table = HuffQ [book][Res_L[n]];
            for ( k = 0; k < 36; k++ ) {
                idx = q[k];
                WriteBits ( Table[idx].Code, Table[idx].Length );
            }
            break;
        default:
            for ( k = 0; k < 36; k++ )
                WriteBits ( q[k], Res_L[n]-1 );
            break;
        }

        sum = 0;
        q   = Q[n].R;

        switch ( Res_R[n] ) {
        case -1:
        case  0:
            break;
        case  1:
            Table0 = HuffQ [0][1];
            Table1 = HuffQ [1][1];
            for ( k = 0; k < 36; k += 3 ) {
                idx  = q[k+0] + 3*q[k+1] + 9*q[k+2];
                sum += Table0 [idx].Length;
                sum -= Table1 [idx].Length;
            }
            book = sum >= 0;
            WriteBits ( book, 1 );
            Table = HuffQ [book][1];
            for ( k = 0; k < 36; k += 3 ) {
                idx = q[k+0] + 3*q[k+1] + 9*q[k+2];
                WriteBits ( Table[idx].Code, Table[idx].Length );
            }
            break;
        case  2:
            Table0 = HuffQ [0][2];
            Table1 = HuffQ [1][2];
            for ( k = 0; k < 36; k += 2 ) {
                idx  = q[k+0] + 5*q[k+1];
                sum += Table0 [idx].Length;
                sum -= Table1 [idx].Length;
            }
            book = sum >= 0;
            WriteBits ( book, 1 );
            Table = HuffQ [book][2];
            for ( k = 0; k < 36; k += 2 ) {
                idx = q[k+0] + 5*q[k+1];
                WriteBits ( Table[idx].Code, Table[idx].Length );
            }
            break;
        case  3:
        case  4:
        case  5:
        case  6:
        case  7:
            Table0 = HuffQ [0][Res_R[n]];
            Table1 = HuffQ [1][Res_R[n]];
            for ( k = 0; k < 36; k++ ) {
                sum += Table0 [q[k]].Length;
                sum -= Table1 [q[k]].Length;
            }
            book = sum >= 0;
            WriteBits ( book, 1 );
            Table = HuffQ [book][Res_R[n]];
            for ( k = 0; k < 36; k++ ) {
                idx = q[k];
                WriteBits ( Table[idx].Code, Table[idx].Length );
            }
            break;
        default:
            for ( k = 0; k < 36; k++ )
                WriteBits ( q[k], Res_R[n] - 1 );
            break;
        }

    }

    LEAVE(110);
    return;
}

#undef ENCODE_SCF1
#undef ENCODE_SCFn


#if 0
void
Dump ( const unsigned int* q, const int Res )
{
    switch ( Res ) {
    case  1:
        for ( k = 0; k < 36; k++, q++ )
            printf ("%2d%c", *q-1, k==35?'\n':' ');
        break;
    case  2:
        for ( k = 0; k < 36; k++, q++ )
            printf ("%2d%c", *q-2, k==35?'\n':' ');
        break;
    case  3: case  4: case  5: case  6: case  7:
        if ( Res == 5 )
            for ( k = 0; k < 36; k++, q++ )
                printf ("%2d%c", *q-7, k==35?'\n':' ');
        break;
    case  8: case  9: case 10: case 11: case 12: case 13: case 14: case 15: case 16: case 17:
        printf ("%2u: ", Res-1 );
        for ( k = 0; k < 36; k++, q++ ) {
            printf ("%6d", *q - (1 << (Res-2)) );
        }
        printf ("\n");
        break;
    }
}
#endif

/* end of encode_sv7.c */
