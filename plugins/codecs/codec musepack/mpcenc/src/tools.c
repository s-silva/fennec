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

/*
 *  A list of different mixed tools
 *  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *  Read_LittleEndians()
 *      Portable file handling
 *  Requantize_MidSideStereo(), Requantize_IntensityStereo()
 *      Requantisation of quantized samples for synthesis filter
 *  Resort_HuffTable(), Make_HuffTable(), Make_LookupTable()
 *      Generating and sorting Huffman tables, making fast lookup tables
 */

#include <string.h>
#include <errno.h>
#include "mppdec.h"


#if defined HAVE_INCOMPLETE_READ  &&  FILEIO != 1

size_t
complete_read ( int fd, void* dest, size_t bytes )
{
    size_t  bytesread = 0;
    size_t  ret;

    while ( bytes > 0 ) {
#if defined _WIN32  &&  defined USE_HTTP  &&  !defined MPP_ENCODER
        ret = fd & 0x4000  ?  recv ( fd & 0x3FFF, dest, bytes, 0)  :  read ( fd, dest, bytes );
#else
        ret = read ( fd, dest, bytes );
#endif
        if ( ret == 0  ||  ret == (size_t)-1 )
            break;
        dest       = (void*)(((char*)dest) + ret);
        bytes     -= ret;
        bytesread += ret;
    }
    return bytesread;
}

#endif


int
isdir ( const char* Name )
{
#if FILEIO == 1
    return 1;
#else
    STRUCT_STAT  st;

    if ( STAT_CMD ( Name, &st ) != 0 )
        return 0;
    return S_ISDIR ( st.st_mode );
#endif
}


/*
 *  Change_Endian32() changes the endianess of a 32-bit memory block in-place
 *  by swapping the byte order. This is a little bit tricky, but a well
 *  known method which is much much faster than byte picking, especially on modern CPUs,
 *  because it avoids memory aliasing. Note that this method
 *  is poison for old 16-bit compilers!
 */

#if ENDIAN == HAVE_BIG_ENDIAN

static void
Change_Endian32 ( Uint32_t* dst, size_t words32bit )
{
    ENTER(160);

    for ( ; words32bit--; dst++ ) {
# if  INT_MAX >= 2147483647L
        Uint32_t  tmp = *dst;
        tmp  = ((tmp << 0x10) & 0xFFFF0000) | ((tmp >> 0x10) & 0x0000FFFF);
        tmp  = ((tmp << 0x08) & 0xFF00FF00) | ((tmp >> 0x08) & 0x00FF00FF);
        *dst = tmp;
# else
        Uint8_t  tmp;
        tmp                = ((Uint8_t*)dst)[0];
        ((Uint8_t*)dst)[0] = ((Uint8_t*)dst)[3];
        ((Uint8_t*)dst)[3] = tmp;
        tmp                = ((Uint8_t*)dst)[1];
        ((Uint8_t*)dst)[1] = ((Uint8_t*)dst)[2];
        ((Uint8_t*)dst)[2] = tmp;
# endif
    }
    LEAVE(160);
    return;
}

#endif /* ENDIAN == HAVE_BIG_ENDIAN */


/*
 *  Read_LittleEndians() reads little endian 32-bit ints from the stream
 *  'fp'.  Quantities are selected in 32-bit items. On big endian machines
 *  the byte order is changed in-place after reading the data, so all is
 *  okay.
 */

size_t
Read_LittleEndians ( FILE_T fp, Uint32_t* dst, size_t words32bit )
{
    size_t  wordsread;

    ENTER(161);
    wordsread = READ ( fp, dst, words32bit * sizeof(*dst) ) / sizeof(*dst);

#if ENDIAN == HAVE_BIG_ENDIAN
    Change_Endian32 ( dst, wordsread );
#endif

    LEAVE(161);
    return wordsread;
}

#ifndef MPP_ENCODER

/*
 *  This is the main requantisation routine which does the following things:
 *
 *      - rescaling the quantized values (int) to their original value (float)
 *      - recalculating both stereo channels for MS stereo
 *
 *  See also: Requantize_IntensityStereo()
 *
 *  For performance reasons all cases are programmed separately and the code
 *  is unrolled.
 *
 *  Input is:
 *      - Stop_Band:
 *          the last band using MS or LR stereo
 *      - used_MS[Band]:
 *          MS or LR stereo flag for every band (0...Stop_Band), Value is 1
 *          for MS and 0 for LR stereo.
 *      - Res[Band].{L,R}:
 *          Quantisation resolution for every band (0...Stop_Band) and
 *          channels (L, R). Value is 0...17.
 *      - SCF_Index[3][Band].{L,R}:
 *          Scale factor for every band (0...Stop_Band), subframe (0...2)
 *          and channel (L, R).
 *      - Q[Band].{L,R}[36]
 *          36 subband samples for every band (0...Stop_Band) and channel (L, R).
 *      - SCF[64], Cc[18], Dc[18]:
 *          Lookup tables for Scale factor and Quantisation resolution.
 *
 *   Output is:
 *     - Y_L:  Left  channel subband signals
 *     - Y_R:  Right channel subband signals
 *
 *   These signals are used for the synthesis filter in the synth*.[ch]
 *   files to generate the PCM output signal.
 */

