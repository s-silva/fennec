/**----------------------------------------------------------------------------

 Fennec Decoder Plug-in 1.0 (MPEG).
 Copyright (C) 2007 Chase <c-h@users.sf.net>

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.

-------------------------------------------------------------------------------

----------------------------------------------------------------------------**/

#include "main.h"

int parse_xing(struct xing *xing, struct mad_bitptr ptr, unsigned int bitlen)
{
    if (bitlen < 64 || mad_bit_read(&ptr, 32) != XING_MAGIC)
        goto fail;

    xing->flags = mad_bit_read(&ptr, 32);
    bitlen -= 64;

    if (xing->flags & XING_FRAMES) {
        if (bitlen < 32)
            goto fail;

        xing->frames = mad_bit_read(&ptr, 32);
        bitlen -= 32;
    }

    if (xing->flags & XING_BYTES) {
        if (bitlen < 32)
            goto fail;

        xing->bytes = mad_bit_read(&ptr, 32);
        bitlen -= 32;
    }

    if (xing->flags & XING_TOC) {
        int i;

        if (bitlen < 800)
            goto fail;

        for (i = 0; i < 100; ++i)
            xing->toc[i] = (unsigned char) mad_bit_read(&ptr, 8);

        bitlen -= 800;
    }

    if (xing->flags & XING_SCALE) {
        if (bitlen < 32)
            goto fail;

        xing->scale = mad_bit_read(&ptr, 32);
        bitlen -= 32;
    }

    return 0;

fail:
    xing->flags = 0;
    return -1;
}

int scan_header(DWORD id, struct mad_header *header, struct xing *xing)
{
    struct mad_stream stream;
    struct mad_frame frame;
    unsigned char buffer[8192];
    unsigned int buflen = 0;
    int count = 0, result = 0;

    mad_stream_init(&stream);
    mad_frame_init(&frame);

    if (xing)
        xing->flags = 0;

    while (1) {

        if (buflen < sizeof(buffer)) {
            DWORD bytes;

			bytes = sys_file_read(pstreams[id].hstream, buffer + buflen, sizeof(buffer) - buflen);
            if(!bytes)
            {
                result = -1;
                break;
            }

            buflen += bytes;
        }

        mad_stream_buffer(&stream, buffer, buflen);

        while (1) {
            if (mad_frame_decode(&frame, &stream) == -1) {
                if (!MAD_RECOVERABLE(stream.error))
                    break;

                continue;
            }

            if (count++ ||
                    (xing && parse_xing(xing, stream.anc_ptr,
                                        stream.anc_bitlen) == 0))
                break;
        }

        if (count || stream.error != MAD_ERROR_BUFLEN)
            break;

        memmove(buffer, stream.next_frame,
                buflen = (unsigned int)(&buffer[buflen] - stream.next_frame));
    }

    if (count) {
        if (header)
            *header = frame.header;
    }
    else
        result = -1;

    mad_frame_finish(&frame);
    mad_stream_finish(&stream);

    return result;
}

__inline unsigned long prng(unsigned long state)
{
    return (state * 0x0019660dL + 0x3c6ef35fL) & 0xffffffffL;
}

__inline signed int dither(mad_fixed_t sample, struct dither *dither)
{
    unsigned int scalebits;
    mad_fixed_t output, mask, random;

    enum {
        MIN = -MAD_F_ONE,
        MAX =  MAD_F_ONE - 1
    };

    /* noise shape */
    sample += dither->error[0] - dither->error[1] + dither->error[2];

    dither->error[2] = dither->error[1];
    dither->error[1] = dither->error[0] / 2;

    /* bias */
    output = sample + (1L << (MAD_F_FRACBITS + 1 - SAMPLE_DEPTH - 1));

    scalebits = MAD_F_FRACBITS + 1 - SAMPLE_DEPTH;
    mask = (1L << scalebits) - 1;

    /* dither */
    random  = prng(dither->random);
    output += (random & mask) - (dither->random & mask);

    dither->random = random;

    /* clip */
    if (output > MAX) {
        output = MAX;

        if (sample > MAX)
            sample = MAX;
    }
    else if (output < MIN) {
        output = MIN;

        if (sample < MIN)
            sample = MIN;
    }

    /* quantize */
    output &= ~mask;

    /* error feedback */
    dither->error[0] = sample - output;

    /* scale */
    return output >> scalebits;
}

void pack_pcm(unsigned char **pcm, unsigned int nsamples, mad_fixed_t const *ch1, mad_fixed_t const *ch2)
{
    register signed int s0, s1;
    static struct dither d0, d1;

    if (ch2) {  /* stereo */
        while (nsamples--) {
            s0 = scale(*ch1++, &d0);
            s1 = scale(*ch2++, &d1);
# if SAMPLE_DEPTH == 16
            (*pcm)[0 + 0] = s0 >> 0;
            (*pcm)[0 + 1] = s0 >> 8;
            (*pcm)[2 + 0] = s1 >> 0;
            (*pcm)[2 + 1] = s1 >> 8;

            *pcm += 2 * 2;
# elif SAMPLE_DEPTH == 8
            (*pcm)[0] = s0 ^ 0x80;
            (*pcm)[1] = s1 ^ 0x80;

            *pcm += 2;
# else
#  error "bad SAMPLE_DEPTH"
# endif
        }
    }
    else {  /* mono */
        while (nsamples--) {
            s0 = scale(*ch1++, &d0);

# if SAMPLE_DEPTH == 16
            (*pcm)[0] = s0 >> 0;
            (*pcm)[1] = s0 >> 8;

            *pcm += 2;
# elif SAMPLE_DEPTH == 8
            *(*pcm)++ = s0 ^ 0x80;
# endif
        }
    }
}

void finish(unsigned long id)
{
    if(pstreams[id].initialized)
    {
		pstreams[id].initialized = 0;
        mad_synth_finish (&pstreams[id].decdata.synth);
        mad_frame_finish (&pstreams[id].decdata.frame);
        mad_stream_finish(&pstreams[id].decdata.stream);
    }
}

/*-----------------------------------------------------------------------------
 fennec, april 2007.
-----------------------------------------------------------------------------*/
