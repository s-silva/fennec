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


Uint32_t      Buffer [BUFFER_FULL];    // Buffer for bitstream-file
Uint32_t      dword         =  0;      // 32-bit-Word for Bitstream-I/O
int           filled        = 32;      // Position in the the 32-bit-word that's currently about to be filled
unsigned int  Zaehler       =  0;      // Position pointer for the processed bitstream-word (32 bit)
UintMax_t     BufferedBits  =  0;      // Counter for the number of written bits in the bitstream


/*
 *  Change_Endian32() changes the endianess of a 32-bit memory block in-place
 *  by swapping the byte order. This is a little bit tricky, but a well
 *  known method which is much much faster, especially on modern CPUs, than
 *  byte picking, because it avoids memory aliasing. Note that this method
 *  is poison for old 16-bit compilers!
 */

#if ENDIAN == HAVE_BIG_ENDIAN

static void
Change_Endian32 ( unsigned int* dst, size_t words32bit )
{
    ENTER(160);

    for ( ; words32bit--; dst++ ) {
# if  INT_MAX >= 2147483647L
        unsigned int  tmp = *dst;
        tmp  = ((tmp << 0x10) & 0xFFFF0000) | ((tmp >> 0x10) & 0x0000FFFF);
        tmp  = ((tmp << 0x08) & 0xFF00FF00) | ((tmp >> 0x08) & 0x00FF00FF);
        *dst = tmp;
# else
        char  tmp;
        tmp             = ((char*)dst)[0];
        ((char*)dst)[0] = ((char*)dst)[3];
        ((char*)dst)[3] = tmp;
        tmp             = ((char*)dst)[1];
        ((char*)dst)[1] = ((char*)dst)[2];
        ((char*)dst)[2] = tmp;
# endif
    }
    LEAVE(160);
    return;
}

#endif /* ENDIAN == HAVE_BIG_ENDIAN */


void
FlushBitstream ( FILE* fp, const Uint32_t* buffer, size_t words32bit )
{
    size_t           WrittenDwords = 0;
    const Uint32_t*  p             = buffer;

#if ENDIAN == HAVE_BIG_ENDIAN
    size_t           CC            = words32bit;
    Change_Endian32 ( (Uint32_t*)buffer, CC );
#endif

    // Write Buffer
    do {
        WrittenDwords = fwrite ( p, sizeof(*buffer), words32bit, fp );
        if ( WrittenDwords == 0 ) {
            stderr_printf ( "\b\n WARNING: Disk full?, retry after 10 sec ...\a" );
            sleep (10);
        }
        if ( WrittenDwords > 0 ) {
            p          += WrittenDwords;
            words32bit -= WrittenDwords;
        }
    } while ( words32bit != 0 );

#if ENDIAN == HAVE_BIG_ENDIAN
    Change_Endian32 ( (Uint32_t*)buffer, CC );
#endif
}


void
UpdateHeader ( FILE* fp, Uint32_t Frames, Uint ValidSamples )
{
    Uint8_t  buff [4];

    // Write framecount to header
    if ( fseek ( fp, 4L, SEEK_SET ) < 0 )
        return;

    buff [0] = (Uint8_t)(Frames >>  0);
    buff [1] = (Uint8_t)(Frames >>  8);
    buff [2] = (Uint8_t)(Frames >> 16);
    buff [3] = (Uint8_t)(Frames >> 24);

    fwrite ( buff, 1, 4, fp );

    // Write ValidSamples to header
    if ( fseek ( fp, 22L, SEEK_SET ) < 0 )
        return;
    fread ( buff, 1, 2, fp );
    if ( ferror(fp) )
        return;
    if ( fseek ( fp, 22L, SEEK_SET ) < 0 )
        return;

    ValidSamples <<= 4;
    ValidSamples  |= 0x800F & (((Uint) buff[1] << 8) | buff[0]);
    buff [0] = (Uint8_t)(ValidSamples >>  0);
    buff [1] = (Uint8_t)(ValidSamples >>  8);

    fwrite ( buff, 1, 2, fp );


    // Set filepointer to end of file (dirty method, should be old position!!)
    fseek ( fp, 0L, SEEK_END );
}


void
WriteBits ( const Uint32_t input, const unsigned int bits )
{
    BufferedBits += bits;
    filled       -= bits;

    if      ( filled > 0 ) {
        dword  |= input << filled;
    }
    else if ( filled < 0 ) {
        Buffer [Zaehler++] = dword | ( input >> -filled );
        filled += 32;
        dword   = input << filled;
    }
    else {
        Buffer [Zaehler++] = dword | input;
        filled  = 32;
        dword   =  0;
    }
}

// Bits in the original stream have to be 0, maximum X bits allowed to be set in input
// Actual bitstream must have already written ptr[0] and ptr[1]
void
WriteBitsAt ( const Uint32_t input, const unsigned int bits, BitstreamPos const pos )
{
    Uint32_t*     ptr    = pos.ptr;
    int           filled = pos.bit - bits;

//    fprintf ( stderr, "%5u %2u %08lX %2u\n", input, bits, pos.ptr, pos.bit );

    Buffer [Zaehler] = dword;

    if      ( filled > 0 ) {
        ptr [0] |= input << (  +filled);
    }
    else if ( filled < 0 ) {
        ptr [0] |= input >> (  -filled);
        ptr [1] |= input << (32+filled);
    }
    else {
        ptr [0] |= input;
    }

    dword = Buffer [Zaehler];
}


void
GetBitstreamPos ( BitstreamPos* const pos )
{
    pos -> ptr = Buffer + Zaehler;
    pos -> bit = filled;
}

/* end of bitstream.c */