static const float ISMatrix [32] [2] = {
    {  1.00000000f,  0.00000000f },
    {  0.98078528f,  0.19509032f },
    {  0.92387953f,  0.38268343f },
    {  0.83146961f,  0.55557023f },
    {  0.70710678f,  0.70710678f },
    {  0.55557023f,  0.83146961f },
    {  0.38268343f,  0.92387953f },
    {  0.19509032f,  0.98078528f },
    {  0.00000000f,  1.00000000f },
    { -0.19509032f,  0.98078528f },
    { -0.38268343f,  0.92387953f },
    { -0.55557023f,  0.83146961f },
    { -0.70710678f,  0.70710678f },
    { -0.83146961f,  0.55557023f },
    { -0.92387953f,  0.38268343f },
    { -0.98078528f,  0.19509032f },
    { -1.00000000f,  0.00000000f },
    { -0.98078528f, -0.19509032f },
    { -0.92387953f, -0.38268343f },
    { -0.83146961f, -0.55557023f },
    { -0.70710678f, -0.70710678f },
    { -0.55557023f, -0.83146961f },
    { -0.38268343f, -0.92387953f },
    { -0.19509032f, -0.98078528f },
    { -0.00000000f, -1.00000000f },
    {  0.19509032f, -0.98078528f },
    {  0.38268343f, -0.92387953f },
    {  0.55557023f, -0.83146961f },
    {  0.70710678f, -0.70710678f },
    {  0.83146961f, -0.55557023f },
    {  0.92387953f, -0.38268343f },
    {  0.98078528f, -0.19509032f },
};


