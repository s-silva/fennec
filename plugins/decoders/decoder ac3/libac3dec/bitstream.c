/*
* This source code is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License as published by the Free Software Foundation; either
* version 2 of the License, or (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*       
* You should have received a copy of the GNU Lesser General Public
* License along with this library; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*
* File Name: bitstream.c							
*
* Reference:
*
* Author: Li Feng,  fli_linux@yahoo.com.cn                                                 
*
* Description:
*
* 	
* 
* History:
* 02/23/2005  Li Feng    Created
*  
*
*CodeReview Log:
* 
*/
#include <stdlib.h>
#include <stdio.h>
#include "ac3.h"
#include "bitstream.h"
#include "decode.h"
#include "crc.h"

#define SWAP_ENDIAN32(x)  ((((uint_8*)&x)[0] << 24) |  \
                         (((uint_8*)&x)[1] << 16) |  \
                         (((uint_8*)&x)[2] << 8) |   \
                         ((uint_8*)&x)[3])           

uint_32 bitstream_get_bh(bitstream_t *bs,uint_32 num_bits);
static void bitstream_fill_current(bitstream_t *bs);

/* Fetches 1-32 bits from the file opened in bitstream_open */
uint_32 bitstream_get(bitstream_t *bs,uint_32 num_bits)
{
	uint_32 result;
	
	if(num_bits < bs->bits_left)
	{
		result = (bs->current_word << (32 - bs->bits_left)) >> (32 - num_bits);
		bs->bits_left -= num_bits;
    	crc_process(result,num_bits);
		return result;
	}

	return bitstream_get_bh(bs,num_bits);
	
}

uint_32 bitstream_get_bh(bitstream_t *bs,uint_32 num_bits)
{
	uint_32 result;

   	num_bits -= bs->bits_left;
   	result = (bs->current_word << (32 - bs->bits_left)) >> (32 - bs->bits_left);

	if(bs->lpCurr < bs->lpEnd)
	{
		bitstream_fill_current(bs);
	}

	if(num_bits != 0)
		result = (result << num_bits) | (bs->current_word >> (32 - num_bits));
	
	bs->bits_left = 32 - num_bits;

	return result;
}

static void bitstream_fill_current(bitstream_t *bs)
{
	uint_32 current_word= *((uint_32*)bs->lpCurr);
	bs->lpCurr += 4;
	bs->current_word = SWAP_ENDIAN32(current_word);
}

/* Opens a bitstream for use in bitstream_get */
bitstream_t* bitstream_open(uint_8* lpStart, uint_8* lpEnd)
{
	bitstream_t *bs;
	bs = (bitstream_t*)malloc(sizeof(bitstream_t));
	if(!bs)
		return 0;

	bs->lpCurr = lpStart;
	bs->lpEnd = lpEnd;
	bs->bits_left = 0 ;

	return bs;
}