void
Requantize_MidSideStereo ( Int Stop_Band, const Bool_t* used_MS )
{
    Int    Band;  // 0...Stop_Band
    Uint   k;     // 0...35
    Float  ML;
    Float  MR;
    Float  mid;
    Float  side;

    ENTER(162);

    for ( Band = 0; Band <= Stop_Band; Band++ ) {

        if ( used_MS[Band] )  // MidSide coded: left channel contains Mid signal, right channel Side signal
            if      ( Res[Band].R < -1 ) {
                k  = 0;
                ML = SCF[SCF_Index[0][Band].L] * Cc[Res[Band].L];
                do {
                    mid = Q[Band].L[k] * ML;
                    Y_L[k][Band] = mid * ISMatrix [used_MS[Band]][0];
                    Y_R[k][Band] = mid * ISMatrix [used_MS[Band]][1];
                } while (++k < 12);
                ML = SCF[SCF_Index[1][Band].L] * Cc[Res[Band].L];
                do {
                    mid = Q[Band].L[k] * ML;
                    Y_L[k][Band] = mid * ISMatrix [used_MS[Band]][0];
                    Y_R[k][Band] = mid * ISMatrix [used_MS[Band]][1];
                } while (++k < 24);
                ML = SCF[SCF_Index[2][Band].L] * Cc[Res[Band].L];
                do {
                    mid = Q[Band].L[k] * ML;
                    Y_L[k][Band] = mid * ISMatrix [used_MS[Band]][0];
                    Y_R[k][Band] = mid * ISMatrix [used_MS[Band]][1];
                } while (++k < 36);
            }
            else if ( Res[Band].L < -1 ) {
                k  = 0;
                ML = SCF[SCF_Index[0][Band].R] * Cc[Res[Band].R];
                do {
                    mid = Q[Band].R[k] * ML;
                    Y_R[k][Band] = mid * ISMatrix [used_MS[Band]][0];
                    Y_L[k][Band] = mid * ISMatrix [used_MS[Band]][1];
                } while (++k < 12);
                ML = SCF[SCF_Index[1][Band].R] * Cc[Res[Band].R];
                do {
                    mid = Q[Band].R[k] * ML;
                    Y_R[k][Band] = mid * ISMatrix [used_MS[Band]][0];
                    Y_L[k][Band] = mid * ISMatrix [used_MS[Band]][1];
                } while (++k < 24);
                ML = SCF[SCF_Index[2][Band].R] * Cc[Res[Band].R];
                do {
                    mid = Q[Band].R[k] * ML;
                    Y_R[k][Band] = mid * ISMatrix [used_MS[Band]][0];
                    Y_L[k][Band] = mid * ISMatrix [used_MS[Band]][1];
                } while (++k < 36);
            }
            else if ( Res[Band].L )
                if ( Res[Band].R ) {     //  M!=0, S!=0
                    k  = 0;
                    ML = SCF[SCF_Index[0][Band].L] * Cc[Res[Band].L];
                    MR = SCF[SCF_Index[0][Band].R] * Cc[Res[Band].R];
                    do {
                        Y_R[k][Band] = (mid = Q[Band].L[k] * ML) - (side = Q[Band].R[k] * MR);
                        Y_L[k][Band] = mid + side;
                    } while (++k < 12);
                    ML = SCF[SCF_Index[1][Band].L] * Cc[Res[Band].L];
                    MR = SCF[SCF_Index[1][Band].R] * Cc[Res[Band].R];
                    do {
                        Y_R[k][Band] = (mid = Q[Band].L[k] * ML) - (side = Q[Band].R[k] * MR);
                        Y_L[k][Band] = mid + side;
                    } while (++k < 24);
                    ML = SCF[SCF_Index[2][Band].L] * Cc[Res[Band].L];
                    MR = SCF[SCF_Index[2][Band].R] * Cc[Res[Band].R];
                    do {
                        Y_R[k][Band] = (mid = Q[Band].L[k] * ML) - (side = Q[Band].R[k] * MR);
                        Y_L[k][Band] = mid + side;
                    } while (++k < 36);
                } else {                 //  M!=0, S=0
                    k  = 0;
                    ML = SCF[SCF_Index[0][Band].L] * Cc[Res[Band].L];
                    do {
                        Y_R[k][Band] =
                        Y_L[k][Band] = Q[Band].L[k] * ML;
                    } while (++k < 12);
                    ML = SCF[SCF_Index[1][Band].L] * Cc[Res[Band].L];
                    do {
                        Y_R[k][Band] =
                        Y_L[k][Band] = Q[Band].L[k] * ML;
                    } while (++k < 24);
                    ML = SCF[SCF_Index[2][Band].L] * Cc[Res[Band].L];
                    do {
                        Y_R[k][Band] =
                        Y_L[k][Band] = Q[Band].L[k] * ML;
                    } while (++k < 36);
                }
            else
                if ( Res[Band].R ) {     //  M==0, S!=0
                    k  = 0;
                    ML = SCF[SCF_Index[0][Band].R] * Cc[Res[Band].R];
                    do {
                        Y_R[k][Band] = - (
                        Y_L[k][Band] = Q[Band].R[k] * ML );
                    } while (++k < 12);
                    ML = SCF[SCF_Index[1][Band].R] * Cc[Res[Band].R];
                    do {
                        Y_R[k][Band] = - (
                        Y_L[k][Band] = Q[Band].R[k] * ML );
                    } while (++k < 24);
                    ML = SCF[SCF_Index[2][Band].R] * Cc[Res[Band].R];
                    do {
                        Y_R[k][Band] = - (
                        Y_L[k][Band] = Q[Band].R[k] * ML );
                    } while (++k < 36);
                } else {                 //  M==0, S==0
                    for (k=0; k<36; k++) {
                        Y_R[k][Band] =
                        Y_L[k][Band] = 0.f;
                    }
                }

        else                  // Left/Right coded: left channel contains left, right the right signal

            if ( Res[Band].L )
                if ( Res[Band].R ) {     //  L!=0, R!=0
                    k  = 0;
                    ML = SCF[SCF_Index[0][Band].L] * Cc[Res[Band].L];
                    MR = SCF[SCF_Index[0][Band].R] * Cc[Res[Band].R];
                    do {
                        Y_R[k][Band] = Q[Band].R[k] * MR;
                        Y_L[k][Band] = Q[Band].L[k] * ML;
                    } while (++k < 12);
                    ML = SCF[SCF_Index[1][Band].L] * Cc[Res[Band].L];
                    MR = SCF[SCF_Index[1][Band].R] * Cc[Res[Band].R];
                    do {
                        Y_R[k][Band] = Q[Band].R[k] * MR;
                        Y_L[k][Band] = Q[Band].L[k] * ML;
                    } while (++k < 24);
                    ML = SCF[SCF_Index[2][Band].L] * Cc[Res[Band].L];
                    MR = SCF[SCF_Index[2][Band].R] * Cc[Res[Band].R];
                    do {
                        Y_R[k][Band] = Q[Band].R[k] * MR;
                        Y_L[k][Band] = Q[Band].L[k] * ML;
                    } while (++k < 36);
                } else {                 //  L!=0, R==0
                    k  = 0;
                    ML = SCF[SCF_Index[0][Band].L] * Cc[Res[Band].L];
                    do {
                        Y_R[k][Band] = 0.f;
                        Y_L[k][Band] = Q[Band].L[k] * ML;
                    } while (++k < 12);
                    ML = SCF[SCF_Index[1][Band].L] * Cc[Res[Band].L];
                    do {
                        Y_R[k][Band] = 0.f;
                        Y_L[k][Band] = Q[Band].L[k] * ML;
                    } while (++k < 24);
                    ML = SCF[SCF_Index[2][Band].L] * Cc[Res[Band].L];
                    do {
                        Y_R[k][Band] = 0.f;
                        Y_L[k][Band] = Q[Band].L[k] * ML;
                    } while (++k < 36);
                }
            else
                if ( Res[Band].R ) {     //  L==0, R!=0
                    k  = 0;
                    MR = SCF[SCF_Index[0][Band].R] * Cc[Res[Band].R];
                    do {
                        Y_R[k][Band] = Q[Band].R[k] * MR;
                        Y_L[k][Band] = 0.f;
                    } while (++k < 12);
                    MR = SCF[SCF_Index[1][Band].R] * Cc[Res[Band].R];
                    do {
                        Y_R[k][Band] = Q[Band].R[k] * MR;
                        Y_L[k][Band] = 0.f;
                    } while (++k < 24);
                    MR = SCF[SCF_Index[2][Band].R] * Cc[Res[Band].R];
                    do {
                        Y_R[k][Band] = Q[Band].R[k] * MR;
                        Y_L[k][Band] = 0.f;
                    } while (++k < 36);
                } else {                 //  L==0, R==0
                    for (k=0; k<36; k++) {
                        Y_R[k][Band] =
                        Y_L[k][Band] = 0.f;
                    }
                }

    }

    LEAVE(162);
    return;
}


/*
 *  This is the main requantisation routine for Intensity Stereo.
 *  It does the same as Requantize_MidSideStereo() but for IS.
 *
 *  Input is:
 *      - Stop_Band:
 *          the last band using MS or LR stereo
 *      - Res[Band].L:
 *          Quantisation resolution for every band (0...Stop_Band) and
 *          the left channel which is used for both channels. Value is 0...17.
 *      - SCF_Index[3][Band].{L,R}:
 *          Scale factor for every band (0...Stop_Band), subframe (0...2)
 *          and channel (L, R).
 *      - Q[Band].L[36]
 *          36 subband samples for every band (0...Stop_Band), both channels use
 *          the of the left channel
 *      - SCF[64], Cc[18], Dc[18]:
 *          Lookup tables for Scale factor and Quantisation resolution.
 *
 *   Output is:
 *     - Y_L:  Left  channel subband signals
 *     - Y_R:  Right channel subband signals
 *
 *   These signals are used for the synthesis filter in the synth*.[ch]
 *   files to generate the PCM output signal.
 */

void
Requantize_IntensityStereo ( Int Start_Band, Int Stop_Band )
{
    Int    Band;  // Start_Band...Stop_Band
    Uint   k;     // 0...35
    Float  ML;
    Float  MR;

    ENTER(163);

    for ( Band = Start_Band; Band <= Stop_Band; Band++ ) {

        if ( Res[Band].L ) {
            k  = 0;
            ML = SCF[SCF_Index[0][Band].L] * Cc[Res[Band].L] * SS05;
            MR = SCF[SCF_Index[0][Band].R] * Cc[Res[Band].L] * SS05;
            do {
                Y_R[k][Band] = Q[Band].L[k] * MR;
                Y_L[k][Band] = Q[Band].L[k] * ML;
            } while (++k < 12);
            ML = SCF[SCF_Index[1][Band].L] * Cc[Res[Band].L] * SS05;
            MR = SCF[SCF_Index[1][Band].R] * Cc[Res[Band].L] * SS05;
            do {
                Y_R[k][Band] = Q[Band].L[k] * MR;
                Y_L[k][Band] = Q[Band].L[k] * ML;
            } while (++k < 24);
            ML = SCF[SCF_Index[2][Band].L] * Cc[Res[Band].L] * SS05;
            MR = SCF[SCF_Index[2][Band].R] * Cc[Res[Band].L] * SS05;
            do {
                Y_R[k][Band] = Q[Band].L[k] * MR;
                Y_L[k][Band] = Q[Band].L[k] * ML;
            } while (++k < 36);
        } else {
            for (k=0; k<36; k++) {
                Y_R[k][Band] =
                Y_L[k][Band] = 0.f;
            }
        }

    }
    LEAVE(163);
    return;
}


/*
 *  Helper function for the qsort() in Resort_HuffTable() to sort a Huffman table
 *  by its codes.
 */

static int Cdecl
cmp_fn ( const void* p1, const void* p2 )
{
    if ( ((const Huffman_t*)p1) -> Code < ((const Huffman_t*)p2) -> Code ) return +1;
    if ( ((const Huffman_t*)p1) -> Code > ((const Huffman_t*)p2) -> Code ) return -1;
    return 0;
}


/*
 *  This functions sorts a Huffman table by its codes. It has also two other functions:
 *
 *    - The table contains LSB aligned codes, these are first MSB aligned.
 *    - The value entry is filled by its position plus 'offset' (Note that
 *      Make_HuffTable() don't fill this item. Offset can be used to offset
 *      range for instance from 0...6 to -3...+3.
 *
 *  Note that this function generates trash if you call it twice!
 */

void
Resort_HuffTable ( Huffman_t* const Table, const size_t elements, Int offset )
{
    size_t  i;

    for ( i = 0; i < elements; i++ ) {
        Table[i].Value  = i + offset;
        Table[i].Code <<= (32 - Table[i].Length);
    }

    qsort ( Table, elements, sizeof(*Table), cmp_fn );
    return;
}

#endif /* MPP_ENCODER */


/*
 *  Fills out the items Code and Length (but not Value) of a Huffman table
 *  from a bit packed Huffman table 'src'. Table is not sorted, so this is
 *  the table which is suitable for an encoder. Be careful: To get a table
 *  usable for a decoder you must use Resort_HuffTable() after this
 *  function. It's a little bit dangerous to divide the functionality, maybe
 *  there is a more secure and handy solution to this problem.
 */

void
Make_HuffTable ( Huffman_t* dst, const HuffSrc_t* src, size_t len )
{
    size_t  i;

    for ( i = 0; i < len; i++,src++,dst++ ) {
        dst->Code   = src->Code  ;
        dst->Length = src->Length;
    }
}


/*
 *  Generates a Lookup table for quick Huffman decoding. This table must
 *  have a size of a power of 2. Input is the pre-sorted Huffman table,
 *  sorted by Resort_HuffTable() and its length, and the length of the
 *  lookup table. Output is the Lookup table. It can be used for table based
 *  decoding (Huffman_decode_fastest) which fully decodes by means of the
 *  LUT. This is only handy for small huffman codes up to 9...10 bit
 *  maximum length. For longer codes partial lookup is possible with
 *  Huffman_decode_faster() which first estimates possible codes by means
 *  of LUT and then searches the exact code like the tableless version
 *  Huffman_decode().
 */

void
Make_LookupTable ( Uint8_t* LUT, size_t LUT_len, const Huffman_t* const Table, const size_t elements )
{
    size_t    i;
    size_t    idx  = elements;
    Uint32_t  dval = (Uint32_t)0x80000000L / LUT_len * 2;
    Uint32_t  val  = dval - 1;

    for ( i = 0; i < LUT_len; i++, val += dval ) {
        while ( idx > 0  &&  val >= Table[idx-1].Code )
            idx--;
        *LUT++ = (Uint8_t)idx;
    }

    return;
}


void
Init_FPU ( void )
{
    Uint16_t  cw;

#if   defined __i386__  &&  defined _FPU_GETCW  &&  defined _FPU_SETCW
    _FPU_GETCW ( cw );
    cw  &=  ~0x300;
    _FPU_SETCW ( cw );
#elif defined __i386__  &&  defined  FPU_GETCW  &&  defined  FPU_SETCW
    FPU_GETCW ( cw );
    cw  &=  ~0x300;
    FPU_SETCW ( cw );
#elif defined __MINGW32__
    __asm__ ("fnstcw %0" : "=m" (*&cw));
    cw  &=  ~0x300;
    __asm__ ("fldcw %0" : : "m" (*&cw));
#elif defined(_WIN32) && !defined(_WIN64)
    _asm { fstcw cw };
    cw  &=  ~0x300;
    _asm { fldcw cw };
#else
    ;
#endif
}

/* end of tools.c */
